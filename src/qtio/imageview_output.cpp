#include <glog/logging.h>

#include <QCoreApplication>
#include <QHBoxLayout>
#include <cstdint>
#include <iostream>

#include "qtio/image_view.hpp"

using namespace vin;

QtViewer::QtViewer() : frame(nullptr), imagePanel(nullptr) {
  // qApp is reserved and is inherited from QLabel
  QMetaObject::invokeMethod(qApp, [this]() {
    this->start();  // Start the widget thread so the callbacks work.
  });
}

void QtViewer::start() {
  frame = new QFrame();
  QHBoxLayout *lay = new QHBoxLayout(frame);
  imagePanel = new image_view();
  lay->addWidget(imagePanel);

  frame->setMinimumSize(800, 600);
  frame->show();
}

std::unique_ptr<int> QtViewer::update(const arrow::Tensor *image) {
  if (imagePanel == nullptr) return nullptr;

  if (image != nullptr && image->shape().size() == 3) {
    switch (image->shape()[0]) {
      case 1:
        imagePanel->set_tensor(*image, visualization_mode::VIZ_HEATMAP);
        break;
      case 3:
        imagePanel->set_tensor(*image, visualization_mode::VIZ_RGB);
        break;
      default:
        LOG(WARNING) << "Unable to visualize image with n_channels: "
                     << image->shape()[0] << std::endl;
    }
  } else if (image != nullptr && image->shape().size() == 2 &&
             image->type_id() == arrow::Type::UINT32) {
    std::vector<int64_t> f{0, 0};
    if (image->shape()[1] == 2) {
      std::vector<std::tuple<uint32_t, uint32_t> > goal_points;
      for (auto i = 0; i < image->shape()[0]; i++) {
        f[0] = i;
        f[1] = 0;
        uint32_t x = image->Value<arrow::UInt32Type>(f);
        f[1] = 1;
        uint32_t y = image->Value<arrow::UInt32Type>(f);
        goal_points.push_back(std::make_tuple(x, y));
      }
      imagePanel->set_gaze_pts(goal_points);
    } else {
      // Each row is a bounding box
      imagePanel->clear_overlay();
      for (int64_t row = 0; row < image->shape()[0]; row++) {
        f[0] = row;
        f[1] = 0;
        int x = image->Value<arrow::UInt32Type>(f);
        f[1] = 1;
        int y = image->Value<arrow::UInt32Type>(f);
        f[1] = 2;
        int width = image->Value<arrow::UInt32Type>(f);
        f[1] = 3;
        int height = image->Value<arrow::UInt32Type>(f);
        imagePanel->draw_box(x, y, width, height);
      }
    }
  } else
    LOG(ERROR) << "Cannot visualize image. Invalid DLTensor in!\n";
  return nullptr;
}
