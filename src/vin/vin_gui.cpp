#include "vin/vin_gui.hpp"

#include <QtCore/qglobal.h>
#include <functional_dag/lib_spec_generated.h>
#include <functional_dag/libutils.h>
#include <glog/logging.h>

#include <QCheckBox>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QSpinBox>
#include <QWidget>
#include <fstream>

using namespace fn_dag;

void populate_lib_list(QListWidget *list,
                       const fn_dag::library_spec *library_spec) {
  for (auto &lib_spec : library_spec->available_nodes) {
    QListWidgetItem *new_item = new QListWidgetItem(lib_spec.name.c_str());
    new_item->setStatusTip(lib_spec.description.c_str());
    new_item->setData(Qt::UserRole, QVariant::fromValue(&lib_spec));
    list->addItem(new_item);
  }
}

namespace vin {
template <typename T>
const T *__offset_to_table(const flatbuffers::FlatBufferBuilder &_builder,
                           const ::flatbuffers::Offset<T> &_offset_table) {
  return reinterpret_cast<const T *>(_builder.GetCurrentBufferPointer() +
                                     _builder.GetSize() - _offset_table.o);
}
::flatbuffers::Offset<fn_dag::node_spec> __create_node_spec(
    flatbuffers::FlatBufferBuilder &_builder, const fn_dag::GUID_vals &guid,
    const std::string &_node_name, const std::string &_parent_node_name,
    const std::vector<std::shared_ptr<construction_options>>
        &_construction_options) {
  vector<::flatbuffers::Offset<string_mapping>> wires;
  vector<::flatbuffers::Offset<construction_option>> options;

  for (auto option : _construction_options) {
    LOG(INFO) << option->name << ": ";
    switch (option->type) {
      case OPTION_TYPE_STRING:
        LOG(INFO) << option->value.str_value << std::endl;
        options.push_back(Createconstruction_optionDirect(
            _builder, option->name.c_str(),
            Createoption_valueDirect(_builder, option->type, 0, false,
                                     option->value.str_value.c_str())));
        break;
      case OPTION_TYPE_INT:
        LOG(INFO) << option->value.int_value << std::endl;
        options.push_back(Createconstruction_optionDirect(
            _builder, option->name.c_str(),
            Createoption_valueDirect(_builder, option->type,
                                     option->value.int_value, false, nullptr)));
        break;
      case OPTION_TYPE_BOOL:
        LOG(INFO) << option->value.bool_value << std::endl;
        options.push_back(Createconstruction_optionDirect(
            _builder, option->name.c_str(),
            Createoption_valueDirect(_builder, option->type, 0,
                                     option->value.bool_value, nullptr)));
        break;
      default:
        LOG(ERROR) << "Unhandled option type: " << option->type << std::endl;
    }
  }
  wires.push_back(Createstring_mappingDirect(_builder, "input_tensor",
                                             _parent_node_name.c_str()));
  return fn_dag::Createnode_specDirect(_builder, &guid, _node_name.c_str(),
                                       fn_dag::PS_TYPE_THREAD, &wires,
                                       &options);
}

void main_window::populate_options_panel(QListWidgetItem *value,
                                         QScrollArea *scroll_area) {
  m_curr_lib_spec = value->data(Qt::UserRole).value<const node_prop_spec *>();
  if (m_curr_lib_spec == nullptr) {
    LOG(ERROR) << "No library spec found when populating the options panel!"
               << std::endl;
    return;
  }
  m_construction_options.clear();
  m_parent_node_name.clear();
  m_node_name.clear();

  QWidget *container_widget = new QWidget();
  QVBoxLayout *vlayout = new QVBoxLayout();

  /////////// Name for both //////////
  {
    QHBoxLayout *hlayout = new QHBoxLayout();
    QLabel *prompt = new QLabel("Name of node:");
    QLineEdit *name_edit = new QLineEdit();
    name_edit->setToolTip("User identifyable name of the node");
    hlayout->addWidget(prompt);
    hlayout->addWidget(name_edit);
    QObject::connect(name_edit, &QLineEdit::textChanged,
                     [this](const QString &newValue) {
                       this->m_node_name = newValue.toStdString();
                     });
    vlayout->addLayout(hlayout);
  }

  if (m_curr_lib_spec->module_type != fn_dag::NODE_TYPE::NODE_TYPE_SOURCE) {
    QHBoxLayout *hlayout = new QHBoxLayout();
    QLabel *prompt = new QLabel("Select parent:");
    QLineEdit *parent_edit = new QLineEdit();
    parent_edit->setToolTip("User identifyable name of the node");
    hlayout->addWidget(prompt);
    hlayout->addWidget(parent_edit);
    QObject::connect(parent_edit, &QLineEdit::textChanged,
                     [this](const QString &newValue) {
                       this->m_parent_node_name = newValue.toStdString();
                     });
    vlayout->addLayout(hlayout);
  }

  for (auto option : m_curr_lib_spec->construction_types) {
    QHBoxLayout *hlayout = new QHBoxLayout();
    QLabel *prompt = new QLabel(option.option_prompt.c_str());
    hlayout->addWidget(prompt);
    switch (option.type) {
      case OPTION_TYPE_STRING: {
        QLineEdit *lineEdit = new QLineEdit();
        shared_ptr<construction_options> str_opt =
            make_shared<construction_options>();
        str_opt->name = option.name;
        str_opt->type = option.type;
        str_opt->value.str_value = "";
        m_construction_options.push_back(str_opt);
        QObject::connect(lineEdit, &QLineEdit::textChanged,
                         [str_opt](const QString &newValue) {
                           str_opt->value.str_value = newValue.toStdString();
                         });

        lineEdit->setToolTip(option.short_description.c_str());
        hlayout->addWidget(lineEdit);
      } break;
      case OPTION_TYPE_INT: {
        QSpinBox *integer_box = new QSpinBox();
        shared_ptr<construction_options> int_opt =
            make_shared<construction_options>();
        int_opt->name = option.name;
        int_opt->type = option.type;
        int_opt->value.int_value = 0;
        m_construction_options.push_back(int_opt);
        QObject::connect(integer_box, &QSpinBox::valueChanged,
                         [int_opt](const int newValue) {
                           int_opt->value.int_value = newValue;
                         });
        integer_box->setValue(0);
        integer_box->setToolTip(option.short_description.c_str());
        hlayout->addWidget(integer_box);
      } break;
      case OPTION_TYPE_BOOL: {
        QCheckBox *checkbox = new QCheckBox();
        shared_ptr<construction_options> bool_opt =
            make_shared<construction_options>();
        bool_opt->name = option.name;
        bool_opt->type = option.type;
        bool_opt->value.bool_value = false;
        m_construction_options.push_back(bool_opt);
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
        QObject::connect(checkbox, &QCheckBox::stateChanged,
#else
        QObject::connect(checkbox, &QCheckBox::checkStateChanged,
#endif
                         [=]() {
                           bool_opt->value.bool_value =
                               checkbox->checkState() == Qt::Checked;
                         });
        checkbox->setChecked(false);
        checkbox->setToolTip(option.short_description.c_str());
        hlayout->addWidget(checkbox);
      } break;
      default: {
        LOG(ERROR) << "Unknown option type: " << option.type << std::endl;
        continue;
      }
    }
    vlayout->addLayout(hlayout);
  }

  container_widget->setLayout(vlayout);
  scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scroll_area->setWidget(container_widget);
  scroll_area->setEnabled(true);
  vlayout->addStretch();
}

main_window::main_window(vin_dag_manager *_library,
                         fn_dag::dag_manager<std::string> *fn_manager)
    : m_builder(512),
      m_curr_lib_spec(nullptr),
      m_fn_manager(fn_manager),
      m_library(_library) {
  main_ui_window.setupUi(this);
  list = main_ui_window.available_libs;

  // Connect actions
  QObject::connect(list, &QListWidget::itemClicked, this,
                   &main_window::refresh_options_panel);

  QObject::connect(main_ui_window.create_button, &QPushButton::released, this,
                   &main_window::handle_create);

  QObject::connect(main_ui_window.actionSave, &QAction::triggered, this,
                   &main_window::save);

  QObject::connect(main_ui_window.actionOpen, &QAction::triggered, this,
                   &main_window::load);

  /// LOAD libraries
  std::this_thread::sleep_for(100ms);  // Wait for the window to come up

  for (const library_spec &lib_spec : _library->get_spec_iter()) {
    populate_lib_list(list, &lib_spec);
  }

  // Load func tree
  m_dag_tree = main_ui_window.current_dag;

  QList<QTreeWidgetItem *> items;
  m_rootitem = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr),
                                   QStringList(QString("Running Nodes")));
  items.append(m_rootitem);

  m_dag_tree->addTopLevelItems(items);
}

main_window::~main_window() {}

void main_window::refresh_options_panel(QListWidgetItem *value) {
  main_ui_window.statusbar->showMessage(value->statusTip(), 3000);
  QScrollArea *scroll_area = main_ui_window.options_pane;

  populate_options_panel(value, scroll_area);
}

void main_window::handle_create() {
  LOG(INFO) << "Create pressed\n";

  auto node_spec_offset =
      __create_node_spec(m_builder, m_curr_lib_spec->guid.m_id, m_node_name,
                         m_parent_node_name, m_construction_options);
  auto node_spec = __offset_to_table(m_builder, node_spec_offset);
  m_library->add_node(*m_fn_manager, node_spec);

  switch (m_curr_lib_spec->module_type) {
    case fn_dag::NODE_TYPE::NODE_TYPE_SOURCE:
      m_sources.push_back(node_spec_offset);
      break;
    case fn_dag::NODE_TYPE::NODE_TYPE_FILTER:
      m_nodes.push_back(node_spec_offset);
      break;
    default:
      LOG(ERROR) << "Unknown node type: " << m_curr_lib_spec->module_type
                 << std::endl;
      break;
  }

  /// Display it in the GUI
  QTreeWidgetItem *a_child = new QTreeWidgetItem();
  a_child->setText(0, m_node_name.c_str());
  if (m_parent_node_name.empty()) {
    m_rootitem->addChild(a_child);
  } else {
    QList<QTreeWidgetItem *> parent_options = m_dag_tree->findItems(
        m_parent_node_name.c_str(), Qt::MatchFlag::MatchRecursive);
    if (parent_options.size() != 1) {
      LOG(INFO) << " Parent not found " << m_parent_node_name << std::endl;
      // return ERR_PARENT_ABSENT;
      return;
    }

    QTreeWidgetItem *parent_node = parent_options[0];
    parent_node->addChild(a_child);
  }
}

void main_window::save() {
  QString filename = QFileDialog::getSaveFileName(
      this, tr("Save file to.."), ".", tr("JSON Files (*.json)"));
  if (filename.length() > 0) {
    LOG(INFO) << "Saving to : " << filename.toStdString() << std::endl;

    auto pipe_spec_direct =
        fn_dag::Createpipe_specDirect(m_builder, &m_sources, &m_nodes);
    m_builder.Finish(pipe_spec_direct);
    // auto pipe_spec = Getpipe_spec(m_builder.GetBufferPointer());
    auto json = fn_dag::fsys_serialize(m_builder.GetBufferPointer());
    if (json.has_value()) {
      std::fstream my_file;
      my_file.open(filename.toStdString(), std::fstream::out);
      my_file << *json;
      my_file.close();
    } else {
      LOG(ERROR) << "Error: Unable to serialize the compute dag due to: "
                 << json.error() << std::endl;
    }
  }
}

void main_window::load() {
  QString filename = QFileDialog::getOpenFileName(
      this, tr("Save file to.."), ".", tr("JSON Files (*.json)"));
  LOG(INFO) << "Opening file : " << filename.toStdString() << std::endl;

  // TODO actually load
}

}  // namespace vin
