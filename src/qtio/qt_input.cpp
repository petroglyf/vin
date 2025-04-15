
#include <glog/logging.h>
#include <signal.h>

#include <QCoreApplication>
#include <QEventLoop>
#include <QThread>
#include <QVideoFrame>
#include <iostream>
#include <thread>

#include "qtio/qt_io.hpp"

constexpr std::array<std::string, 4> m_supported_img_formats = {"jpg", "png",
                                                                "bmp", "jpeg"};

qt_video_player::qt_video_player(std::string uri, int32_t width, int32_t height)
    : m_source_type(UNDEFINED),
      m_is_running(true),
      m_camera(nullptr),
      m_capture(nullptr),
      m_player(nullptr),
      m_specified_url(uri),
      m_surface_for_player(nullptr),
      m_width(width),
      m_height(height),
      m_mutex{},
      m_frame_queue{} {
  start();
}

void playerErrorOccurred(QMediaPlayer::Error error,
                         const QString &errorString) {
  if (error != QMediaPlayer::Error::NoError) {
    LOG(ERROR) << "ERROR: " << errorString.toStdString() << std::endl;
  }
}

void cameraErrorOccurred(QCamera::Error error)  ///, const QString &errorString)
{
  if (error != QCamera::Error::NoError) {
    LOG(ERROR) << "ERROR: " << std::endl;
  }
}

void qt_video_player::frame_changed(const QVideoFrame &frame) {
  const QImage &imageSmall = frame.toImage();

  QImage image;
  if (m_width > 0 && m_height > 0) {
    image = imageSmall.scaled(m_width, m_height, Qt::IgnoreAspectRatio,
                              Qt::SmoothTransformation);
  } else {
    image = imageSmall;
  }

  std::lock_guard<std::mutex> lock(m_mutex);

  QImage image2 = image.convertToFormat(QImage::Format_RGB888);
  if (m_width > 0 && m_height > 0) {
    image2 = image2.scaled(m_width, m_height, Qt::IgnoreAspectRatio,
                           Qt::SmoothTransformation);
  }
  int64_t height = image.height();
  int64_t width = image.width();

  uint8_t *bits = image2.bits();

  auto buffer = std::make_shared<arrow::Buffer>(bits, width * height * 3);
  std::vector<int64_t> shape = {3, height, width};

  m_frame_queue.push(
      std::make_unique<arrow::Tensor>(arrow::uint8(), buffer, shape));
  m_condition.notify_one();
}

void qt_video_player::run() {
  if (m_specified_url == "camera")
    m_source_type = CAMERA;
  else {
    std::string ext =
        m_specified_url.substr(m_specified_url.find_last_of(".") + 1);
    if (std::find(m_supported_img_formats.begin(),
                  m_supported_img_formats.end(),
                  ext) != m_supported_img_formats.end()) {
      m_source_type = IMAGE;
    } else {
      m_source_type = VIDEO;
    }
  }

  switch (m_source_type) {
    case CAMERA: {
      m_camera = new QCamera(QCameraDevice::UnspecifiedPosition);
      const auto settings = m_camera->cameraDevice().videoFormats();
      auto s = settings.at(1);
      m_camera->setCameraFormat(s);

      m_capture = new QMediaCaptureSession;
      m_surface_for_player = new QVideoSink(m_capture);

      QObject::connect(m_camera, &QCamera::errorOccurred, cameraErrorOccurred);
      m_capture->setCamera(m_camera);
      m_camera->setExposureMode(QCamera::ExposureMode::ExposureAuto);
      m_capture->setVideoSink(m_surface_for_player);

      QObject::connect(
          m_surface_for_player, &QVideoSink::videoFrameChanged,
          [this](const QVideoFrame &frame) { this->frame_changed(frame); });
      m_camera->start();
    } break;
    case IMAGE: {
      m_image = QImage(m_specified_url.c_str())
                    .convertToFormat(QImage::Format_RGB888);
      if (m_width > 0 && m_height > 0) {
        m_image = m_image.scaled(m_width, m_height, Qt::IgnoreAspectRatio,
                                 Qt::SmoothTransformation);
      } else {
        m_width = m_image.width();
        m_height = m_image.height();
      }
    } break;
    case VIDEO: {
      m_player = new QMediaPlayer;

      QObject::connect(m_player, &QMediaPlayer::mediaStatusChanged,
                       [this](QMediaPlayer::MediaStatus status) {
                         //  mediaStatusChanged(status);
                         if (status == QMediaPlayer::MediaStatus::EndOfMedia) {
                           LOG(INFO) << "End of media. Exiting." << std::endl;
                           kill(getpid(), SIGHUP);
                           this->m_is_running = false;
                           this->m_condition.notify_one();
                         }
                       });
      QObject::connect(m_player, &QMediaPlayer::errorOccurred,
                       playerErrorOccurred);

      QUrl resource = QUrl::fromLocalFile((m_specified_url).c_str());

      m_player->setSource(resource);

      m_surface_for_player = new QVideoSink(m_player);
      m_player->setVideoSink(m_surface_for_player);

      QObject::connect(
          m_surface_for_player, &QVideoSink::videoFrameChanged,
          [this](const QVideoFrame &frame) { this->frame_changed(frame); });
      LOG(INFO) << "Playing video: " << m_specified_url << std::endl;
      m_player->play();
    } break;
    default: {
      break;
    }
  }
  exec();
}

qt_video_player::~qt_video_player() {
  requestInterruption();
  exit(555);
  wait(1000);

  if (m_player != nullptr) {
    m_player = nullptr;
  }

  if (m_camera != nullptr) {
    m_camera = nullptr;
  }
}

std::unique_ptr<arrow::Tensor> qt_video_player::update() {
  if (m_source_type == UNDEFINED) return nullptr;
  if (m_source_type == IMAGE) {
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    if (m_image.isNull()) {
      return nullptr;
    }
    QImage image = m_image.convertToFormat(QImage::Format_RGB888);
    uint8_t *bits = image.bits();

    auto buffer = std::make_shared<arrow::Buffer>(bits, m_width * m_height * 3);
    std::vector<int64_t> shape = {3, m_height, m_width};
    return std::make_unique<arrow::Tensor>(arrow::uint8(), buffer, shape);
  }

  // Video or camera type here --
  std::unique_lock<std::mutex> lock(m_mutex);
  if (!m_is_running) {
    return nullptr;
  }

  while (m_frame_queue.empty()) {
    // release lock as long as the wait and reaquire it afterwards.
    m_condition.wait(lock);
    if (!m_is_running) {
      return nullptr;
    }
  }
  std::unique_ptr<arrow::Tensor> val;
  val = std::move(m_frame_queue.front());
  m_frame_queue.pop();

  return val;
}
