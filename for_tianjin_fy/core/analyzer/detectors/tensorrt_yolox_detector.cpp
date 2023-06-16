#include "tensorrt_yolox_detector.h"
#include <NvOnnxParser.h>
#include "private/buffers.h"
#include "private/common.h"
#include "private/buffers.h"
#include <core/util/str_util.h>

//-----------------------------------------------------

#define YOLO_INPUT_HEIGHT 640
#define YOLO_INPUT_WIDTH 640
#define ANCHER_NUM 8400

using samplesCommon::SampleUniquePtr;

struct GridAndStride {
  int grid0;
  int grid1;
  int stride;
};

static void generate_grids_and_stride(const std::vector<int>& strides, std::vector<GridAndStride>& grid_strides)
{
  for(auto& stride : strides) {
    int num_grid_y = YOLO_INPUT_HEIGHT / stride;
    int num_grid_x = YOLO_INPUT_WIDTH / stride;
    for(int g1 = 0; g1 < num_grid_y; g1++) {
      for(int g0 = 0; g0 < num_grid_x; g0++) {
        GridAndStride g;
        g.grid0 = g0;
        g.grid1 = g1;
        g.stride = stride;
        grid_strides.push_back(g);
      }
    }
  }
}

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

class TensorRTYoloXDetectorPrivate
{
public:
  TensorRTYoloXDetectorPrivate(float overlap_threshold, float obj_threshold);

  ~TensorRTYoloXDetectorPrivate();

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

TensorRTYoloXDetectorPrivate::TensorRTYoloXDetectorPrivate(
  float overlap_threshold,
  float obj_threshold)
  : overlap_threshold(overlap_threshold),
    obj_threshold(obj_threshold)
{

}

TensorRTYoloXDetectorPrivate::~TensorRTYoloXDetectorPrivate()
{
  cudaStreamDestroy(stream);
}

bool TensorRTYoloXDetectorPrivate::load(const std::string& labels_file_path, const std::string& engine_file_path)
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
    std::cout << "TensorRTYoloXDetectorPrivate load model failed!" << std::endl;
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

static cv::Mat resize_image(const cv::Mat& img)
{
  float r = std::min(YOLO_INPUT_WIDTH / (img.cols * 1.0), YOLO_INPUT_HEIGHT / (img.rows * 1.0));
  int unpad_w = r * img.cols;
  int unpad_h = r * img.rows;
  cv::Mat re(unpad_h, unpad_w, CV_8UC3);
  cv::resize(img, re, re.size(), 0, 0, cv::INTER_CUBIC);
  cv::Mat out(YOLO_INPUT_WIDTH, YOLO_INPUT_HEIGHT, CV_8UC3, cv::Scalar(114, 114, 114));
  re.copyTo(out(cv::Rect(0, 0, re.cols, re.rows)));
  return out;
}

static int argmax(float* data, int num)
{
  float max_value = 0.0;
  int max_index = 0;
  for(int i = 0; i < num; ++i) {
    float value = data[i];
    if(value > max_value) {
      max_value = value;
      max_index = i;
    }
  }

  return max_index;
}

bool TensorRTYoloXDetectorPrivate::detect(const cv::Mat& img, std::vector<BoxInfo>& boxes)
{
  cv::Mat input_img = resize_image(img);

  //图片归一化并处理到缓冲区
  float* hostDataBuffer = static_cast<float*>(_buffers->getHostBuffer("images"));
  {
    int width = input_img.rows;
    int height = input_img.cols;
    for(int channel = 0; channel < 3; channel++) {
      for(int r = 0; r < width; r++) {
        for(int c = 0; c < height; c++) {
          float pix = input_img.ptr<uchar>(r)[c * 3 + channel];
		  if (channel == 0) hostDataBuffer[channel * width * height + r * height + c] = (pix / 255.0 - 0.485) / 0.229;
		  else if (channel == 1) hostDataBuffer[channel * width * height + r * height + c] = (pix / 255.0 - 0.456) / 0.224;
		  else if (channel == 2) hostDataBuffer[channel * width * height + r * height + c] = (pix / 255.0 - 0.406) / 0.225;
		  else;
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
  float* output_data = static_cast<float*>(_buffers->getHostBuffer("output"));
  std::vector<int> strides = { 8, 16, 32 }; //8400
  std::vector<GridAndStride> grid_strides;
  generate_grids_and_stride(strides, grid_strides);
  assert(grid_strides.size() == ANCHER_NUM);

  for(int i = 0; i < ANCHER_NUM; i++) {
    const int grid0 = grid_strides[i].grid0;
    const int grid1 = grid_strides[i].grid1;
    const int stride = grid_strides[i].stride;

    float* ptr = output_data + i * (this->class_names.size() + 5);
    float score = ptr[4];

    {
      int class_id = argmax(&ptr[5], this->class_names.size());
      float confidence = ptr[class_id + 5] * score;
      if(confidence >= this->obj_threshold) {
        float centerX = (ptr[0] + grid0) * stride;
        float centerY = (ptr[1] + grid1) * stride;
        float width = exp(ptr[2]) * stride;
        float height = exp(ptr[3]) * stride;
        float x0 = centerX - width * 0.5f;
        float y0 = centerY - height * 0.5f;

        auto label = this->class_names[class_id];
        boxes.push_back(BoxInfo{ cv::Rect(x0, y0, width, height), confidence, label });
		
      }
    }	
  }

  nms(boxes, this->overlap_threshold); //过滤

  std::vector<BoxInfo> picked;
  picked.resize(boxes.size());
  float scale = std::min(YOLO_INPUT_WIDTH / (img.cols * 1.0), YOLO_INPUT_HEIGHT / (img.rows * 1.0));
  for(auto& b : boxes) {
    float x0 = (b.rect.x) / scale;
    float y0 = (b.rect.y) / scale;
    float x1 = (b.rect.x + b.rect.width) / scale;
    float y1 = (b.rect.y + b.rect.height) / scale;
    x0 = std::max(std::min(x0, (float)(img.cols - 1)), 0.f);
    y0 = std::max(std::min(y0, (float)(img.rows - 1)), 0.f);
    x1 = std::max(std::min(x1, (float)(img.cols - 1)), 0.f);
    y1 = std::max(std::min(y1, (float)(img.rows - 1)), 0.f);
    picked.push_back(BoxInfo{ cv::Rect(x0, y0, x1 - x0, y1 - y0), b.score, b.label });
	//std::cout << "-----------666666666----------------------->" << b.score << " " << b.label << std::endl;
  }
  boxes = picked;
  return true;
}

//------------------------------------------------------

TensorRTYoloXDetector::TensorRTYoloXDetector(float overlap_threshold, float obj_threshold)
{
  _p = new TensorRTYoloXDetectorPrivate(overlap_threshold, obj_threshold);
}

TensorRTYoloXDetector::~TensorRTYoloXDetector()
{
  delete _p;
}

bool TensorRTYoloXDetector::detect(const cv::Mat& img, std::vector<BoxInfo>& boxes)
{
  return _p->detect(img, boxes);
}

bool TensorRTYoloXDetector::load(const std::string& labels_file_path, const std::string& engine_file_path)
{
  return _p->load(labels_file_path, engine_file_path);
}
