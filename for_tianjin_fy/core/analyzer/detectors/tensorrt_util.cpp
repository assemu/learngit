#include "./tensorrt_util.h"
#include <NvOnnxParser.h>
#include "private/buffers.h"
#include "private/common.h"
#include "private/buffers.h"

//------------------------------------------------------

using samplesCommon::SampleUniquePtr;

void TensorRTUtil::convertOnnxToTensorRT(const std::string& onnx_file_path,
    const std::string& engine_file_path)
{
  std::ifstream fs(engine_file_path);
  if(!fs) {
    std::cerr <<  engine_file_path << " not exist, onnx model ===> tensorrt model" << std::endl;

    auto _builder = SampleUniquePtr<nvinfer1::IBuilder>(nvinfer1::createInferBuilder(sample::gLogger.getTRTLogger()));
    if(!_builder) {
      std::cerr << "init tensorrt yolo  detector - createInferBuilder failed!" << std::endl;
      return;
    }

    const auto explicitBatch = 1U << static_cast<uint32_t>(NetworkDefinitionCreationFlag::kEXPLICIT_BATCH);
    auto _network = SampleUniquePtr<nvinfer1::INetworkDefinition>(_builder->createNetworkV2(explicitBatch));
    if(!_network) {
      std::cerr << "init tensorrt yolo  detector - create network failed!" << std::endl;
      return;
    }

    auto _config = SampleUniquePtr<nvinfer1::IBuilderConfig>(_builder->createBuilderConfig());
    if(!_config) {
      std::cerr << "init tensorrt yolo  detector - createBuilderConfig failed!" << std::endl;
      return;
    }

    //解析onnx网络
    auto _parser = SampleUniquePtr<nvonnxparser::IParser>(nvonnxparser::createParser(*_network, sample::gLogger.getTRTLogger()));
    if(!_parser) {
      std::cerr << "init tensorrt yolo  detector - create Onnx Parser failed!" << std::endl;
      return;
    }
    std::cout << "parse onnx model file to tensorrt engine file ... 30min ~ 1hour ... be patient" << std::endl;
    _parser->parseFromFile(onnx_file_path.c_str(), static_cast<int>(sample::gLogger.getReportableSeverity()));

    //构造网络
    _config->setMaxWorkspaceSize(2 * (1ULL << 30)); //2G
    _config->setFlag(BuilderFlag::kFP16);
    samplesCommon::enableDLA(_builder.get(), _config.get(), -1); //对于Xavier NX 和 AGX 设备,还有 DLA 硬件可以调用

    // CUDA stream used for profiling by the builder.
    auto profileStream = samplesCommon::makeCudaStream();
    _config->setProfileStream(*profileStream);

    SampleUniquePtr<IHostMemory> _engine{ _builder->buildSerializedNetwork(*_network, *_config) };
    std::ofstream ofs(engine_file_path.c_str(), std::ios::out | std::ios::binary); //保存
    ofs.write((char*)(_engine->data()), _engine->size());
    ofs.close();

    std::cout << "init tensorrt yolo  detector model - covert and save engine file" << std::endl;
  }
}
