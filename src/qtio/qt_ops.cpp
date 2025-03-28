#include <dlpack.h>
#include <stdio.h>

#include <QImage>
#include <iostream>
#include <memory>

#include "functional_dag/dag_interface.hpp"
#include "functional_dag/guid_generated.h"
#include "functional_dag/guid_impl.hpp"
#include "functional_dag/lib_spec_generated.h"
#include "functional_dag/libutils.h"

typedef enum { UNDEFINED, RESIZE } OP;
const static fn_dag::GUID<fn_dag::node_prop_spec> __qt_op_guid =
    *fn_dag::GUID<fn_dag::node_prop_spec>::from_uuid(
        "1133483f-06b8-41e3-9a72-098cb4cd71b4");

class qt_op : public fn_dag::dag_node<DLTensor, DLTensor> {
 private:
  OP op_code;
  uint32_t m_width;
  uint32_t m_height;

 public:
  qt_op(OP op, int32_t width, int32_t height)
      : op_code(op), m_width(width), m_height(height) {}

  ~qt_op() {}

  std::unique_ptr<DLTensor> update(const DLTensor *input_dltensor_) {
    QImage input_image((uchar *)input_dltensor_->data,
                       input_dltensor_->shape[2], input_dltensor_->shape[1],
                       QImage::Format_RGB888);
    QImage output_image;
    switch (op_code) {
      case RESIZE:
        if (m_width > 0 && m_height > 0) {
          output_image =
              input_image.scaled(m_width, m_height, Qt::IgnoreAspectRatio,
                                 Qt::SmoothTransformation);
        }
        break;
      default:
        return nullptr;  // Stop the flow since the op code is unsupported.
    }

    uint8_t *bits = output_image.bits();
    std::unique_ptr<DLTensor> output_tensor(new DLTensor);
    output_tensor->device.device_type = DLDeviceType::kDLCPU;
    output_tensor->ndim = 3;
    output_tensor->shape = new int64_t[]{3, m_height, m_width};
    output_tensor->strides = NULL;
    output_tensor->byte_offset = 0;
    output_tensor->dtype.code = DLDataTypeCode::kDLUInt;
    output_tensor->dtype.bits = 8;
    output_tensor->dtype.lanes = 3;
    output_tensor->data = new uint8_t[m_width * m_height * 3];
    memcpy(output_tensor->data, bits, m_width * m_height * 3 * sizeof(uint8_t));
    return output_tensor;
  }
};

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wreturn-type-c-linkage"
#endif
#define DL_EXPORT __attribute__((visibility("default")))

extern "C" DL_EXPORT fn_dag::library_spec get_library_details() {
  // All UUID conversions should always work if it ever worked at all.
  fn_dag::library_spec spec{
      .guid = *fn_dag::GUID<fn_dag::library>::from_uuid(
          "dbf2c095-b757-47a0-af94-d795354a6179"),
      .available_nodes = {},
  };

  // This is the operators that we support
  spec.available_nodes.push_back(fn_dag::node_prop_spec{
      .guid = __qt_op_guid,
      .name = "Image ops",
      .description = "Qt based image processing operator.",
      .module_type = fn_dag::NODE_TYPE::NODE_TYPE_FILTER,
      .construction_types =
          {
              fn_dag::option_spec{
                  .type = fn_dag::OPTION_TYPE::OPTION_TYPE_INT,
                  .name = "operator",
                  .option_prompt = "Operator to run on the image. This is "
                                   "typically something basic. See description "
                                   "for potential options.",
                  .short_description = "Think of this like an OpenCV operator. "
                                       "Supported options are (1) RESIZE IMAGE",
              },
              fn_dag::option_spec{
                  .type = fn_dag::OPTION_TYPE::OPTION_TYPE_INT,
                  .name = "width",
                  .option_prompt = "Width of the image to resize to.",
                  .short_description = "Width of the image to resize to. Does "
                                       "nothing if value is negative.",
              },
              fn_dag::option_spec{
                  .type = fn_dag::OPTION_TYPE::OPTION_TYPE_INT,
                  .name = "height",
                  .option_prompt = "Height of the image to resize to.",
                  .short_description = "Height of the image to resize to. Does "
                                       "nothing if value is negative.",
              },
          },
  });
  return spec;
}

extern "C" DL_EXPORT bool construct_node(
    fn_dag::dag_manager<std::string> &manager, const fn_dag::node_spec &spec) {
  // Make sure the node is what we expected
  if (__qt_op_guid.m_id.bits1() == spec.target_id()->bits1() &&
      __qt_op_guid.m_id.bits2() == spec.target_id()->bits2()) {
    // Get the name and options for instantiation
    std::string_view name = spec.name()->string_view();
    OP op_code = OP::UNDEFINED;
    int width = -1;
    int height = -1;
    for (auto option : *spec.options()) {
      if (option->value()->type() == fn_dag::OPTION_TYPE_INT) {
        if (option->name()->string_view() == "operator") {
          op_code = static_cast<OP>(option->value()->int_value());
        } else if (option->name()->string_view() == "width") {
          width = option->value()->int_value();
        } else if (option->name()->string_view() == "height") {
          height = option->value()->int_value();
        }
      }
    }

    // Now get the parent node name
    if (spec.wires()->size() > 0) {
      std::string_view parent_node_name =
          spec.wires()->Get(0)->value()->string_view();
      qt_op *new_filter = new qt_op(op_code, width, height);
      if (auto parent_ret = manager.add_node(std::string(name), new_filter,
                                             std::string(parent_node_name));
          parent_ret.has_value()) {
        return parent_node_name == *parent_ret;
      } else {
        delete new_filter;
        std::cout << "Error: Could not add node to the dag. Error code: "
                  << parent_ret.error() << std::endl;
      }
    } else {
      std::cout << "Error: No parent node name found for in options for "
                << name << std::endl;
    }
  }
  return false;
}
