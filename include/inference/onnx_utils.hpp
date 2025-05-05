#pragma once
#include <onnxruntime/onnxruntime_cxx_api.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <arrow/api.h>
#pragma GCC diagnostic pop

namespace onnx_utils {

typedef enum { CPU, CUDA, COREML } __provider;

class __onnx_session_handles {
 public:
  __onnx_session_handles();
  ~__onnx_session_handles();

  Ort::Env env;
  std::unique_ptr<Ort::Session> session;
  std::unique_ptr<Ort::MemoryInfo> memory_info;
  __provider provider;
};

/**
 * Load a tensor from a file and return the result as a DLPack tensor
 */
std::unique_ptr<arrow::Tensor> loadTensor(const char *const filename);

/**
 * Load a tensor from a file and return the result as a ONNX Runtime Value
 */
// Ort::Value loadTensorToONNX(const char *const _filename,
//                             const char *const _paramName,
//                             std::shared_ptr<__onnx_session_handles>
//                             _context);

/**
 *  Load the weights and model from a file.
 *
 *  @param filename The file path for the file to load
 *  @return The model that can be used for inference
 */
std::unique_ptr<__onnx_session_handles> loadModel(std::string purpose,
                                                  std::string file_path);

/**
 *  Convert a DLPack Tensor to an ORT Tensor
 *
 *  @param _tensor Tensor to convert
 *  @param _memory_info Memory info to use for the conversion
 *  @param expand_for_batch If true, the tensor will be expanded to support a
 * batch dimension
 *  @return That same tensor as an onnx runtime value
 */
Ort::Value arrow_to_onnx(arrow::Tensor &_tensor,
                         const Ort::MemoryInfo &_memory_info,
                         bool expand_for_batch = false);
}  // namespace onnx_utils
