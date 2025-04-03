#include "vin/vin_gui.hpp"

#include <QCheckBox>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QSpinBox>
#include <QWidget>

#include "functional_dag/lib_spec_generated.h"

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

void main_window::populate_options_panel(QListWidgetItem *value,
                                         QScrollArea *scroll_area) {
  m_curr_lib_spec = value->data(Qt::UserRole).value<const node_prop_spec *>();
  // m_curr_spec_handle = m_curr_lib_spec->option_specs;
  if (m_curr_lib_spec == nullptr) {
    std::cerr << "No library spec found when populating the options panel!"
              << std::endl;
    return;
  }
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
        lineEdit->setToolTip(option.short_description.c_str());
        hlayout->addWidget(lineEdit);
      } break;
      case OPTION_TYPE_INT: {
        QSpinBox *integer_box = new QSpinBox();
        integer_box->setValue(0);
        integer_box->setToolTip(option.short_description.c_str());
        hlayout->addWidget(integer_box);
      } break;
      case OPTION_TYPE_BOOL: {
        QCheckBox *checkbox = new QCheckBox();
        checkbox->setChecked(false);
        checkbox->setToolTip(option.short_description.c_str());
        hlayout->addWidget(checkbox);
      } break;
      default: {
        std::cerr << "Unknown option type: " << option.type << std::endl;
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

main_window::main_window(fn_dag::library *_library) : m_curr_lib_spec(nullptr) {
  main_ui_window.setupUi(this);
  list = main_ui_window.available_libs;

  QObject::connect(list, &QListWidget::itemClicked, this,
                   &main_window::refresh_options_panel);

  QObject::connect(main_ui_window.create_button, &QPushButton::released, this,
                   &main_window::handle_create);

  QObject::connect(main_ui_window.actionSave, &QAction::triggered, this,
                   &main_window::save);

  QObject::connect(main_ui_window.actionOpen, &QAction::triggered, this,
                   &main_window::load);

  // m_dag.initialize_view(main_ui_window.current_dag);

  std::this_thread::sleep_for(100ms);  // Wait for the window to come up

  for (const library_spec &lib_spec : _library->get_spec_iter()) {
    populate_lib_list(list, &lib_spec);
  }
}

main_window::~main_window() {}

void main_window::refresh_options_panel(QListWidgetItem *value) {
  main_ui_window.statusbar->showMessage(value->statusTip(), 3000);
  QScrollArea *scroll_area = main_ui_window.options_pane;

  populate_options_panel(value, scroll_area);
}

void main_window::handle_create() {
  std::cout << "Create pressed\n";
  // if (m_curr_spec_handle != nullptr) {
  // for (auto option : *m_curr_spec_handle->options()) {
  //   std::cout << option.option_prompt << ": ";
  //   switch (option.type) {
  //     case OPTION_TYPE::STRING:
  //       std::cout << option.value.string_value << std::endl;
  //       break;
  //     case OPTION_TYPE::INT:
  //       std::cout << option.value.int_value << std::endl;
  //       break;
  //     case OPTION_TYPE::BOOL:
  //       std::cout << option.value.bool_value << std::endl;
  //       break;
  //   }
  // }

  // shared_ptr<module> new_module =
  // m_curr_lib_spec->instantiate(*m_curr_spec_handle);
  // if(new_module->get_type() == MODULE_TYPE::SOURCE) {
  //   module_source *src = new_module->get_handle_as_source();
  //   m_dag.vin_add_src(m_node_name, m_curr_lib_spec->serial_id_guid,
  //   m_curr_spec_handle, src);
  // } else {
  //   module_transmit *filter = new_module->get_slot_handle_as_mapping("no");
  //   m_dag.vin_add_node(m_node_name, m_curr_lib_spec->serial_id_guid,
  //   filter, m_curr_spec_handle, m_parent_node_name);
  // }
  // }
}

void main_window::save() {
  QString filename = QFileDialog::getSaveFileName(
      this, tr("Save file to.."), ".", tr("JSON Files (*.json)"));
  if (filename.length() > 0) {
    std::cout << "Saving to : " << filename.toStdString() << std::endl;
    // TODO: Actually save
    //  m_dag.serialize("test.json");
  }
}

void main_window::load() {
  QString filename = QFileDialog::getOpenFileName(
      this, tr("Save file to.."), ".", tr("JSON Files (*.json)"));
  std::cout << "Opening file : " << filename.toStdString() << std::endl;

  // TODO actually load
}

}  // namespace vin
