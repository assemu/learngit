#include "tensorrt_yolov5_detector.h"
#include <NvOnnxParser.h>
#include "private/buffers.h"
#include "private/common.h"
#include "private/buffers.h"
#include <core/util/str_util.h>

//------------------------------------------------------

#define YOLO_INPUT_HEIGHT 640
#define YOLO_INPUT_WIDTH 640
#define NUM_ANCHORS 25200

using samplesCommon::SampleUniquePtr;

static void nms(std::vector<BoxInfo>& boxes, float overlap_threshold)
{
  //排序一下
  std::sort(boxes.begin(), boxes.end(),
  [](BoxInfo a, BoxInfo b) {
    return a.score > b.score;
  });

  std::vector<bool> isSuppressed(boxes.size(), false);
  for(int i = 0; i < int(boxes.size()); ++i) {
    if(isSuppressed[i])
      continue;

    for(int j = i + 1; j < int(boxes.size()); ++j) {
      if(isSuppressed[j])
        continue;

      cv::Rect irect = boxes[i].rect & boxes[j].rect; //矩形求交,不交时area()==0 ,empty()==true
      float ovr = ((float)irect.area()) / ((float)(boxes[i].rect.area() + boxes[j].rect.area() - irect.area()));

      if(ovr >= overlap_threshold)
        isSuppressed[j] = true;
    }
  }

  std::vector<BoxInfo> bb = boxes;
  boxes.clear();
  for(int i = 0; i < bb.size(); i++) {
    auto b = bb[i];
    if(!isSuppressed[i])
      boxes.push_back(b);
  }
}

class TensorRTYoloV5DetectorPrivate
{
public:
  TensorRTYoloV5DetectorPrivate(float overlap_threshold, float obj_threshold);

  ~TensorRTYoloV5DetectorPrivate();

  bool load(const std::string& labels_file_path, const std::string& engine_file_path);

  //img 输入图片
  //rects 检测得到的缺陷所在的外接矩形
  bool detect(const cv::Mat& img, std::vector<BoxInfo>& boxes);

private:
  float overlap_threshold;
  float obj_threshold;
  std::vector<std::string> class_names;

  cudaStream_t stream;
  std::shared_ptr<nvinfer1::ICudaEngine>                        _engine;
  std::shared_ptr<samplesCommon::BufferManager>                 _buffers;
  std::shared_ptr<nvinfer1::IExecutionContext>                  _context;
};

TensorRTYoloV5DetectorPrivate::TensorRTYoloV5DetectorPrivate(
  float overlap_threshold,
  float obj_threshold)
  : overlap_threshold(overlap_threshold),
    obj_threshold(obj_threshold)
{

}

TensorRTYoloV5DetectorPrivate::~TensorRTYoloV5DetectorPrivate()
{
  cudaStreamDestroy(stream);
}

bool TensorRTYoloV5DetectorPrivate::load(const std::string& labels_file_path, const std::string& engine_file_path)
{
  std::ifstream ifs(labels_file_path);
  std::string line;
  while(getline(ifs, line)) {
    str_trim(line);
    if(!line.empty())
      this->class_names.push_back(line);
  }

  FILE* fp = fopen(engine_file_path.c_str(), "r");
  if(!fp) { //如果有序列化缓存文件
    std::cout << "TensorRTYoloV5DetectorPrivate load model failed!" << std::endl;
    return false;
  }

  fseek(fp, 0L, SEEK_END);
  int size = ftell(fp);
  fseek(fp, 0L, SEEK_SET);
  char* buff = (char*)malloc(size);
  fread(buff, size, 1, fp);
  fclose(fp);
  std::cout << "loading tensorrt engine file ... " << std::endl;
  SampleUniquePtr<IRuntime> _runtime{ createInferRuntime(sample::gLogger.getTRTLogger()) };
  _engine = std::shared_ptr<nvinfer1::ICudaEngine>(_runtime->deserializeCudaEngine(buff, size), samplesCommon::InferDeleter());
  free(buff);

  _context = std::shared_ptr<nvinfer1::IExecutionContext>(_engine->createExecutionContext());
  _buffers = std::make_shared<samplesCommon::BufferManager>(_engine, 0, _context.get());

  if(!_engine) {
    std::cerr << "init tensorrt yolo  detector - deserializeCudaEngine failed!" << std::endl;
    return false;
  } else {
    std::cout << "tensorrt engine file loaded!" << std::endl;
  }

  cudaStreamCreate(&stream);
  return true;
}

static cv::Mat resize_image(const cv::Mat& img, int& neww, int& newh, int& left, int& top)
{
  float r_w = YOLO_INPUT_WIDTH / (img.cols * 1.0);
  float r_h = YOLO_INPUT_HEIGHT / (img.rows * 1.0);
  if(r_h > r_w) {
    neww = YOLO_INPUT_WIDTH;
    newh = r_w * img.rows;
    left = 0;
    top = (YOLO_INPUT_HEIGHT - newh) / 2;
  } else {
    neww = r_h * img.cols;
    newh = YOLO_INPUT_HEIGHT;
    left = (YOLO_INPUT_WIDTH - neww) / 2;
    top = 0;
  }
  cv::Mat re(newh, neww, CV_8UC3);
  cv::resize(img, re, re.size(), 0, 0, cv::INTER_LINEAR);
  cv::Mat out(YOLO_INPUT_HEIGHT, YOLO_INPUT_WIDTH, CV_8UC3, cv::Scalar(128, 128, 128));
  re.copyTo(out(cv::Rect(left, top, re.cols, re.rows)));
  return out;
}

bool TensorRTYoloV5DetectorPrivate::detect(const cv::Mat& img, std::vector<BoxInfo>& boxes)
{
  int left = 0, top = 0, newh = 0, neww = 0;
  cv::Mat input_img = resize_image(img, neww, newh, left, top);

  //图片归一化并处理到缓冲区
  float* hostDataBuffer = static_cast<float*>(_buffers->getHostBuffer("images"));
  {
    int width = input_img.rows;
    int height = input_img.cols;
    for(int channel = 0; channel < 3; channel++) {
      for(int r = 0; r < width; r++) {
        for(int c = 0; c < height; c++) {
          float pix = input_img.ptr<uchar>(r)[c * 3 + channel];
          hostDataBuffer[channel * width * height + r * height + c] = pix / 255.0;
        }
      }
    }
  }

  _buffers->copyInputToDeviceAsync(stream); //从内存拷贝到显存

  //推理
  bool status = _context->enqueueV2(_buffers->getDeviceBindings().data(), stream, NULL);
  if(!status) {
    std::cerr << "execute detection in GPU failed!" << std::endl;
    return false;
  }
  _buffers->copyOutputToHostAsync(stream); //显存拷贝到内存
  cudaStreamSynchronize(stream); //同步流

  //处理结果
  /*
   输出的内存结构
   -----------------------------------------------------------------------------------------
   | 框心x | 框心y | 框宽  | 框高  | 总概率| 分类1概率 | 分类2概率 | 分类3概率 | 分类4概率 |
   -----------------------------------------------------------------------------------------
   | float | float | float | float | float |   float   |   float   |   float   |   float   |
   -----------------------------------------------------------------------------------------
   | float | float | float | float | float |   float   |   float   |   float   |   float   |
   -----------------------------------------------------------------------------------------
   ...  共25200
   -----------------------------------------------------------------------------------------
   | float | float | float | float | float |   float   |   float   |   float   |   float   |
   -----------------------------------------------------------------------------------------
   | float | float | float | float | float |   float   |   float   |   float   |   float   |
   -----------------------------------------------------------------------------------------
   | float | float | float | float | float |   float   |   float   |   float   |   float   |
   -----------------------------------------------------------------------------------------
  */

  float* outs = static_cast<float*>(_buffers->getHostBuffer("output"));
  float ratioh = (float)img.rows / newh;
  float ratiow = (float)img.cols / neww;
  int i = 0, j = 0, nout = this->class_names.size() + 5;
  for(i = 0; i < NUM_ANCHORS; i++) {
    const float* pdata = outs + i * nout;
    float obj_conf = pdata[4];
    if(obj_conf > this->obj_threshold) {
      int max_ind = 0;
      float max_class_socre = 0;
      for(j = 0; j < this->class_names.size(); j++) {
        if(pdata[5 + j] > max_class_socre) {
          max_class_socre = pdata[5 + j];
          max_ind = j;
        }
      }

      int x0 = (int)std::max<float>((pdata[0] - 0.5 * pdata[2] - left) * ratiow, 0.f);
      int y0 = (int)std::max<float>((pdata[1] - 0.5 * pdata[3] - top) * ratioh, 0.f);
      auto label = this->class_names[max_ind];
      boxes.push_back(BoxInfo{ cv::Rect(x0, y0, pdata[2]*ratiow, pdata[3]*ratioh), max_class_socre * obj_conf, label });
    }
  }

  nms(boxes, this->overlap_threshold); //过滤
  return true;
}

//------------------------------------------------------

TensorRTYoloV5Detector::TensorRTYoloV5Detector(float overlap_threshold, float obj_threshold)
{
  _p = new TensorRTYoloV5DetectorPrivate(overlap_threshold, obj_threshold);
}

TensorRTYoloV5Detector::~TensorRTYoloV5Detector()
{
  delete _p;
}

bool TensorRTYoloV5Detector::detect(const cv::Mat& img, std::vector<BoxInfo>& boxes)
{
  return _p->detect(img, boxes);
}

bool TensorRTYoloV5Detector::load(const std::string& labels_file_path, const std::string& engine_file_path)
{
  return _p->load(labels_file_path, engine_file_path);
}
