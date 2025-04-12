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
 * Vin's main entry point. Really this just triggers the model loading and the
 * video feed.
 *
 * @author: ndepalma@alum.mit.edu
 * @license: MIT License
 */

#define GLOG_CUSTOM_PREFIX_SUPPORT 1
#include <functional_dag/libutils.h>
#include <glog/logging.h>
#include <stdlib.h>
#include <time.h>

#include <QCommandLineParser>
#include <QtCore/QTimer>
#include <QtWidgets/QApplication>
#include <filesystem>
#include <fstream>
#include <functional_dag/filter_sys.hpp>
#include <future>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

#ifndef CLI_ONLY
#include "vin/vin_dag_manager.hpp"
#include "vin/vin_gui.hpp"
#endif
#include "vin/error_codes.h"

#define XSTR(x) STR(x)
#define STR(x) #x

std::atomic_bool SHOULD_QUIT;

using namespace std::placeholders;
using namespace std::chrono_literals;
namespace fs = std::filesystem;

void print_banner() { std::cout << "\n\t\t| V I N |\n\n"; }

int initialize_library(fn_dag::library &dag_library) {
  const char *lib_list[] = {XSTR(VIN_LIB_DIR), "./lib", "./"};
  std::vector<fs::directory_entry> available_directories;

  // Just list out for the user what is available.
  for (const char *lib_dir : lib_list) {
    auto lib_fsdir = fs::directory_entry(lib_dir);
    std::cout << "Searching for modules in directory " << lib_fsdir << " -> ";
    auto available_libs = fn_dag::get_all_available_libs(lib_fsdir);
    if (available_libs.has_value() && available_libs->size() > 0) {
      std::cout << "found " << available_libs->size() << " candidates.";
      available_directories.push_back(lib_fsdir);
    } else {
      std::cout << "none found or nonexistant.";
    }
    std::cout << std::endl;
  }

  if (available_directories.size() == 0) return vin::error_codes::LIBRARY_EMPTY;

  dag_library.load_all_available_libs(available_directories);

  return vin::error_codes::NO_ERROR;
}

void glog_formatter_debug(std::ostream &s, const google::LogMessageInfo &m,
                          void *) {
  s << m.severity[0] << ' ' << std::setw(5) << m.thread_id << std::setfill('0')
    << ' ' << m.filename << ':' << m.line_number << "]";
}

void glog_formatter_release(std::ostream &s, const google::LogMessageInfo &m,
                            void *) {
  s << m.severity[0] << ' ' << std::setw(2) << m.time.hour() << ':'
    << std::setw(2) << m.time.min() << ':' << std::setw(2) << m.time.sec()
    << "." << std::setw(6) << m.time.usec() << "]";
}

int main(int argc, char *argv[]) {
  // Construct the GUI
  google::InitGoogleLogging(argv[0], &glog_formatter_release);
  FLAGS_logtostdout = 1;
  FLAGS_colorlogtostdout = 1;

#ifndef CLI_ONLY
  QApplication app(argc, argv);
#else
  QCoreApplication app(argc, argv);
#endif

  QCoreApplication::setApplicationName("VIN");
  QCoreApplication::setApplicationVersion(XSTR(VIN_VERSION));

  QCommandLineParser parser;
  parser.setApplicationDescription(
      "Video INput: a functional DAG for visualizing DLTensors");
  parser.addHelpOption();
  parser.addVersionOption();

  // Load a file
  QCommandLineOption loadDagOption(
      QStringList() << "c" << "dag_config",
      QCoreApplication::translate("main",
                                  "Specification of the DAG as JSON <file>."),
      QCoreApplication::translate("main", "file"));
  parser.addOption(loadDagOption);

  // Process the actual command line arguments given by the user
  parser.process(app);

  srand(time(NULL));

  print_banner();

  // Check to see if the user specified a config file argument
  const QStringList args = parser.positionalArguments();
  bool has_file_specd = parser.isSet(loadDagOption);

// Construct the library
#ifdef CLI_ONLY
  fn_dag::library dag_library;
#else
  vin::vin_dag_manager dag_library;
#endif
  std::cout << "Constructing module library...\n";
  initialize_library(dag_library);

  // Create a pointer to the eventual dag manager
  fn_dag::dag_manager<std::string> *fn_manager = nullptr;

  if (has_file_specd) {
    QString targetFile = parser.value(loadDagOption);
    std::string json_file = targetFile.toStdString();

    // Load the file to a string
    LOG(INFO) << "Loading config from file: " << json_file;
    std::ifstream file_stream(json_file);
    if (!file_stream.is_open()) {
      LOG(ERROR) << "Error: Unable to open file " << json_file;
      return vin::error_codes::FILE_NOT_FOUND;
    }
    std::stringstream buffer;
    buffer << file_stream.rdbuf();
    std::string file_contents = buffer.str();
    file_stream.close();

    // This runs on a thread to avoid blocking the GUI
    auto run_thread = std::async([&dag_library, &fn_manager, file_contents]() {
      // Waits for the GUI to come up first.
      std::this_thread::sleep_for(1000ms);
      // Parse the JSON and construct the dags
      if (auto reified_dag = dag_library.fsys_deserialize(file_contents, true);
          reified_dag.has_value()) {
        fn_manager = *reified_dag;
        // Print out what got loaded and started
        fn_manager->print_all_dags();
        LOG(INFO) << "Setup complete\n";
      } else {
        LOG(ERROR) << "Error: Unable to deserialize the compute dag due to: "
                   << reified_dag.error() << std::endl;
        return vin::error_codes::INVALID_JSON_FORMAT;
      }
      do {
        std::this_thread::sleep_for(300ms);
      } while (!SHOULD_QUIT);
      return vin::error_codes::NO_ERROR;
    });

    auto status = run_thread.wait_for(6s);
    if (status != std::future_status::ready) {
      app.exec();
      run_thread.wait();
    }
  }
#ifndef CLI_ONLY
  else {
    fn_manager = new fn_dag::dag_manager<std::string>();
    vin::main_window main_window(&dag_library, fn_manager);
    main_window.show();

    app.exec();
  }
#else
  else {
    LOG(ERROR) << "No spec to run!\n";
  }
#endif
  if (fn_manager != nullptr) {
    fn_manager->stahp();
    delete fn_manager;
    fn_manager = nullptr;
  }
  return 0;
}
