
#include <glog/logging.h>
#include <signal.h>

#include <QCoreApplication>
#include <QEventLoop>
#include <QThread>
#include <QVideoFrame>
#include <iostream>
#include <thread>

#include "qtio/qt_io.hpp"

qt_video_player::qt_video_player(std::string uri, int32_t width, int32_t height)
    : m_is_running(true),
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

  int64_t height = image.height();
  int64_t width = image.width();

  QImage image2 = image.convertToFormat(QImage::Format_RGB888);
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
  else
    m_source_type = VIDEO;

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
    case VIDEO:
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
