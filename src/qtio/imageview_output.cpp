#include <QCoreApplication>
#include <QHBoxLayout>
#include <iostream>

#include "dlpack.h"
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

std::unique_ptr<DLTensor> QtViewer::update(const DLTensor *image) {
  if (imagePanel == nullptr) return nullptr;

  if (image != nullptr && image->ndim == 3) {
    switch (image->shape[0]) {
      case 1:
        imagePanel->set_tensor(*image, visualization_mode::VIZ_HEATMAP);
        break;
      case 3:
        imagePanel->set_tensor(*image, visualization_mode::VIZ_RGB);
        break;
      default:
        std::cerr << "Unable to visualize image with n_channels: "
                  << image->shape[0] << std::endl;
    }
  } else if (image != nullptr && image->ndim == 2 &&
             image->dtype.code == DLDataTypeCode::kDLUInt) {
    if (image->shape[1] == 2) {
      std::vector<std::tuple<uint32_t, uint32_t> > goal_points;
      for (auto i = 0; i < image->shape[0]; i++) {
        uint32_t x = reinterpret_cast<uint32_t *>(image->data)[i * 2];
        uint32_t y = reinterpret_cast<uint32_t *>(image->data)[i * 2 + 1];
        goal_points.push_back(std::make_tuple(x, y));
      }
      imagePanel->set_gaze_pts(goal_points);
    } else {
      // Each row is a bounding box
      int64_t stride = image->strides[0];
      int32_t *data_ptr = reinterpret_cast<int32_t *>(image->data);

      imagePanel->clear_overlay();
      for (int64_t row = 0; row < image->shape[0]; row++) {
        int x = data_ptr[row * stride];
        int y = data_ptr[row * stride + 1];
        int width = data_ptr[row * stride + 2];
        int height = data_ptr[row * stride + 3];
        imagePanel->draw_box(x, y, width, height);
      }
    }
  } else
    std::cerr << "Cannot visualize image. Invalid DLTensor in!\n";
  return nullptr;
}
