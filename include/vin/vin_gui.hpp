#pragma once
/**
 *           _________ _       
 *  |\     /|\__   __/( (    /|
 *  | )   ( |   ) (   |  \  ( |
 *  ( (   ) )   | |   | (\ \) |
 *   \ \_/ /    | |   | | \   |
 *    \   /  ___) (___| )  \  |
 *     \_/   \_______/|/    )_)
 *                             
 * Main window header
 * 
 * @author: ndepalma@alum.mit.edu
 * @license: MIT License
 */ 

#include "vin/viz/ui_putitup.h"
#include <QListWidget>
#include "functional_dag/lib_utils.h"
#include "vin/vin_dag_manager.hpp"

void populate_lib_list(QListWidget *list);

using module_getter_fn = fn_dag::module* (*)(const fn_dag::lib_options *);

class lib_specification {
public:
  lib_specification();
  lib_specification(void * const handle);
  ~lib_specification();
  
  long serial_id_guid;
  string lib_name;
  string simple_description;
  bool is_source_module;
  string detailed_description;
  shared_ptr<fn_dag::lib_options> available_options;
  module_getter_fn module_handle;
  
  shared_ptr<fn_dag::module> instantiate(const fn_dag::lib_options &options);

private:
  void * lib_handle;
  void load_lib();
};

class main_window : public QMainWindow {
public:
  Ui::PutItUp main_ui_window;
  QListWidget *list;

  main_window();
  ~main_window();
  

private:  
  shared_ptr<fn_dag::lib_options> m_curr_spec_handle;
  shared_ptr<lib_specification> m_curr_lib_spec;
  vin::vin_dag m_dag;
  std::string m_node_name;
  std::string m_parent_node_name;

  void refresh_options_panel(QListWidgetItem *value );
  void handle_create();
  void populate_options_panel(QListWidgetItem *value, QScrollArea *scroll_area);
  void save();
  void load();
};


shared_ptr< vector<fs::directory_entry> > get_all_available_libs(const fs::directory_entry &library_path);
bool preflight_lib(const fs::path _lib_path);

shared_ptr<lib_specification> fsys_load_lib(fs::path vin_lib);
