#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <arrow/api.h>
#pragma GCC diagnostic pop
#include <functional_dag/guid_generated.h>
#include <functional_dag/lib_spec_generated.h>
#include <functional_dag/libutils.h>
#include <glog/logging.h>

#include <functional_dag/dag_interface.hpp>
#include <functional_dag/guid_impl.hpp>

#include "inference/onnx_utils.hpp"

const static fn_dag::GUID<fn_dag::node_prop_spec> __onnx_node_guid =
    *fn_dag::GUID<fn_dag::node_prop_spec>::from_uuid(
        "e9f7db90-271d-419c-a964-afbf06188e96");

namespace vin {
class ONNXNode : public fn_dag::dag_node<arrow::Tensor, arrow::Tensor> {
 private:
  static constexpr uint32_t expected_buffer_size = 3 * 768 * 1024;
  std::unique_ptr<onnx_utils::__onnx_session_handles> m_context;
  std::unique_ptr<arrow::Tensor> m_supporting_tensor;
  Ort::Value m_supporting_tensor_onnx;

  std::string m_path_to_model;
  std::string m_path_to_supporting_file;
  std::string m_supporting_param_name;
  std::vector<const char *> m_input_names;
  std::vector<const char *> m_output_names;

  std::vector<Ort::Value> m_output_tensors;
  std::unique_ptr<OrtAllocator> m_allocator;
  std::shared_ptr<arrow::Buffer> m_output_buffer;
  std::vector<int64_t> m_output_shape;

 public:
  ONNXNode(std::string name_of_node, std::string path_to_model,
           std::string path_to_supporting_file,
           std::string supporting_param_name)
      : m_path_to_model(path_to_model),
        m_path_to_supporting_file(path_to_supporting_file),
        m_supporting_param_name(supporting_param_name),
        m_allocator(nullptr) {
    // Load the bias input (it's a constant for DeepGaze)
    m_supporting_tensor =
        onnx_utils::loadTensor(path_to_supporting_file.c_str());

    // Load the model and weights
    m_context = onnx_utils::loadModel(name_of_node, path_to_model);

    // Get an allocator for allocating memory
    OrtAllocator *allocator = nullptr;
    if (auto status = Ort::GetApi().CreateAllocator(
            *m_context->session, *m_context->memory_info, &allocator);
        status != nullptr) {
      throw std::runtime_error(
          "Failed to create allocator: " +
          std::string(Ort::GetApi().GetErrorMessage(status)));
    }

    m_allocator = std::unique_ptr<OrtAllocator>(allocator);

    // input image
    m_supporting_tensor_onnx = onnx_utils::arrow_to_onnx(
        *m_supporting_tensor, *m_context->memory_info);

    // Output images
    Ort::TypeInfo output_type_info = m_context->session->GetOutputTypeInfo(0);
    auto output_tensor_info = output_type_info.GetTensorTypeAndShapeInfo();
    ONNXTensorElementDataType output_type = output_tensor_info.GetElementType();
    std::vector<int64_t> output_dims = output_tensor_info.GetShape();

    switch (output_type) {
      case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT:
        m_output_tensors.push_back(Ort::Value::CreateTensor<float>(
            m_allocator.get(), output_dims.data(), output_dims.size()));
        break;
      case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8:
        m_output_tensors.push_back(Ort::Value::CreateTensor<uint8_t>(
            m_allocator.get(), output_dims.data(), output_dims.size()));
        break;
      case ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE:
        m_output_tensors.push_back(Ort::Value::CreateTensor<double>(
            m_allocator.get(), output_dims.data(), output_dims.size()));
        break;
      default:
        LOG(ERROR) << "Unsupported output type: " << output_type << std::endl;
        return;
    }
    size_t nbytes =
        m_output_tensors[0].GetTensorTypeAndShapeInfo().GetElementCount();
    m_output_buffer = std::make_shared<arrow::Buffer>(new uint8_t[nbytes],
                                                      nbytes * sizeof(uint8_t));
    m_output_shape.push_back(1);
    m_output_shape.insert(m_output_shape.end(), output_dims.begin(),
                          output_dims.end());

    // Set up the mapping between values
    m_input_names.push_back("x");
    m_input_names.push_back("onnx::Reshape_1");
    m_output_names.push_back("117");
  }

  std::unique_ptr<arrow::Tensor> update(const arrow::Tensor *image) {
    std::cout << "Got an image!\n";
    // input tensors
    Ort::Value inputTensors[] = {
        onnx_utils::arrow_to_onnx(*const_cast<arrow::Tensor *>(image),
                                  *m_context->memory_info, true),
        onnx_utils::arrow_to_onnx(*m_supporting_tensor,
                                  *m_context->memory_info)};

    // // Perform the inference
    try {
      m_context->session->Run(Ort::RunOptions{nullptr}, m_input_names.data(),
                              inputTensors, m_input_names.size(),
                              m_output_names.data(), m_output_tensors.data(),
                              m_output_names.size());
    } catch (const Ort::Exception &e) {
      std::cerr << "Error during inference: " << e.what() << std::endl;
      return nullptr;
    }
    // Extract the saliency map out
    const float *saliency_map = m_output_tensors.at(0).GetTensorData<float>();

    long nelements =
        m_output_tensors.at(0).GetTensorTypeAndShapeInfo().GetElementCount();

    for (int i = 0; i < nelements; i++) {
      float f = saliency_map[i];
      f = -f / 25.0;
      f = f * 255.0;
      // and for some reason it's inverted
      const_cast<uint8_t *>(m_output_buffer->data())[i] = 255 - (uint8_t)f;
    }

    auto output_tensor = std::make_unique<arrow::Tensor>(
        arrow::uint8(), m_output_buffer, m_output_shape);

    return output_tensor;
  }
};

}  // namespace vin

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wreturn-type-c-linkage"
#endif
#define DL_EXPORT __attribute__((visibility("default")))

extern "C" DL_EXPORT fn_dag::library_spec get_library_details() {
  // All UUID conversions should always work if it ever worked at all.
  fn_dag::library_spec spec{
      .guid = *fn_dag::GUID<fn_dag::library>::from_uuid(
          "6e1af8e3-0818-4fce-bf81-748a7c3704e9"),
      .available_nodes = {},
  };

  // This is the operators that we support
  spec.available_nodes.push_back(fn_dag::node_prop_spec{
      .guid = __onnx_node_guid,
      .name = "ONNX Inference Node",
      .description = "Run an onnx file on the GPU.",
      .module_type = fn_dag::NODE_TYPE::NODE_TYPE_FILTER,
      .construction_types =
          {
              fn_dag::option_spec{
                  .type = fn_dag::OPTION_TYPE::OPTION_TYPE_STRING,
                  .name = "model_filename",
                  .option_prompt = "ONNX model filepath",
                  .short_description = "This path must exist.",
              },
              fn_dag::option_spec{
                  .type = fn_dag::OPTION_TYPE::OPTION_TYPE_STRING,
                  .name = "supporting_tensor",
                  .option_prompt = "Path to suppporting tensor",
                  .short_description =
                      "If any tensors are needed to run the model, "
                      "this is the path to the tensor. This path must exist.",
              },
              fn_dag::option_spec{
                  .type = fn_dag::OPTION_TYPE::OPTION_TYPE_STRING,
                  .name = "tensor_param_name",
                  .option_prompt = "Param name to store the tensor",
                  .short_description =
                      "ONNX models take a parameter name for input tensors "
                      "into the forward function. Use this parameter to pass "
                      "the supporting tensor into the models forward pass.",
              },
          },
  });
  return spec;
}

extern "C" DL_EXPORT bool construct_node(
    fn_dag::dag_manager<std::string> &manager, const fn_dag::node_spec &spec) {
  std::string path_to_model;
  std::string path_to_supporting_file;
  std::string supporting_param_name;
  // Make sure the node is what we expected
  if (__onnx_node_guid.m_id.bits1() == spec.target_id()->bits1() &&
      __onnx_node_guid.m_id.bits2() == spec.target_id()->bits2()) {
    // Get the name and options for instantiation
    std::string_view name = spec.name()->string_view();
    for (auto option : *spec.options()) {
      if (option->value()->type() == fn_dag::OPTION_TYPE_STRING) {
        if (option->name()->string_view() == "model_filename") {
          path_to_model = option->value()->string_value()->str();
        } else if (option->name()->string_view() == "supporting_tensor") {
          path_to_supporting_file = option->value()->string_value()->str();
        } else if (option->name()->string_view() == "tensor_param_name") {
          supporting_param_name = option->value()->string_value()->str();
        }
      }
    }

    // Now get the parent node name
    if (spec.wires()->size() > 0) {
      std::string_view parent_node_name =
          spec.wires()->Get(0)->value()->string_view();

      vin::ONNXNode *new_filter =
          new vin::ONNXNode(spec.name()->str(), path_to_model,
                            path_to_supporting_file, supporting_param_name);
      if (auto parent_ret = manager.add_node(std::string(name), new_filter,
                                             std::string(parent_node_name));
          parent_ret.has_value()) {
        return parent_node_name == *parent_ret;
      } else {
        delete new_filter;
        LOG(ERROR) << "Error: Could not add node to the dag. Error code: "
                   << parent_ret.error() << std::endl;
      }
    } else {
      LOG(ERROR) << "Error: No parent node name found for in options for "
                 << name << std::endl;
    }
  }
  return false;
}
