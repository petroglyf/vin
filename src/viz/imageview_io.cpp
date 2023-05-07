#include "oculator/viz/ImageView.hpp"
#include "functional_dag/lib_utils.h"
#include <QHBoxLayout>
#include <QCoreApplication>

using namespace oculator;
class QtViewer : public fn_dag::module_transmit {
  public:
  QtViewer(): frame(nullptr), imagePanel(nullptr) {
    QApplication *app = (QApplication *)QCoreApplication::instance();
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
    std::cout << "SHOWING THREAD\n";
    frame->show();
  }

  DLTensor *update(const DLTensor *image);
  private:
  QFrame *frame;
  ImageView *imagePanel;
};

DLTensor *QtViewer::update(const DLTensor *image) {
  // std::cout << "RECEIVEDD<---------------\n";
  imagePanel->setTensor(*image, VizMode::VIZ_RGB);
  // frame->resize();
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
  fn_dag::module_handler *viewer_out = new fn_dag::module_handler(new QtViewer());
  return (fn_dag::module *)viewer_out;
}