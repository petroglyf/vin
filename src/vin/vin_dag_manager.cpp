#include <functional_dag/libutils.h>

#include <QList>
#include <QTreeWidget>
#include <fstream>
#include <sstream>
#include <vin/error_codes.hpp>
#include <vin/vin_dag_manager.hpp>

using namespace fn_dag;

namespace vin {
vin_dag::vin_dag(fn_dag::dag_manager<std::string> *const _fn_manager)
    : m_dag_tree(nullptr), m_fn_manager(_fn_manager), m_all_loaded_specs() {}

void vin_dag::initialize_view(QTreeWidget *_view) {
  m_dag_tree = _view;

  QList<QTreeWidgetItem *> items;
  m_rootitem = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr),
                                   QStringList(QString("World")));
  items.append(m_rootitem);

  if (m_fn_manager != nullptr) m_fn_manager->stahp();

  m_dag_tree->addTopLevelItems(items);
}

vin_dag::~vin_dag() {
  if (m_dag_tree != nullptr) {
    delete m_dag_tree;
    m_dag_tree = nullptr;
  }

  if (m_fn_manager != nullptr) m_fn_manager->stahp();

  for (auto spec : m_all_loaded_specs)
    for (auto option : spec.instantiation_options)
      if (option.type == OPTION_TYPE::STRING)
        free((void *)option.value.string_value);
}

int vin_dag::vin_add_node(const std::string &name, const uint32_t guid,
                          module_transmit *dag_node, lib_options *options,
                          const std::string &parent_name) {
  std::function str_matches = [](const std::string val_find,
                                 const fn_dag::library_spec &val) {
    return val.name == val_find;
  };

  auto name_matches = std::bind(str_matches, name, std::placeholders::_1);
  auto parent_matches =
      std::bind(str_matches, parent_name, std::placeholders::_1);

  if (!std::any_of(m_all_loaded_specs.begin(), m_all_loaded_specs.end(),
                   parent_matches))
    return ERR_PARENT_ABSENT;

  if (std::any_of(m_all_loaded_specs.begin(), m_all_loaded_specs.end(),
                  name_matches))
    return ERR_NAME_ALREADY_EXISTS;

  library_spec packed_stats;
  packed_stats.name = name;
  packed_stats.lib_guid = guid;
  packed_stats.parent_name = parent_name;
  packed_stats.is_source = false;
  packed_stats.instantiation_options = *options;

  m_fn_manager->add_node(name, dag_node, parent_name);
  m_all_loaded_specs.push_back(packed_stats);

  // m_fn_manager.printAllTrees();

  QList<QTreeWidgetItem *> parent_options =
      m_dag_tree->findItems(parent_name.c_str(), Qt::MatchFlag::MatchExactly);
  if (parent_options.size() != 1) {
    std::cout << " Parent not found " << parent_name << " and size "
              << parent_options.size() << std::endl;
    return ERR_PARENT_ABSENT;
  }

  QTreeWidgetItem *parent_node = parent_options[0];

  QTreeWidgetItem *a_child = new QTreeWidgetItem();
  a_child->setText(0, name.c_str());
  parent_node->addChild(a_child);

  return ERR_NO_ERROR;
}

int vin_dag::vin_add_src(const std::string &name, const uint32_t guid,
                         lib_options *options, module_source *dag_node) {
  std::function str_matches = [](const std::string val_find,
                                 const library_spec &val) {
    return val.name == val_find;
  };

  auto name_matches = std::bind(str_matches, name, std::placeholders::_1);

  if (std::any_of(m_all_loaded_specs.begin(), m_all_loaded_specs.end(),
                  name_matches))
    return ERR_NAME_ALREADY_EXISTS;

  library_spec packed_stats;
  packed_stats.name = name;
  packed_stats.lib_guid = guid;
  packed_stats.is_source = true;
  packed_stats.instantiation_options = *options;

  QTreeWidgetItem *a_child = new QTreeWidgetItem(
      static_cast<QTreeWidget *>(nullptr), QStringList(QString(name.c_str())));

  m_fn_manager->add_dag(name, dag_node, true);
  m_all_loaded_specs.push_back(packed_stats);

  m_rootitem->addChild(a_child);

  return ERR_NO_ERROR;
}

void vin_dag::serialize(const fs::path file_name) {
  std::string raw_json = fsys_serialize(&m_all_loaded_specs);
  std::ofstream ofstream(file_name.c_str());
  if (ofstream.is_open()) ofstream.write(raw_json.c_str(), raw_json.length());
  ofstream.close();
}

void vin_dag::shutdown() {
  if (m_fn_manager != nullptr) m_fn_manager->stahp();
}
}  // namespace vin
