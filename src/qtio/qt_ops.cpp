#include <iostream>
#include <future>
#include <stdio.h>

#include <QThread>
#include <QApplication>
#include <QEventLoop>
#include <QCoreApplication>
#include <QVideoFrame>

#include "functional_dag/lib_utils.h"

typedef enum {
  RESIZE
} OP;

class qt_op : public fn_dag::module_transmit {
private:
  OP op_code;
  uint32_t m_width;
  uint32_t m_height;
  
public:
  qt_op(OP op, int32_t width, int32_t height) :
        op_code(op),
        m_width(width),
        m_height(height)
  {}

  ~qt_op() {}

  std::vector<std::string> const get_available_slots() {
    std::vector<std::string> slots;
    slots.push_back("default");
    return slots;
  }

  DLTensor *update(const DLTensor *input_dltensor_) {
    QImage input_image((uchar *)input_dltensor_->data, 
                        input_dltensor_->shape[2], 
                        input_dltensor_->shape[1], 
                        QImage::Format_RGB888);
    QImage output_image;
    bool skip = false;
    switch(op_code) {
      case RESIZE:
        if(m_width > 0 && m_height > 0) {
          output_image = input_image.scaled(m_width, 
                                            m_height,
                                            Qt::IgnoreAspectRatio,
                                            Qt::SmoothTransformation);
        }
        break;
      default:
        skip = true;
    }

    if(skip)
      return const_cast<DLTensor *>(input_dltensor_);
    else {
      uint8_t *bits = output_image.bits();
      DLTensor *output_tensor = new DLTensor;
      output_tensor->device.device_type = DLDeviceType::kDLCPU;
      output_tensor->ndim = 3;
      output_tensor->shape = new int64_t[]{3,m_height,m_width};
      output_tensor->strides = NULL;
      output_tensor->byte_offset = 0;
      output_tensor->dtype.code = DLDataTypeCode::kDLUInt;
      output_tensor->dtype.bits = 8;
      output_tensor->dtype.lanes = 3;
      output_tensor->data = new uint8_t[m_width*m_height*3];
      memcpy(output_tensor->data, bits, m_width*m_height*3*sizeof(uint8_t));
      return output_tensor;
    }

    return nullptr;
  }
};

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wreturn-type-c-linkage"
#endif

#define DL_EXPORT __attribute__ ((visibility ("default")))

extern "C" DL_EXPORT std::string get_simple_description() {
  return std::string("This library will use QT for simple image ops.");
}

extern "C" DL_EXPORT std::string get_detailed_description() {
  return std::string("This library uses Qt for image ops since Qt is available and does simple image operations.");
}

extern "C" DL_EXPORT std::string get_name() {
  return std::string("QT Image Op");
}

extern "C" DL_EXPORT long get_serial_guid() {
  return 24681;
}

extern "C" DL_EXPORT bool is_source() {
  return false;
}

extern "C" DL_EXPORT fn_dag::lib_options get_options() {
  return fn_dag::lib_options();
}

extern "C" DL_EXPORT fn_dag::module *get_module(const fn_dag::lib_options *options) {
  if(options->size() != 2)
    return nullptr;

  int32_t width, height;

  for(auto option : *options) {
    switch(option.serial_id) {
      case 9011:
        width = option.value.int_value;
        break;
      case 9012:
        height = option.value.int_value;
        break;
    }
  }

  fn_dag::module_handler *vlc_out = new fn_dag::module_handler(new qt_op(OP::RESIZE, width, height));
  return (fn_dag::module *)vlc_out;
}
