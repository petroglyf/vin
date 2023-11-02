#include <iostream>
#include <future>
#include <stdio.h>

#include <QThread>
#include <QApplication>
#include <QEventLoop>
#include <QCoreApplication>
#include <QVideoFrame>

#include "vin/viz/qt_io.hpp"

player_event_thread::player_event_thread(std::string uri, int32_t width, int32_t height) :
m_player(nullptr),
m_surface_for_player(nullptr),
m_specified_url(uri),
m_width(width),
m_height(height),
m_mutex{}
{
  start();
}

void stateChanged(QMediaPlayer::PlaybackState newState) {
std::cout << "Playback state changed: " << newState << std::endl;
}

void errorOccurred(QMediaPlayer::Error error, const QString &errorString) {
if(error != QMediaPlayer::Error::NoError) {
std::cout << "ERROR: " << errorString.toStdString() << std::endl;
}
}

static int ic = 0;
void player_event_thread::videoFrameChanged(const QVideoFrame &frame) {
// std::cout << "Frame changed!\n";
const QImage &imageSmall = frame.toImage();

// std::cout << "Width: " << m_width << std::endl;
// std::cout << "Height: " << m_height << std::endl;
QImage image;
if(m_width > 0 && m_height > 0) {
// std::cout << "Scaling\n";
image = imageSmall.scaled(m_width, 
m_height,
Qt::IgnoreAspectRatio,
Qt::SmoothTransformation);
} else {
image = imageSmall;
}

std::string filename = "img " + std::to_string(ic) + ".jpg";
ic++;
  

std::lock_guard<std::mutex> lock(m_mutex);

int64_t height = image.height();
int64_t width = image.width();
// std::cout << "widthin: " << width << std::endl;
// std::cout << "heightin: " << height << std::endl;
uint8_t rgb = 3;
// if(output_tensor.sizes()[0] == 0)
//   output_tensor = output_tensor.reshape({height, width});
// std::cout << "w: " << width << " h: " << height << std::endl;

QImage image2 = image.convertToFormat(QImage::Format_RGB888);

uint8_t *bits = image2.bits();
// auto options = torch::TensorOptions().dtype(torch::kUInt8);
// output_tensor = torch::from_blob(bits, {height, width}, options);
// unsigned char *img_data = output_tensor.data_ptr<unsigned char>();
// memcpy(img_data, image.bits(), sizeof(unsigned char)*3*width*height);
DLTensor *output_tensor = new DLTensor;
output_tensor->device.device_type = DLDeviceType::kDLCPU;
output_tensor->ndim = 3;
output_tensor->shape = new int64_t[]{rgb,height,width};
output_tensor->strides = NULL;
output_tensor->byte_offset = 0;
output_tensor->dtype.code = DLDataTypeCode::kDLUInt;
output_tensor->dtype.bits = 8;
output_tensor->dtype.lanes = 3;
output_tensor->data = new uint8_t[width*height*rgb];
memcpy(output_tensor->data, bits, width*height*rgb*sizeof(uint8_t));

// unsigned char* img_data;
// height = output_tensor.size(0);
// width = output_tensor.size(1);

// std::cout << "2w: " << width << " h: " << height << std::endl;
// img_data = output_tensor.data_ptr<unsigned char>();
// QImage imageq(img_data, width, height, sizeof(unsigned char)*3*width, QImage::Format_RGB888);
// imageq.save(QString(filename.c_str()));

// std::lock_guard<std::mutex> lock(m);
// m_frame_queue.push(new torch::Tensor(torch::from_blob(bits, {height, width}, options)));
m_frame_queue.push(output_tensor);
m_condition.notify_one();
}

void mediaStatusChanged(QMediaPlayer::MediaStatus status) {
std::cout << "Media status changed: " << status << std::endl;
}

void player_event_thread::run()
{
  m_player = new QMediaPlayer;

// QObject::connect(m_player, &QMediaPlayer::positionChanged, 
//                  positionChanged2);
// QObject::connect(m_player, &QMediaPlayer::positionChanged, 
//                  this, &player_event_thread::positionChanged);
QObject::connect(m_player, &QMediaPlayer::playbackStateChanged, 
stateChanged);
QObject::connect(m_player, &QMediaPlayer::mediaStatusChanged, 
mediaStatusChanged);
QObject::connect(m_player, &QMediaPlayer::errorOccurred, 
errorOccurred);
  
QUrl resource = QUrl::fromLocalFile((m_specified_url).c_str());
  
m_player->setSource(resource);

QVideoSink *sink = new QVideoSink(m_player);
m_player->setVideoSink(sink);

// QObject::connect(sink, &QVideoSink::videoFrameChanged, 
//                  this, &player_event_thread::videoFrameChanged);
QObject::connect(sink, &QVideoSink::videoFrameChanged, 
[this](const QVideoFrame &frame) {
// std::cout << "Just a test\n";
this->videoFrameChanged(frame);
});
// QMetaObject::invokeMethod(qApp, [this](){
//   m_player->play();
//   exec();
// });
  m_player->play();
  // QEventLoop loop;
  exec();
  // loop.exec();
std::cout << "DO NOT EXIT!!!!\n";
}

player_event_thread::~player_event_thread() {
if(m_player != nullptr) {
delete m_player;
m_player = nullptr;
}
}

DLTensor *player_event_thread::update() {
std::string filename = "img " + std::to_string(ic) + ".jpg";
ic++;

std::cout << "starting the update thread!\n";

// unsigned char* img_data;
// int height = output_tensor.size(0);
// int width = output_tensor.size(1);

// std::cout << "2w: " << width << " h: " << height << std::endl;
// img_data = output_tensor.data_ptr<unsigned char>();
// QImage imageq(img_data, width, height, sizeof(unsigned char)*3*width, QImage::Format_RGB888);
// imageq.save(QString(filename.c_str()));

std::unique_lock<std::mutex> lock(m_mutex);

while(m_frame_queue.empty())
{
    std::cout << "Waiting for queue\n";
// release lock as long as the wait and reaquire it afterwards.
m_condition.wait(lock);
std::cout << "Continuing\n";
}
  // std::cout <<" POPPING!!!\n";
DLTensor *val = m_frame_queue.front();
m_frame_queue.pop();
// return new torch::Tensor(output_tensor);
std::cout << "source update called\n";
return val;
// tensor_promise = std::move(tensor_promise);
// std::future<torch::Tensor *> tensor_future = tensor_promise.get_future();
// std::cout << "Waiting\n";
// return tensor_future.get();

// return nullptr;
}



#pragma GCC diagnostic ignored "-Wreturn-type-c-linkage"

#define DL_EXPORT __attribute__ ((visibility ("default")))

extern "C" DL_EXPORT std::string get_simple_description() {
  return std::string("This library will load video files with QT multimedia and push it through the DAG.");
}

extern "C" DL_EXPORT std::string get_detailed_description() {
  return std::string("This library will load video files with QT multimedia and push it through the DAG. DETAILED");
}

extern "C" DL_EXPORT std::string get_name() {
  return std::string("QT Multimedia input");
}

extern "C" DL_EXPORT long get_serial_guid() {
  return 24680;
}

extern "C" DL_EXPORT bool is_source() {
  return true;
}

extern "C" DL_EXPORT shared_ptr<fn_dag::lib_options> get_options() {
  shared_ptr<fn_dag::lib_options> options(new fn_dag::lib_options());
  fn_dag::construction_option optionFilePath{fn_dag::STRING, {""}, 9185, "Local location of file", "Specify the file location of the video source."};
  fn_dag::construction_option optionWidth{fn_dag::INT, {"640"}, 9011, "Width of output image", "Specify the width of the output image in pixels. Set either of these to zero to keep the source resolution."};
  fn_dag::construction_option optionHeight{fn_dag::INT, {"480"}, 9012, "Height of output image", "Specify the height of the output image in pixels. Set either of these to zero to keep the source resolution."};

  options->push_back(optionFilePath);
  options->push_back(optionWidth);
  options->push_back(optionHeight);
  return options;
}

extern "C" DL_EXPORT fn_dag::module *get_module(const fn_dag::lib_options *options) {
  if(options->size() != 3)
    return nullptr;

  std::string file_path;
  int32_t width, height;

  for(auto option : *options) {
    switch(option.serial_id) {
      case 9185:
        if(option.type == fn_dag::OPTION_TYPE::STRING)
          file_path = option.value.string_value;
        break;
      case 9011:
        width = option.value.int_value;
        break;
      case 9012:
        height = option.value.int_value;
        break;
    }
  }
  
  fn_dag::source_handler *vlc_out = new fn_dag::source_handler(new player_event_thread(file_path, width, height));
  return (fn_dag::module *)vlc_out;
}