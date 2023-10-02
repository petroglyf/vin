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

#include "vin/utils/vin_library.hpp"
#include "vin/utils/lib_specification.hpp"
#include "vin/vin_dag_manager.hpp"

// void populate_lib_list(QListWidget *list, vin::vin_library *library);
void populate_lib_list();

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

