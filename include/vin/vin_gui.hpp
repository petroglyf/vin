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
#include "vin/vin_dag_manager.hpp"

namespace vin {

typedef struct {
  std::string str_value;
  uint32_t int_value;
  bool bool_value;
} __value_union;

typedef struct {
  fn_dag::OPTION_TYPE type;
  std::string name;
  __value_union value;
} construction_options;

class main_window : public QMainWindow {
 public:
  Ui::main_win main_ui_window;
  QListWidget *list;

  main_window(vin_dag_manager *_library,
              fn_dag::dag_manager<std::string> *fn_manager);
  ~main_window();

 private:
  flatbuffers::FlatBufferBuilder m_builder;
  const fn_dag::node_prop_spec *m_curr_lib_spec;
  fn_dag::dag_manager<std::string> *m_fn_manager;
  vin_dag_manager *m_library;
  std::string m_node_name;
  std::string m_parent_node_name;
  std::vector<std::shared_ptr<construction_options>> m_construction_options;
  QTreeWidget *m_dag_tree;
  QTreeWidgetItem *m_rootitem;
  std::vector<::flatbuffers::Offset<fn_dag::node_spec>> m_nodes;
  std::vector<::flatbuffers::Offset<fn_dag::node_spec>> m_sources;

  void refresh_options_panel(QListWidgetItem *value);
  void handle_create();
  void populate_options_panel(QListWidgetItem *value, QScrollArea *scroll_area);
  void save();
  void load();
};
}  // namespace vin
