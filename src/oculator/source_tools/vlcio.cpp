#include <vlc/vlc.h>
#include <mutex>
#include <functional>
#include <iostream>
#include <future>

#include "functional_dag/lib_utils.h"


static void update_fn(void *data, void *id)
{
  std::cout<< "Update called\n";
  /*if (!capture_.grab())
  {
    qWarning() << "Failed to grab frame";
    return;
  }
  cv::Mat frame;
  if (!capture_.retrieve(frame))
  {
    qWarning() << "Failed to retrieve frame";
    return;
  }
  target_->setMat(frame);
  saliency_->setMat(RunMR_AIM(frame, basis_, *mk_aim_, 0));
  */
}

static void *lock_output_fn(void *data, void **p_pixels) {
  return NULL;
}
static void unlock_output_fn(void *data, void *id, void *const *p_pixels) {

}
using namespace oculator;

class DeviceReader
{
public:
  DeviceReader(std::string uri);
  ~DeviceReader();
  
  bool is_playing();
private:
  libvlc_instance_t *m_vlc_instance;
  libvlc_media_player_t *m_vlc_player;
  bool m_is_initialized;
  std::mutex mutex;

  bool initializeVLC();
  std::function< void() > m_callback;
  void lock_output_(void *data, void **p_pixels);
  void update_(void *data, void *id);
  void unlock_output_(void *data, void *id, void *const *p_pixels);
};


DeviceReader::DeviceReader(std::string uri) :
                              // m_callback(callback),
                              m_vlc_instance(NULL),
                              m_vlc_player(NULL),
                              mutex{}
{
  m_is_initialized = initializeVLC();

  if(m_is_initialized) {
    libvlc_media_t *vlc_media = NULL;
    if (uri.find("http") == 0)
      vlc_media = libvlc_media_new_location (m_vlc_instance, uri.c_str());
    else {
      std::cout << "Opening file: " << uri << std::endl;
      vlc_media = libvlc_media_new_path (m_vlc_instance, uri.c_str() );
      if(vlc_media == NULL) {
        std::cout << "Failed to open file: " << uri << std::endl;
      } else {
        std::cout << "File opened with handle: " << uri << std::endl;
      }
      // Make it loop
      // libvlc_media_add_option(vlc_media, "input-repeat=-1");
    }
    // Check to see what kind of media we're dealing with and handle it appropriately
  
    // Make sure the media has a handle and continue
    if(vlc_media) {
      std::cout<< "Now let's create a player\n";
      // Create a player and free the media
      m_vlc_player = libvlc_media_player_new_from_media (vlc_media);
      
      // std::cout<< "Releasing media\n";
      // libvlc_media_release (vlc_media);
      auto leng = libvlc_media_player_get_length(m_vlc_player);
      std::cout << "Length : " << leng << std::endl;
      if(m_vlc_player == NULL) {
        std::cout << "Error creating a player\n";
      }
      std::cout<< "Setting callback\n";
      // Set the callbacks and format
      libvlc_video_set_callbacks(m_vlc_player, 
                          &lock_output_fn, 
                          &unlock_output_fn,
                          &update_fn,
                          NULL);
      libvlc_video_set_format(m_vlc_player, "RV16", 640, 480, 640*2);
      // play
      std::cout<< "Playing\n";
      int ret_code = libvlc_media_player_play(m_vlc_player);
      std::cout << "With return code: " << ret_code << std::endl;
    }
  }
}

DeviceReader::~DeviceReader() {
  /* Release libVLC instance on quit */
  if (m_vlc_player) {
    libvlc_media_player_stop(m_vlc_player);
    libvlc_media_player_release(m_vlc_player);
  }
  if (m_vlc_instance)
    libvlc_release(m_vlc_instance);
}
// VLC_PLUGIN_PATH=/opt/homebrew/opt/libvlc-3.0.16-arm/plugins ./oculator -u /Users/drobotnik/Movies/Project1_BottomLeft.mov
bool DeviceReader::initializeVLC() {
  char const *vlc_argv[] = {
        "--no-audio", /* skip any audio track */
        "--no-xlib", /* tell VLC to not use Xlib */
        // "--reset-plugins-cache",
        "-v",
        "2"  /* tell VLC to be verbose */
    };
  int vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);
  std::cout << "Starting vlc\n";
  /* Initialize libVLC */
  m_vlc_instance = libvlc_new(vlc_argc, vlc_argv);
  // m_vlc_instance = libvlc_new(0, NULL);
  
  if(m_vlc_instance == NULL) {
    const char *err_msg = libvlc_errmsg ();
    if(err_msg != NULL) {
      std::cerr << err_msg << std::endl;
    } else {
      std::cerr << "Critical failure: could not find libVLC!\n";
    }
    return false;
  }
  return true;
}

void DeviceReader::update_(void *data, void *id)
{
  std::cout<< "Update called\n";
  /*if (!capture_.grab())
  {
    qWarning() << "Failed to grab frame";
    return;
  }
  cv::Mat frame;
  if (!capture_.retrieve(frame))
  {
    qWarning() << "Failed to retrieve frame";
    return;
  }
  target_->setMat(frame);
  saliency_->setMat(RunMR_AIM(frame, basis_, *mk_aim_, 0));
  */
}

void DeviceReader::lock_output_(void *data, void **p_pixels) {
  std::cout << "lock called\n";
}
void DeviceReader::unlock_output_(void *data, void *id, void *const *p_pixels) {}

bool DeviceReader::is_playing() {
  if(m_is_initialized)
    return libvlc_media_player_is_playing(m_vlc_player);
  return false;
}






class VLCIO : public oculator::module_source {
  shared_ptr<DeviceReader> reader;
  std::promise<torch::Tensor *> tensor_promise;

public:
  VLCIO(const std::string file_path);
  ~VLCIO() {}

  // void update_tensor(torch::Tensor *new_tensor);
  torch::Tensor *update();
};

// void GSTIO::update_tensor(torch::Tensor *new_tensor) {
//   std::cout << "Got a callback from GST, setting value\n";
//   tensor_promise.set_value(new_tensor);
//   std::cout << "Going to get another frame\n";
// }

VLCIO::VLCIO(const std::string file_path) {
  DeviceReader *hook = new DeviceReader(file_path);

  reader = std::shared_ptr<DeviceReader>(hook);
}

torch::Tensor *VLCIO::update() {
  // std::cout << "Moving tensor and getting future\n";
  // tensor_promise = std::move(tensor_promise);
  // std::future<torch::Tensor *> tensor_future = tensor_promise.get_future();
  // std::cout << "Waiting\n";
  // return tensor_future.get();
  return nullptr;
}




#pragma GCC diagnostic ignored "-Wreturn-type-c-linkage"

#define DL_EXPORT __attribute__ ((visibility ("default")))

extern "C" DL_EXPORT std::string get_simple_description() {
  return std::string("This library will load video files with VLC and push it through the DAG.");
}

extern "C" DL_EXPORT std::string get_detailed_description() {
  return std::string("This library will load video files with VLC and push it through the DAG. DETAILED");
}

extern "C" DL_EXPORT std::string get_name() {
  return std::string("VLC input");
}

extern "C" DL_EXPORT std::string get_serial_guid() {
  return std::string("75392");
}

extern "C" DL_EXPORT shared_ptr<oculator::lib_options> get_options() {
  shared_ptr<oculator::lib_options> options(new oculator::lib_options());
  oculator::construction_option optionFilePath{STRING, "", "Local location of file", "Specify the file location of the video source."};

  options->push_back(optionFilePath);
  return options;
}

extern "C" DL_EXPORT module *get_module(const lib_options *options) {
  if(options->size() != 1)
    return nullptr;

  std::string file_path;

  for(auto option : *options) {
    switch(option.type) {
      case OPTION_TYPE::BOOL:
        break;
      case OPTION_TYPE::INT:
        break;
      case OPTION_TYPE::STRING:
        file_path = option.value.string_value;
        break;
    }     
  }

  oculator::source_handler *vlc_out = new oculator::source_handler(new VLCIO(file_path));
  return (module *)vlc_out;
}