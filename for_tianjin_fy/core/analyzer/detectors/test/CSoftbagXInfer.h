#ifndef _CSOFTBAGXINFER_H_
#define _CSOFTBAGXINFER_H_

#include <fstream>
#include <iostream>
#include <sstream>
#include <numeric>
#include <chrono>
#include <vector>
#include <core/common.h>
#include <opencv2/opencv.hpp>
#include "dirent.h"
#include "NvInfer.h"
#include "cuda_runtime_api.h"
#include "logging.h"
#include "objectInfo.h"

#define CHECK(status) \
    do\
    {\
        auto ret = (status);\
        if (ret != 0)\
        {\
            std::cerr << "Cuda failure: " << ret << std::endl;\
            abort();\
        }\
    } while (0)

#define DEVICE 0  // GPU id
//#define NMS_THRESH 0.2
//#define BBOX_CONF_THRESH 0.1

using namespace cv;
using namespace std;
using namespace nvinfer1;

//struct Object
//{
//	cv::Rect_<float> rect;
//	int label;
//	float prob;
//};
//
//struct GridAndStride
//{
//	int grid0;
//	int grid1;
//	int stride;
//};

class CSoftbagXInfer
{
public:
  CSoftbagXInfer();
  ~CSoftbagXInfer();


  void doInference(IExecutionContext& context, float* input, float* output, const int output_size, cv::Size input_shape);

  void load(float overlap_thresh, float obj_thresh, const char* engine_file_path);
  void infer(unsigned char* img, int W, int H, int N, char* result);
  bool detect(const cv::Mat& cv_img, std::vector<Object>& boxes);

  IRuntime* runtime;
  ICudaEngine* engine;
  IExecutionContext* context;
};

#endif //_CSOFTBAGXINFER_H_
