#include "vin/viz/ImageView.hpp"
#include "functional_dag/lib_utils.h"
#include <QHBoxLayout>
#include <QCoreApplication>

using namespace vin;
class QtViewer : public fn_dag::module_transmit {
  public:
  QtViewer(): frame(nullptr), imagePanel(nullptr) {
    // QApplication *app = (QApplication *)QCoreApplication::instance();
    // QMetaObject::invokeMethod(app, "start", Qt::QueuedConnection, Q_ARG());
    QMetaObject::invokeMethod(qApp, [this](){
      this->start();
    });
    // start();
    }
  void start() {
    frame = new QFrame();
    QHBoxLayout *lay = new QHBoxLayout(frame);
    
    
    
    imagePanel = new ImageView();
    lay->addWidget(imagePanel);

    frame->setMinimumSize(800, 600);
    frame->show();
  }

  DLTensor *update(const DLTensor *image);
  private:
  QFrame *frame;
  ImageView *imagePanel;
};

DLTensor *QtViewer::update(const DLTensor *image) {
  if(image != nullptr && image->ndim == 3) {
    switch(image->shape[0]) {
    case 1:
      imagePanel->setTensor(*image, VizMode::VIZ_HEATMAP);
      break;
    case 3:
      imagePanel->setTensor(*image, VizMode::VIZ_RGB);
      break;
    default:
      std::cerr << "Unable to visualize image with n_channels: " << image->shape[0] << std::endl;
    }
    // imagePanel->drawBox(100, 100, 50, 50);
  } else if(image != nullptr && image->ndim == 2 && image->dtype.code == DLDataTypeCode::kDLUInt) {
    // Each row is a bounding box
    int64_t stride = image->strides[0];
    int32_t *data_ptr = reinterpret_cast<int32_t*>(image->data);
    
    imagePanel->clearBoxOverlay();
    for(int64_t row = 0;row < image->shape[0];row++) {
      int x = data_ptr[row*stride];
      int y = data_ptr[row*stride+1];
      int width = data_ptr[row*stride+2];
      int height = data_ptr[row*stride+3];
      imagePanel->drawBox(x, y, width, height);
    }
  }else
    std::cerr << "Cannot visualize image. Invalid DLTensor in!\n";
  return nullptr;
}

#pragma GCC diagnostic ignored "-Wreturn-type-c-linkage"

#define DL_EXPORT __attribute__ ((visibility ("default")))

extern "C" DL_EXPORT std::string get_simple_description() {
  return std::string("This package will take an image from the DAG and visualize just the image in a simple window.");
}

extern "C" DL_EXPORT std::string get_detailed_description() {
  return std::string("This library has no options, just attaches to an image publisher and views the image.");
}

extern "C" DL_EXPORT std::string get_name() {
  return std::string("Image viewer I/O");
}

extern "C" DL_EXPORT long get_serial_guid() {
  return 1369;
}

extern "C" DL_EXPORT bool is_source() {
  return false;
}

extern "C" DL_EXPORT shared_ptr<fn_dag::lib_options> get_options() {
  shared_ptr<fn_dag::lib_options> options(new fn_dag::lib_options());
  
  return options;
}

extern "C" DL_EXPORT fn_dag::module *get_module(const fn_dag::lib_options *options) {
  (void)options;
  fn_dag::module_handler *viewer_out = new fn_dag::module_handler(new QtViewer());
  return (fn_dag::module *)viewer_out;
}