#include "CSoftbagXInfer.h"



CSoftbagXInfer::CSoftbagXInfer()

{

}



CSoftbagXInfer::~CSoftbagXInfer()

{

  context->destroy();

  engine->destroy();

  runtime->destroy();

}

// stuff we know about the network and the input/output blobs

static const int INPUT_W = 640;

static const int INPUT_H = 640;

static const int NUM_CLASSES = 6;

const char* INPUT_BLOB_NAME = "images";/* "model1017.pth"*/

const char* OUTPUT_BLOB_NAME = "output";

static Logger gLogger;



static cv::Mat static_resize(const cv::Mat& img)

{
  float r = min(INPUT_W / (img.cols * 1.0), INPUT_H / (img.rows * 1.0));

  // r = std::min(r, 1.0f);

  int unpad_w = r * img.cols;
  int unpad_h = r * img.rows;
  cv::Mat re(unpad_h, unpad_w, CV_8UC3);
  cv::resize(img, re, re.size());
  cv::Mat out(INPUT_H, INPUT_W, CV_8UC3, cv::Scalar(114, 114, 114));
  re.copyTo(out(cv::Rect(0, 0, re.cols, re.rows)));
  return out;
}


static void generate_grids_and_stride(std::vector<int>& strides, std::vector<GridAndStride>& grid_strides)
{
  for(auto stride : strides) {
    int num_grid_y = INPUT_H / stride;
    int num_grid_x = INPUT_W / stride;
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

static inline float intersection_area(const Object& a, const Object& b)
{
  cv::Rect_<float> inter = a.rect & b.rect;
  return inter.area();
}

static void qsort_descent_inplace(std::vector<Object>& faceobjects, int left, int right)
{
  int i = left;
  int j = right;
  float p = faceobjects[(left + right) / 2].prob;

  while(i <= j) {
    while(faceobjects[i].prob > p)
      i++;

    while(faceobjects[j].prob < p)
      j--;

    if(i <= j) {
      // swap
      std::swap(faceobjects[i], faceobjects[j]);

      i++;
      j--;
    }
  }

  #pragma omp parallel sections
  {
    #pragma omp section
    {
      if(left < j) qsort_descent_inplace(faceobjects, left, j);
    }
    #pragma omp section
    {
      if(i < right) qsort_descent_inplace(faceobjects, i, right);
    }
  }
}

static void qsort_descent_inplace(std::vector<Object>& objects)
{
  if(objects.empty())
    return;

  qsort_descent_inplace(objects, 0, objects.size() - 1);
}

static void nms_sorted_bboxes(const std::vector<Object>& faceobjects, std::vector<int>& picked, float nms_threshold)
{
  picked.clear();

  const int n = faceobjects.size();

  std::vector<float> areas(n);
  for(int i = 0; i < n; i++) {
    areas[i] = faceobjects[i].rect.area();
  }

  for(int i = 0; i < n; i++) {
    const Object& a = faceobjects[i];

    int keep = 1;
    for(int j = 0; j < (int)picked.size(); j++) {
      const Object& b = faceobjects[picked[j]];

      // intersection over union
      float inter_area = intersection_area(a, b);
      float union_area = areas[i] + areas[picked[j]] - inter_area;
      // float IoU = inter_area / union_area
      if(inter_area / union_area > nms_threshold)
        keep = 0;
    }

    if(keep)
      picked.push_back(i);
  }
}

static void generate_yolox_proposals(std::vector<GridAndStride> grid_strides, float* feat_blob, float prob_threshold, std::vector<Object>& objects)
{

  const int num_anchors = grid_strides.size();

  for(int anchor_idx = 0; anchor_idx < num_anchors; anchor_idx++) {
    const int grid0 = grid_strides[anchor_idx].grid0;
    const int grid1 = grid_strides[anchor_idx].grid1;
    const int stride = grid_strides[anchor_idx].stride;

    const int basic_pos = anchor_idx * (NUM_CLASSES + 5);

    // yolox/models/yolo_head.py decode logic
    float x_center = (feat_blob[basic_pos + 0] + grid0) * stride;
    float y_center = (feat_blob[basic_pos + 1] + grid1) * stride;
    float w = exp(feat_blob[basic_pos + 2]) * stride;
    float h = exp(feat_blob[basic_pos + 3]) * stride;
    float x0 = x_center - w * 0.5f;
    float y0 = y_center - h * 0.5f;

    float box_objectness = feat_blob[basic_pos + 4];
    for(int class_idx = 0; class_idx < NUM_CLASSES; class_idx++) {
      float box_cls_score = feat_blob[basic_pos + 5 + class_idx];
      float box_prob = box_objectness * box_cls_score;
      if(box_prob > prob_threshold) {
        Object obj;
        obj.rect.x = x0;
        obj.rect.y = y0;
        obj.rect.width = w;
        obj.rect.height = h;
        obj.label = class_idx;
        obj.prob = box_prob;

        objects.push_back(obj);
      }

    } // class loop

  } // point anchor loop
}

float* blobFromImage(cv::Mat& img)
{
  float* blob = new float[img.total() * 3];
  int channels = 3;
  int img_h = img.rows;
  int img_w = img.cols;
  for(size_t c = 0; c < channels; c++) {
    for(size_t h = 0; h < img_h; h++) {
      for(size_t w = 0; w < img_w; w++) {
        //blob[c * img_w * img_h + h * img_w + w] = (float)img.at<cv::Vec3b>(h, w)[c];
        if(c == 0) blob[c * img_w * img_h + h * img_w + w] = ((float)(img.at<cv::Vec3b>(h, w)[c]) / 255.0 - 0.485) / 0.229;  //c==0 channel Red, cause input is gray
        else if(c == 1) blob[c * img_w * img_h + h * img_w + w] = ((float)(img.at<cv::Vec3b>(h, w)[c]) / 255.0 - 0.456) / 0.224;  //c==1 channel G, cause input is gray
        else if(c == 2) blob[c * img_w * img_h + h * img_w + w] = ((float)(img.at<cv::Vec3b>(h, w)[c]) / 255.0 - 0.400) / 0.225;  //c==2 channel B, cause input is gray
        else;
      }
    }
  }
  return blob;
}

float overlapThresh = 0.0;
float objThresh = 0.0;
static void decode_outputs(float* prob, std::vector<Object>& objects, float scale, const int img_w, const int img_h)
{
  std::vector<Object> proposals;
  std::vector<int> strides = { 8, 16, 32 };
  std::vector<GridAndStride> grid_strides;
  generate_grids_and_stride(strides, grid_strides);
  generate_yolox_proposals(grid_strides, prob, overlapThresh, proposals);
  std::cout << "num of boxes before nms: " << proposals.size() << std::endl;

  qsort_descent_inplace(proposals);

  std::vector<int> picked;
  nms_sorted_bboxes(proposals, picked, objThresh);


  int count = picked.size();

  std::cout << "num of boxes: " << count << std::endl;

  objects.resize(count);
  for(int i = 0; i < count; i++) {
    objects[i] = proposals[picked[i]];

    // adjust offset to original unpadded
    float x0 = (objects[i].rect.x) / scale;
    float y0 = (objects[i].rect.y) / scale;
    float x1 = (objects[i].rect.x + objects[i].rect.width) / scale;
    float y1 = (objects[i].rect.y + objects[i].rect.height) / scale;

    // clip
    x0 = max(min(x0, (float)(img_w - 1)), 0.f);
    y0 = max(min(y0, (float)(img_h - 1)), 0.f);
    x1 = max(min(x1, (float)(img_w - 1)), 0.f);
    y1 = max(min(y1, (float)(img_h - 1)), 0.f);

    objects[i].rect.x = x0;
    objects[i].rect.y = y0;
    objects[i].rect.width = x1 - x0;
    objects[i].rect.height = y1 - y0;
  }
}

const float color_list[80][3] = {
  {0.000, 0.447, 0.741},
  {0.850, 0.325, 0.098},
  {0.929, 0.694, 0.125},
  {0.494, 0.184, 0.556},
  {0.466, 0.674, 0.188},
  {0.301, 0.745, 0.933},
  {0.635, 0.078, 0.184},
  {0.300, 0.300, 0.300},
  {0.600, 0.600, 0.600},
  {1.000, 0.000, 0.000},
  {1.000, 0.500, 0.000},
  {0.749, 0.749, 0.000},
  {0.000, 1.000, 0.000},
  {0.000, 0.000, 1.000},
  {0.667, 0.000, 1.000},
  {0.333, 0.333, 0.000},
  {0.333, 0.667, 0.000},
  {0.333, 1.000, 0.000},
  {0.667, 0.333, 0.000},
  {0.667, 0.667, 0.000},
  {0.667, 1.000, 0.000},
  {1.000, 0.333, 0.000},
  {1.000, 0.667, 0.000},
  {1.000, 1.000, 0.000},
  {0.000, 0.333, 0.500},
  {0.000, 0.667, 0.500},
  {0.000, 1.000, 0.500},
  {0.333, 0.000, 0.500},
  {0.333, 0.333, 0.500},
  {0.333, 0.667, 0.500},
  {0.333, 1.000, 0.500},
  {0.667, 0.000, 0.500},
  {0.667, 0.333, 0.500},
  {0.667, 0.667, 0.500},
  {0.667, 1.000, 0.500},
  {1.000, 0.000, 0.500},
  {1.000, 0.333, 0.500},
  {1.000, 0.667, 0.500},
  {1.000, 1.000, 0.500},
  {0.000, 0.333, 1.000},
  {0.000, 0.667, 1.000},
  {0.000, 1.000, 1.000},
  {0.333, 0.000, 1.000},
  {0.333, 0.333, 1.000},
  {0.333, 0.667, 1.000},
  {0.333, 1.000, 1.000},
  {0.667, 0.000, 1.000},
  {0.667, 0.333, 1.000},
  {0.667, 0.667, 1.000},
  {0.667, 1.000, 1.000},
  {1.000, 0.000, 1.000},
  {1.000, 0.333, 1.000},
  {1.000, 0.667, 1.000},
  {0.333, 0.000, 0.000},
  {0.500, 0.000, 0.000},
  {0.667, 0.000, 0.000},
  {0.833, 0.000, 0.000},
  {1.000, 0.000, 0.000},
  {0.000, 0.167, 0.000},
  {0.000, 0.333, 0.000},
  {0.000, 0.500, 0.000},
  {0.000, 0.667, 0.000},
  {0.000, 0.833, 0.000},
  {0.000, 1.000, 0.000},
  {0.000, 0.000, 0.167},
  {0.000, 0.000, 0.333},
  {0.000, 0.000, 0.500},
  {0.000, 0.000, 0.667},
  {0.000, 0.000, 0.833},
  {0.000, 0.000, 1.000},
  {0.000, 0.000, 0.000},
  {0.143, 0.143, 0.143},
  {0.286, 0.286, 0.286},
  {0.429, 0.429, 0.429},
  {0.571, 0.571, 0.571},
  {0.714, 0.714, 0.714},
  {0.857, 0.857, 0.857},
  {0.000, 0.447, 0.741},
  {0.314, 0.717, 0.741},
  {0.50, 0.5, 0}
};

static void draw_objects(const cv::Mat& bgr, const std::vector<Object>& objects, int N, string& t)
{
  static const char* class_names[] = {
    "bright", "dark", "dirty", "circle", "scratch", "water"
  };

  cv::Mat image = bgr.clone();
  string temp;
  for(size_t i = 0; i < objects.size(); i++) {
    const Object& obj = objects[i];
    fprintf(stderr, "%d = %.5f at %.2f %.2f %.2f x %.2f\n", obj.label, obj.prob,
            obj.rect.x, obj.rect.y, obj.rect.width, obj.rect.height);

    cv::Scalar color = cv::Scalar(color_list[obj.label][0], color_list[obj.label][1], color_list[obj.label][2]);
    float c_mean = cv::mean(color)[0];
    cv::Scalar txt_color;
    if(c_mean > 0.5) {
      txt_color = cv::Scalar(0, 0, 0);
    } else {
      txt_color = cv::Scalar(255, 255, 255);
    }

    cv::rectangle(image, obj.rect, color * 255, 2);

    char text[256];
    sprintf(text, "%s %.1f%%", class_names[obj.label], obj.prob * 100);

    int baseLine = 0;
    cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.4, 1, &baseLine);

    cv::Scalar txt_bk_color = color * 0.7 * 255;

    int x = obj.rect.x;
    int y = obj.rect.y + 1;
    //int y = obj.rect.y - label_size.height - baseLine;
    if(y > image.rows)
      y = image.rows;

    //temp = temp + to_string((int)(obj.rect.x)) + "_" + to_string((int)obj.rect.y) + "_" + to_string((int)obj.rect.width) + "_" + to_string((int)obj.rect.height) + "_" + to_string(obj.prob) + "_" + class_names[obj.label] +"_";
    //if ((strcmp(class_names[obj.label], "white") == 0 && (obj.prob > overlapThresh /*objThresh*/) && N == 2) || (strcmp(class_names[obj.label], "hair") == 0 && obj.prob > objThresh)
    //	|| (strcmp(class_names[obj.label], "stick") == 0 && obj.prob > objThresh) || (strcmp(class_names[obj.label], "insect") == 0 && obj.prob > objThresh-0.06)
    //	|| (strcmp(class_names[obj.label], "big") == 0 && obj.prob > objThresh && N != 2))
    {
      temp = temp + to_string((int)obj.rect.x) + "_" + to_string((int)obj.rect.y) + "_" + to_string((int)obj.rect.height) + "_"
             + to_string((int)obj.rect.width) + "_" + to_string(obj.prob) + "_" + class_names[obj.label] + "_";
    }

    t = temp;
    //strcpy(result, temp.c_str());
    //fdll << t << endl;
    //if (i == 3) break;

    //if (x + label_size.width > image.cols)
    //x = image.cols - label_size.width;
  }
}


void CSoftbagXInfer::doInference(IExecutionContext& context, float* input, float* output, const int output_size, cv::Size input_shape)
{
  const ICudaEngine& engine = context.getEngine();

  // Pointers to input and output device buffers to pass to engine.
  // Engine requires exactly IEngine::getNbBindings() number of buffers.
  assert(engine.getNbBindings() == 2);
  void* buffers[2];

  // In order to bind the buffers, we need to know the names of the input and output tensors.
  // Note that indices are guaranteed to be less than IEngine::getNbBindings()
  const int inputIndex = engine.getBindingIndex(INPUT_BLOB_NAME);

  assert(engine.getBindingDataType(inputIndex) == nvinfer1::DataType::kFLOAT);
  const int outputIndex = engine.getBindingIndex(OUTPUT_BLOB_NAME);
  assert(engine.getBindingDataType(outputIndex) == nvinfer1::DataType::kFLOAT);
  int mBatchSize = engine.getMaxBatchSize();

  // Create GPU buffers on device
  CHECK(cudaMalloc(&buffers[inputIndex], 3 * input_shape.height * input_shape.width * sizeof(float)));
  CHECK(cudaMalloc(&buffers[outputIndex], output_size * sizeof(float)));

  // Create stream
  cudaStream_t stream;
  CHECK(cudaStreamCreate(&stream));

  // DMA input batch data to device, infer on the batch asynchronously, and DMA output back to host
  CHECK(cudaMemcpyAsync(buffers[inputIndex], input, 3 * input_shape.height * input_shape.width * sizeof(float), cudaMemcpyHostToDevice, stream));
  context.enqueue(1, buffers, stream, nullptr);
  CHECK(cudaMemcpyAsync(output, buffers[outputIndex], output_size * sizeof(float), cudaMemcpyDeviceToHost, stream));
  cudaStreamSynchronize(stream);

  // Release stream and buffers
  cudaStreamDestroy(stream);
  CHECK(cudaFree(buffers[inputIndex]));
  CHECK(cudaFree(buffers[outputIndex]));
}

void CSoftbagXInfer::load(float overlap_thresh, float obj_thresh, const char* engine_file_path)
{
  overlapThresh = overlap_thresh;
  objThresh = obj_thresh;

  cudaSetDevice(DEVICE);
  // create a model using the API directly and serialize it to a stream
  char* trtModelStream{ nullptr };
  size_t size{ 0 };

  const std::string engine_file = engine_file_path; ///
  std::ifstream file(engine_file, std::ios::binary);
  if(file.good()) {
    file.seekg(0, file.end);
    size = file.tellg();
    file.seekg(0, file.beg);
    trtModelStream = new char[size];
    assert(trtModelStream);
    file.read(trtModelStream, size);
    file.close();
    cout << "engine file loaded..." << endl;
  }

  runtime = createInferRuntime(gLogger);
  assert(runtime != nullptr);
  engine = runtime->deserializeCudaEngine(trtModelStream, size);
  assert(engine != nullptr);
  context = engine->createExecutionContext();
  assert(context != nullptr);

  delete[] trtModelStream;
}

bool CSoftbagXInfer::detect(const cv::Mat& cv_img, std::vector<Object>& boxes)
{
  auto output_size = 1;
  auto out_dims = engine->getBindingDimensions(1);
  //cout << "3..." << endl;
  for(int j = 0; j < out_dims.nbDims; j++) {
    output_size *= out_dims.d[j];
  }
  //cout << "output_size: " << output_size << endl;
  int img_w = cv_img.cols;
  int img_h = cv_img.rows;
  cv::Mat pr_img = static_resize(cv_img);
  //normalize(pr_img, pr_img, 1.0, 0, NORM_INF);
  std::cout << "blob image" << std::endl;
  /*static*/ float* prob = new float[output_size];

  float* blob;
  blob = blobFromImage(pr_img);
  float scale = min(INPUT_W / (cv_img.cols * 1.0), INPUT_H / (cv_img.rows * 1.0));

  doInference(*context, blob, prob, output_size, pr_img.size());

  std::vector<Object> objs;
  decode_outputs(prob, objs, scale, img_w, img_h);
  boxes = objs;

  delete blob;
  return true;
}

void CSoftbagXInfer::infer(unsigned char* img, int W, int H, int N, char* result)
{
  //cout << "1..." << endl;
  Mat cv_img(H, W, CV_8UC3, img); //CV_8UC3

  auto output_size = 1;
  auto out_dims = engine->getBindingDimensions(1);
  //cout << "3..." << endl;
  for(int j = 0; j < out_dims.nbDims; j++) {
    output_size *= out_dims.d[j];
  }
  //cout << "output_size: " << output_size << endl;
  int img_w = cv_img.cols;
  int img_h = cv_img.rows;
  cv::Mat pr_img = static_resize(cv_img);
  //normalize(pr_img, pr_img, 1.0, 0, NORM_INF);
  std::cout << "blob image" << std::endl;
  /*static*/ float* prob = new float[output_size];

  float* blob;
  blob = blobFromImage(pr_img);
  float scale = min(INPUT_W / (cv_img.cols * 1.0), INPUT_H / (cv_img.rows * 1.0));

  auto start = std::chrono::system_clock::now();
  doInference(*context, blob, prob, output_size, pr_img.size());
  auto end = std::chrono::system_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

  std::vector<Object> objects;
  string temp;
  decode_outputs(prob, objects, scale, img_w, img_h);
  cout << objects.size() << endl;
  draw_objects(cv_img, objects, N, temp);
  strcpy(result, temp.c_str());
  objects.clear();

  delete blob;

}
