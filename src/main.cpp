/**
 *           _________ _       
 *  |\     /|\__   __/( (    /|
 *  | )   ( |   ) (   |  \  ( |
 *  ( (   ) )   | |   | (\ \) |
 *   \ \_/ /    | |   | | \   |
 *    \   /  ___) (___| )  \  |
 *     \_/   \_______/|/    )_)
 *                             
 * 
 * Vin's main entry point. Really this just triggers the model loading and the video feed. 
 * 
 * @author: ndepalma@alum.mit.edu
 * @license: MIT License
 */                                             



#include <iostream>
#include <cstdint>
#include <variant>
#include <chrono>
#include <thread>
#include <filesystem>
#include <vector>

#include <QCommandLineParser>
#include <QtWidgets/QApplication>
#include <QtCore/QTimer>
#include "functional_dag/lib_utils.h"
#include "vin/vin_gui.hpp"

#define XSTR(x) STR(x)
#define STR(x) #x

std::atomic_bool SHOULD_QUIT;
namespace fs = std::filesystem;

using namespace std::placeholders;
using namespace std::chrono_literals;

typedef enum {
  LOOP_FOREVER,
  LOOP_UNTIL_COMPLETE,
  SINGLE_FRAME
} PLAY_MODE;

// void updateViz(torch::Tensor new_image, Ui::MainWindow ui) {
//   ui.raw->setTensor(new_image, VIZ_RGB);
//   // First make it greyscale 
//   at::Tensor float_img = new_image.toType(at::ScalarType::Double);
//   float_img = torch::transpose(float_img, 0, 2);
//   float_img = torch::permute(float_img, {0, 2, 1});
//   at::Tensor saliency_input = at::upsample_bilinear2d(float_img.toType(at::ScalarType::Float).unsqueeze(0), {768, 1024}, false);

//   torch::Tensor saliency_map = perform_deep_gaze_inference(module, saliency_input, centerbias);
//   at::Tensor saliency_output = at::upsample_bilinear2d(saliency_map.toType(at::ScalarType::Double).unsqueeze(0).unsqueeze(0), {new_image.size(1), new_image.size(0)}, false);
//   saliency_output = saliency_output.toType(at::ScalarType::Byte).squeeze().t();
//   ui.saliency->setTensor(saliency_output, VIZ_HEATMAP);
//  }

shared_ptr<fn_dag::module> __instantiate_fn_prototype(std::shared_ptr<lib_specification> lib_handle, 
                                           const fn_dag::lib_options * const opts) {
  shared_ptr<fn_dag::module> module_ptr = lib_handle->instantiate(*opts);
  return module_ptr;
}


int main(int argc, char *argv[])
{
  // Construct the GUI 
  QApplication app(argc, argv);

  QCoreApplication::setApplicationName("VIN");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("Video INput: a functional DAG wit torch support");
  parser.addHelpOption();
  parser.addVersionOption();

  // Load a file
  QCommandLineOption loadDagOption(QStringList() << "c" << "dag_config", 
          QCoreApplication::translate("main", "Specification of the DAG as JSON <file>."),
          QCoreApplication::translate("main", "file"));
  parser.addOption(loadDagOption);

  // Process the actual command line arguments given by the user
  parser.process(app);

  const QStringList args = parser.positionalArguments();
  bool has_file_specd = parser.isSet(loadDagOption);
  if(has_file_specd) {
    
    QString targetFile = parser.value(loadDagOption);
    std::cout << "Trying to open file " << targetFile.toStdString() << std::endl;
    std::cout << "Load file " << targetFile.toStdString() << std::endl;

    #ifdef VIN_LIB_DIR
      auto all_libs = get_all_available_libs(fs::directory_entry(XSTR(VIN_LIB_DIR)));
    #else
      auto all_libs = get_all_available_libs(fs::directory_entry("./lib"));
    #endif
    std::vector<std::shared_ptr<lib_specification> > lib_specs;
    std::unordered_map<uint32_t, fn_dag::instantiate_fn> library;
    for(auto lib: *all_libs) {
      if(preflight_lib(lib)) {
        std::shared_ptr<lib_specification> lib_handle = fsys_load_lib(lib);
        lib_specs.push_back(lib_handle);
        fn_dag::instantiate_fn create_fn = std::bind(__instantiate_fn_prototype, lib_handle, std::placeholders::_1);
        std::cout << "Emplacing serial guid " << lib_handle->serial_id_guid << std::endl;
        library.emplace(lib_handle->serial_id_guid, create_fn);
      }
    }

    // QMetaObject::invokeMethod(qApp, [&]() {
    //   std::cout << "Waiting to load file\n";
    //   this_thread::sleep_for(1000ms);
    //   std::cout << "Loading stuff\n";
    //   manager.load_from_file(targetFile.toStdString(), library);
    //   std::cout << "Continue\n";
    //   do {
    //     this_thread::sleep_for(300ms);
    //   } while(!SHOULD_QUIT);
    // });
    std::thread run_thread([&]() {
      this_thread::sleep_for(1000ms); 
      vin::vin_dag manager; 
      manager.load_from_file(targetFile.toStdString(), library);

      do {
        this_thread::sleep_for(300ms);
      } while(!SHOULD_QUIT);
    });
    
    app.exec();
    // TODO
    run_thread.join();
    // manager.Stahp();
  } else {
    main_window main_window;
    std::thread list_thread(populate_lib_list, main_window.list);
    main_window.show();

    int ret_code = app.exec();

    list_thread.join();
    
    return ret_code;
  }
  return 0;
 

  // switch(how_long_to_run) {
  //   case LOOP_UNTIL_COMPLETE:
  //     break;
  //   case SINGLE_FRAME:
  //     {
  //       // Load the raw image to process
  //       torch::Tensor raw_image = image_utils::loadFile(filename.c_str());
  //       updateViz(raw_image, ui);
  //     }
  //     break;    
  //   default:
  //     std::cerr << "Error: case not handled yet.\n";
  // }

}
