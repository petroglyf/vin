/**
 *   ____  ____  _     _     ____  _____  ____  ____ 
 *  /  _ \/   _\/ \ /\/ \   /  _ \/__ __\/  _ \/  __\
 *  | / \||  /  | | ||| |   | / \|  / \  | / \||  \/|
 *  | \_/||  \_ | \_/|| |_/\| |-||  | |  | \_/||    /
 *  \____/\____/\____/\____/\_/ \|  \_/  \____/\_/\_\
 * 
 * Uses GStreamer to read streaming media like webcams, online video, and movie files.
 * Streams to a torch tensor.
 * 
 * @author: ndepalma@alum.mit.edu
 * @license: MIT License
 */ 
#include "oculator/viz/DeviceReader.hpp"

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <functional>
#include<iostream>
#include <chrono>
#include <future>
#include <thread>
#include <torch/torch.h>

#include "functional_dag/lib_utils.h"

using namespace std::chrono_literals;
using namespace oculator;
using namespace std::placeholders;

GstPadProbeReturn oculator::cb_have_data (GstPad *pad,
                                GstPadProbeInfo *info,
                                DeviceReader    *user_data) {
  GstEvent *event = GST_PAD_PROBE_INFO_EVENT(info);
  DeviceReader *reader = reinterpret_cast<DeviceReader *>(user_data);
  if (GST_EVENT_CAPS == GST_EVENT_TYPE(event)) {
    GstCaps * caps = gst_caps_new_any();
    gst_event_parse_caps(event, &caps);
    int32_t width, height;

    GstStructure *s = gst_caps_get_structure(caps, 0);

    gboolean res;
    res = gst_structure_get_int (s, "width", &width);
    res |= gst_structure_get_int (s, "height", &height);
    const gchar *format_string = gst_structure_get_string(s, "format");

    if (!res)
      g_error( "no dimensions");

    reader->mImageWidth = width;
    reader->mImageHeight = height;
    g_print(" Width: %d, Height: %d\n", width, height);
    g_print(" Format: %s\n", format_string);
    // g_print("Structure: %s\n", gst_structure_to_string(s));
    gst_caps_unref(caps);
  } else { // if(GST_EVENT_TYPE(event) == 1208042016)
    gint x, y;
    GstMapInfo map;
    guint16 *ptr, t;
    GstBuffer *buffer;

    buffer = GST_PAD_PROBE_INFO_BUFFER (info);
    if (buffer == NULL )
        return GST_PAD_PROBE_OK;
    
    if(GST_IS_BUFFER(buffer)) {

      int total_size = gst_buffer_get_size(buffer);
      unsigned char *array = (unsigned char *)malloc(total_size);
      gst_buffer_extract(buffer, 0, array, total_size);
      
      // Create the torch tensor to return and fill it from OpenIL. 
      const uint32_t width = reader->mImageWidth;
      const uint32_t height = reader->mImageHeight;
      const uint32_t stride = total_size / height;
      torch::Tensor imageData = torch::zeros({height,width, 3}, torch::TensorOptions().dtype(torch::kUInt8));
      for(int h = 0;h < height;h++)
        std::memcpy(imageData[h].data_ptr(), array+(h*stride), width*3);
      free(array);
      reader->pump(imageData); 
    }
  } 

  return GST_PAD_PROBE_OK;
}

/* This function is called when an error message is posted on the bus */
void oculator::error_cb (GstBus *bus, GstMessage *msg, DeviceReader *data) {
  GError *err;
  gchar *debug_info;

  /* Print error details on the screen */
  gst_message_parse_error (msg, &err, &debug_info);
  g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
  g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
  g_clear_error (&err);
  g_free (debug_info);

  /* Set the pipeline to READY (which stops playback) */
  gst_element_set_state (data->mGstPlay, GST_STATE_READY);
}

/* This function is called when an End-Of-Stream message is posted on the bus.
 * We just set the pipeline to READY (which stops playback) */
void oculator::eos_cb (GstBus *bus, GstMessage *msg, DeviceReader *data) {
  g_print ("End-Of-Stream reached.\n");
  gst_element_set_state (data->mGstPlay, GST_STATE_READY);
  data->should_quit = true;
}

static void on_pad_added (GstElement *element,
                          GstPad     *pad,
                          gpointer    data)
{
  GstPad *sinkpad;
  GstElement *vid_sink = (GstElement *) data;
  /* We can now link this pad with the vorbis-decoder sink pad */
  sinkpad = gst_element_get_static_pad (vid_sink, "sink");

  gst_pad_link (pad, sinkpad);

  gst_object_unref (sinkpad);
}

void oculator::state_changed_cb (GstBus *bus, GstMessage *msg, DeviceReader *data) {
  GstState old_state, new_state, pending_state;
  gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
  if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data->mGstPlay)) {
    data->mState = new_state;
    // g_print ("State was %s\n", gst_element_state_get_name (old_state));
    g_print ("State set to %s\n", gst_element_state_get_name (new_state));
  }
}

DeviceReader::DeviceReader(const oculator::SourceType source_type,
                           const std::string uri, 
                           const std::function< void(torch::Tensor *) > callback) : 
                                      mCallback{callback} {
  mIsInitialized = false;

  // if(uri.rfind("file", 0) == 0)
  mIsInitialized = initializeGST(uri, source_type);

  // mIsInitialized = initializeGST(uri, FROM_DEVICE);
  // else if(uri.rfind("rtsp", 0) == 0)
  //   mIsInitialized = initializeGST(uri, FROM_RTSP);
}

void DeviceReader::shutdownGST() {
  if(this->mGstPlay != NULL && mIsInitialized) {
    gst_element_set_state (mGstPlay, GST_STATE_READY);

    
    mIsInitialized = false;
  }
}

DeviceReader::~DeviceReader() {
    shutdownGST();
}

bool DeviceReader::initializeGST(const std::string uri, const SourceType type) {
  GstStateChangeReturn ret;

  /* Initialize GStreamer */
  gst_init (NULL, NULL);

  mGSTLoop = g_main_loop_new (NULL, FALSE);

  // filesrc location=CF1.png ! decodebin ! autovideosink
  mPipeline = gst_pipeline_new ("oculator-gst-pipeline");

  /* Create the elements */
  should_quit = false;
  if(type == FROM_VIDEO_FILE) {
    std::cout << "uri decode\n";
    // mGstPlay = gst_element_factory_make ("uridecodebin", "uridecodebin");
    // GstElementFactory *fac = gst_element_factory_find ("uridecodebin");
    // std::cout << "Fac: " << fac << std::endl;
    mGstPlay = gst_element_factory_make ("uridecodebin", NULL);
    // mGstPlay = gst_element_factory_make ("dataurisrc","dataurisrc");
  } else if(type == FROM_DEVICE)
    mGstPlay = gst_element_factory_make ("autovideosrc", "autovideosrc");
  mGstConv = gst_element_factory_make ("videoconvert", "videoconvert");
  // mGstSink = gst_element_factory_make ("autovideosink", "autovideosink"); // for debugging
  mGstSink = gst_element_factory_make ("fakesink", "fakesink"); // for release
  GstCaps * caps = gst_caps_new_simple ("video/x-raw",
                                        "format", G_TYPE_STRING, "RGB",
                                        NULL);
  std::cout << "Play: " << mGstPlay << std::endl;
  std::cout << "Sink: " << mGstSink << std::endl;
  if (!mGstPlay || !mGstSink) {
    g_printerr ("Not all elements could be created. Perhaps you need to install gst-plugins-good?\n");
    shutdownGST();
    return false;
  }
  
  gst_bin_add_many (GST_BIN (mPipeline), mGstPlay, mGstConv, mGstSink, NULL);

  gst_element_link(mGstPlay, mGstConv);
  gst_element_link_filtered(mGstConv, mGstSink, caps);
  /* Set the URI to play */
  if(type == FROM_VIDEO_FILE)
    g_object_set (mGstPlay, "uri", uri.c_str(), NULL);
  else if(type == FROM_DEVICE)
    g_object_set (mGstPlay, "src", uri.c_str(), NULL);
  else if(type == FROM_RTSP)
    g_object_set (mGstPlay, "location", uri.c_str(), NULL);
  /* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
  GstBus *bus = gst_element_get_bus (mPipeline);
  gst_bus_add_signal_watch (bus);
  g_signal_connect (G_OBJECT (bus), "message::error", (GCallback)error_cb, this);
  g_signal_connect (G_OBJECT (bus), "message::eos", (GCallback)eos_cb, this);
  g_signal_connect (G_OBJECT (bus), "message::state-changed", (GCallback)state_changed_cb, this);
  gst_object_unref (bus);

  /* Finally connect to the pad added event which will let us probe the images and convert to torch tensors */
  g_signal_connect (mGstPlay, "pad-added", G_CALLBACK (on_pad_added), mGstConv);
  GstPad  *pad = gst_element_get_static_pad(mGstConv, "src");
  
  if(pad != NULL)
    gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_DATA_DOWNSTREAM, (GstPadProbeCallback) cb_have_data, this, NULL);
  else
    g_printerr("PAD NOT RETREIVED\n");
  
  /* Start playing */
  ret = gst_element_set_state (mPipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    shutdownGST();
    return false;
  }

  return true;
}


class GSTIO : public oculator::module_source {
  shared_ptr<DeviceReader> reader;
  std::promise<torch::Tensor *> tensor_promise;

public:
  GSTIO(const int type, const std::string uri, const std::string file_path);
  ~GSTIO() {}

  void update_tensor(torch::Tensor *new_tensor);
  torch::Tensor *update();
};

void GSTIO::update_tensor(torch::Tensor *new_tensor) {
  std::cout << "Got a callback from GST, setting value\n";
  tensor_promise.set_value(new_tensor);
  std::cout << "Going to get another frame\n";
}

GSTIO::GSTIO(const int type, const std::string uri, const std::string file_path) {
  oculator::SourceType stype = type == 0 ? FROM_VIDEO_FILE : FROM_RTSP;
  std::string path = type == 0 ? file_path : uri;
  DeviceReader *hook = new DeviceReader(stype, path, 
                                  [this](torch::Tensor *t) { 
                                    std::cout << "got a tensor from gst\n";
                                    this->update_tensor(t);
                                  });

  reader = std::shared_ptr<DeviceReader>();
}

torch::Tensor *GSTIO::update() {
  std::cout << "Moving tensor and getting future\n";
  tensor_promise = std::move(tensor_promise);
  std::future<torch::Tensor *> tensor_future = tensor_promise.get_future();
  std::cout << "Waiting\n";
  return tensor_future.get();
}

bool DeviceReader::isPlaying() {
  return mIsPlaying;
}

void DeviceReader::pump(torch::Tensor &tensor) {
  mCallback(&tensor);
}
#pragma GCC diagnostic ignored "-Wreturn-type-c-linkage"

#define DL_EXPORT __attribute__ ((visibility ("default")))

extern "C" DL_EXPORT std::string get_simple_description() {
  return std::string("This library will subscribe and publish to gstreamer streams.");
}

extern "C" DL_EXPORT std::string get_detailed_description() {
  return std::string("This library will subscribe and publish to gstreamer streams. DETAILED");
}

extern "C" DL_EXPORT std::string get_name() {
  return std::string("GStreamer I/O");
}

extern "C" DL_EXPORT std::string get_serial_guid() {
  return std::string("01568");
}

extern "C" DL_EXPORT shared_ptr<oculator::lib_options> get_options() {
  shared_ptr<oculator::lib_options> options(new oculator::lib_options());
  oculator::construction_option optionType{INT, 0, "Which type of source?", "0 for movie, 1 for URL"};
  oculator::construction_option optionURI{STRING, "", "Online location URL", "Specify the online location of the video source."};
  oculator::construction_option optionFilePath{STRING, "", "Local location of file", "Specify the file location of the video source."};

  options->push_back(optionType);
  options->push_back(optionURI);
  options->push_back(optionFilePath);
  return options;
}

extern "C" DL_EXPORT module *get_module(const lib_options *options) {
  if(options->size() != 3)
    return nullptr;
  int type = 0;
  std::string uri;
  std::string file_path;

  for(auto option : *options) {
    switch(option.type) {
      case OPTION_TYPE::BOOL:
        break;
      case OPTION_TYPE::INT:
        type = option.value.int_value;
        break;
      case OPTION_TYPE::STRING:
        if(option.option_prompt.starts_with("Online"))
          uri = option.value.string_value;
        else
          file_path = option.value.string_value;
        break;
    }     
  }

  oculator::source_handler *gst_out = new oculator::source_handler(new GSTIO(type, uri, file_path));
  return (module *)gst_out;
}