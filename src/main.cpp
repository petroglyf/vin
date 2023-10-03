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
#include <vector>
#include <stdlib.h>
#include <time.h>

#include <QCommandLineParser>
#include <QtWidgets/QApplication>
#include <QtCore/QTimer>

#include "vin/vin_gui.hpp"
#include "vin/utils/vin_library.hpp"

std::atomic_bool SHOULD_QUIT;

using namespace std::placeholders;
using namespace std::chrono_literals;
using namespace vin;

void print_banner() {
  std::cout << "\n\t\t| V I N |\n\n";
}

int main(int argc, char *argv[])
{
  // Construct the GUI 
  QApplication app(argc, argv);

  QCoreApplication::setApplicationName("VIN");
  QCoreApplication::setApplicationVersion("0.8");

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

  if(has_file_specd) {
    QString targetFile = parser.value(loadDagOption);
    vin::vin_dag manager; 

    // QMetaObject::invokeMethod(qApp, [&]() {
    //   std::cout << "Waiting to load file\n";
    //   this_thread::sleep_for(1000ms);
    //   std::cout << "Loading stuff\n";
    //   manager.load_from_file(targetFile.toStdString(), library.get_library());
    //   std::cout << "Continue\n";
    //   do {
    //     this_thread::sleep_for(300ms);
    //   } while(!SHOULD_QUIT);
    // });

    std::thread run_thread([&]() {
      this_thread::sleep_for(1000ms); 
      manager.load_from_file(targetFile.toStdString(), library.get_library());

      do {
        this_thread::sleep_for(300ms);
      } while(!SHOULD_QUIT);
    });
    
    app.exec();
    run_thread.join();
    manager.shutdown();
  } else {
    main_window main_window;
    // std::thread list_thread(populate_lib_list, main_window.list, &library);
    main_window.show();

    int ret_code = app.exec();

    // list_thread.join();
    
    return ret_code;
  }
  return 0;
}
