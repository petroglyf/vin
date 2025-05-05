#include "inference/onnx_utils.hpp"

#include "arrow/type_fwd.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <arrow/io/compressed.h>  // Include the correct header for CompressedInputStream
#include <arrow/ipc/reader.h>  // Include the correct header for ReadTensor
#include <arrow/util/compression.h>  // Include the correct header for Codec
#pragma GCC diagnostic pop

#include <glog/logging.h>

#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "arrow/io/file.h"

namespace onnx_utils {

// We need to declare the constructor and destructor to trigger the destructors
// of our members
__onnx_session_handles::__onnx_session_handles() {}
__onnx_session_handles::~__onnx_session_handles() {}

// Load a compressed arrow tensor from an arrow file.
std::unique_ptr<arrow::Tensor> loadTensor(const char *const _filename) {
  LOG(INFO) << "Loading tensor from file: " << _filename;
  auto raw_stream_status = arrow::io::ReadableFile::Open(_filename);
  if (!raw_stream_status.ok()) {
    LOG(ERROR) << "Error opening file: " << _filename << std::endl;
    return nullptr;
  }
  auto raw_stream = raw_stream_status.ValueOrDie();
  auto codec =
      arrow::util::Codec::Create(arrow::Compression::GZIP).ValueOrDie();
  auto stream_wrapper_result =
      arrow::io::CompressedInputStream::Make(codec.get(), raw_stream);
  if (!stream_wrapper_result.ok()) {
    LOG(ERROR) << "Error creating compressed stream: "
               << stream_wrapper_result.status().ToString() << std::endl;
    return nullptr;
  }

  auto stream_wrapper = stream_wrapper_result.ValueOrDie();
  auto tensor_result = arrow::ipc::ReadTensor(&*stream_wrapper);
  if (!tensor_result.ok()) {
    LOG(ERROR) << "Error parsing tensor: " << tensor_result.status().ToString()
               << std::endl;
    return nullptr;
  }

  auto tensor_out = tensor_result.ValueOrDie();
  std::unique_ptr<arrow::Tensor> tensor_out_ptr =
      std::make_unique<arrow::Tensor>(
          tensor_out->type(), tensor_out->data(), tensor_out->shape(),
          tensor_out->strides(), tensor_out->dim_names());

  return tensor_out_ptr;
}

/**
 *  Load the weights and model from a file.
 *
 *  @param filename The file path for the file to load
 *  @return The model that can be used for inference
 */
std::unique_ptr<__onnx_session_handles> loadModel(std::string purpose,
                                                  std::string file_path) {
  std::unique_ptr<__onnx_session_handles> context =
      std::unique_ptr<__onnx_session_handles>(new __onnx_session_handles);

  /******* Create ORT environment *******/
  context->env =
      Ort::Env(OrtLoggingLevel::ORT_LOGGING_LEVEL_FATAL, purpose.c_str());

  /******* Create ORT session *******/
  // Set up options for session
  Ort::SessionOptions sessionOptions;
  context->provider = __provider::CPU;

  switch (context->provider) {
    case __provider::CPU:
      context->memory_info = std::make_unique<Ort::MemoryInfo>(
          Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator,
                                     OrtMemType::OrtMemTypeDefault));
      break;
    default:
      LOG(ERROR) << "Don't know how to allocate memory on device: "
                 << context->provider << std::endl;
  }

  // Sets graph optimization level (Here, enable all possible optimizations)
  sessionOptions.SetGraphOptimizationLevel(
      GraphOptimizationLevel::ORT_ENABLE_ALL);
  // Create session by loading the onnx model
  context->session = std::make_unique<Ort::Session>(
      Ort::Session(context->env, file_path.c_str(), sessionOptions));

  return context;
}

Ort::Value arrow_to_onnx(arrow::Tensor &_tensor,
                         const Ort::MemoryInfo &_memory_info,
                         bool expand_for_batch) {
  size_t tensor_size =
      std::accumulate(_tensor.shape().begin(), _tensor.shape().end(), 1,
                      std::multiplies<int>());

  std::vector<int64_t> final_shape;
  if (expand_for_batch) {
    final_shape.push_back(1);
  }
  final_shape.insert(final_shape.end(), _tensor.shape().begin(),
                     _tensor.shape().end());
  size_t ndims = final_shape.size();

  switch (_tensor.type()->id()) {
    case arrow::Type::INT8:  // signed integer
      return Ort::Value::CreateTensor<int8_t>(
          _memory_info, const_cast<int8_t *>(_tensor.data()->data_as<int8_t>()),
          tensor_size, final_shape.data(), ndims);
    case arrow::Type::UINT8:  // unsigned integer
      return Ort::Value::CreateTensor<uint8_t>(
          _memory_info, const_cast<uint8_t *>(_tensor.data()->data()),
          tensor_size, final_shape.data(), ndims);
    case arrow::Type::FLOAT:
      return Ort::Value::CreateTensor<float>(
          _memory_info, const_cast<float *>(_tensor.data()->data_as<float>()),
          tensor_size, final_shape.data(), ndims);
    case arrow::Type::DOUBLE:
      std::cout << "DOUBLE\n";
      return Ort::Value::CreateTensor<double>(
          _memory_info, const_cast<double *>(_tensor.data()->data_as<double>()),
          tensor_size, final_shape.data(), ndims);
    default:
      LOG(ERROR) << "Unknown type when converting from DLTensor to ONNX\n";
  }
  return Ort::Value(nullptr);
}
}  // namespace onnx_utils