#ifndef TENSORRT_UTIL_H_
#define TENSORRT_UTIL_H_

#include <memory>
#include <string>

class TensorRTUtil
{
public:
  /**
   * @brief 转换模型,
   * 需要在识别之前调用
   * 从onnx模型生成tensorRT的engine文件 每台机器生成一次(时间很长)
   *
   * @param onnx_file_path          onnx文件路径
   * @param engine_file_path          生成得engine文件路径
   */
  static void convertOnnxToTensorRT(const std::string& onnx_file_path, const std::string& engine_file_path);
};

#endif //TENSORRT_UTIL_H_
