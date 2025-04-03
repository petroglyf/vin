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

#include <functional_dag/libutils.h>

#include <QListWidget>

#include "ui_main_dialog.h"

namespace vin {

class main_window : public QMainWindow {
 public:
  Ui::main_win main_ui_window;
  QListWidget *list;

  main_window(fn_dag::library *_library);
  ~main_window();

 private:
  const fn_dag::node_prop_spec *m_curr_lib_spec;
  std::string m_node_name;
  std::string m_parent_node_name;

  void refresh_options_panel(QListWidgetItem *value);
  void handle_create();
  void populate_options_panel(QListWidgetItem *value, QScrollArea *scroll_area);
  void save();
  void load();
};
}  // namespace vin
