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
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <sstream>

#include <QCommandLineParser>
#include <QtWidgets/QApplication>
#include <QtCore/QTimer>

#ifndef CLI_ONLY
#include "vin/vin_gui.hpp"
#include "vin/vin_dag_manager.hpp"
#endif 
#include "vin/utils/vin_library.hpp"

#define XSTR(x) STR(x)
#define STR(x) #x

std::atomic_bool SHOULD_QUIT;

using namespace std::placeholders;
using namespace std::chrono_literals;
using namespace vin;

void print_banner() {
  std::cout << "\n\t\t| V I N |\n\n";
}

int load_from_file(const fs::path file_name, const std::unordered_map<uint32_t, fn_dag::instantiate_fn> &library, fn_dag::dag_manager<std::string>* &fn_manager) 
{
  std::cout << "Opening file " << file_name << std::endl;
  std::ifstream ifstream(file_name);
  if(ifstream.is_open()) {
    std::stringstream buffer;
    buffer << ifstream.rdbuf();
    const std::string all_contents = buffer.str();
    if(fn_manager == nullptr) 
      delete fn_manager;
    for(auto entry : library) {
      std::cout << entry.first << std::endl;
    }
    fn_manager = fsys_deserialize(all_contents, library);
    if(fn_manager != nullptr)
      fn_manager->run_single_threaded(true);
  }
  ifstream.close();

  if(fn_manager == nullptr) {
    std::cerr << "Unable to deserialize the compute dag..\n";
    return 0;
  } else {
    fn_manager->print_all_dags();
    std::cout << "Setup complete\n\n";
  }
  return 1;
}

int main(int argc, char *argv[])
{
  // Construct the GUI 
#ifndef CLI_ONLY
  QApplication app(argc, argv);
#else
  QCoreApplication app(argc, argv);
#endif

  QCoreApplication::setApplicationName("VIN");
  QCoreApplication::setApplicationVersion(XSTR(VIN_VERSION));

  QCommandLineParser parser;
  parser.setApplicationDescription("Video INput: a functional DAG for visualizing DLTensors");
  parser.addHelpOption();
  parser.addVersionOption();

  // Load a file
  QCommandLineOption loadDagOption(QStringList() << "c" << "dag_config", 
          QCoreApplication::translate("main", "Specification of the DAG as JSON <file>."),
          QCoreApplication::translate("main", "file"));
  parser.addOption(loadDagOption);

  // Process the actual command line arguments given by the user
  parser.process(app);

  srand (time(NULL));

  print_banner();

  // Check to see if the user specified a config file argument
  const QStringList args = parser.positionalArguments();
  bool has_file_specd = parser.isSet(loadDagOption);
  
  // Construct the library
  vin_library library;
  std::cout << "Constructing module library...\n";
  library.initialize();

  fn_dag::dag_manager<std::string> *fn_manager = new fn_dag::dag_manager<std::string>();
  fn_manager->run_single_threaded(true);

  if(has_file_specd) {
    QString targetFile = parser.value(loadDagOption);

    auto run_thread = std::async([&]() {
      this_thread::sleep_for(1000ms); 
      if(load_from_file(targetFile.toStdString(), library.get_library(), fn_manager)) {
        do {
          this_thread::sleep_for(300ms);
        } while(!SHOULD_QUIT);
        return 0;
      }
      return 1;
    });
    
    auto status = run_thread.wait_for(6s);
    if(status != std::future_status::ready) {
      app.exec();
      run_thread.wait();
    }
    
    
    if(fn_manager != nullptr) {
      fn_manager->stahp();
      delete fn_manager;
      fn_manager = nullptr;
    }
  } 
#ifndef CLI_ONLY
  else {
    main_window main_window(&library, fn_manager);
    main_window.show();

    app.exec();
  }
#else 
  else {
    std::cerr << "No spec to run!\n";
  }
#endif
   fn_manager->stahp();
  return 0;
}
