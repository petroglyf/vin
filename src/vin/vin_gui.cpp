#include "vin/vin_gui.hpp"
#include <QObject>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include <QSpinBox>
#include <QCheckBox>

#define XSTR(x) STR(x)
#define STR(x) #x

using namespace fn_dag;

void populate_lib_list() { // QListWidget *list, vin::vin_library *library
  std::this_thread::sleep_for(100ms); // Wait for the window to come up
  
// #ifdef VIN_LIB_DIR
//   auto lib_list = get_all_available_libs(fs::directory_entry(XSTR(VIN_LIB_DIR)));
// #else
  // auto lib_list = get_all_available_libs(fs::directory_entry("./lib"));
// #endif
  // auto lib_list = get_all_available_libs(fs::directory_entry("./"));
  // for(auto entry : *lib_list) {
  //   if(preflight_lib(entry.path())) {
  //     auto lib_spec = fsys_load_lib(entry.path());

  //     QListWidgetItem *new_item = new QListWidgetItem(lib_spec->lib_name.c_str());
  //     new_item->setToolTip(lib_spec->detailed_description.c_str());
  //     new_item->setStatusTip(lib_spec->simple_description.c_str());
  //     new_item->setData(Qt::UserRole, QVariant::fromValue(lib_spec));
  //     list->addItem(new_item);
  //   } else {
  //     std::cout << "Lib not loadable\n";
  //   }
  // }
}

void main_window::populate_options_panel(QListWidgetItem *value, QScrollArea *scroll_area) {
  (void)value;
  (void)scroll_area;
    // m_curr_lib_spec = value->data(Qt::UserRole).value<shared_ptr<lib_specification> >();
    // m_curr_spec_handle = m_curr_lib_spec->available_options;

    // QWidget *container_widget = new QWidget();
    // QVBoxLayout *vlayout = new QVBoxLayout();

    // /////////// Name for both //////////
    // {
    //   QHBoxLayout *hlayout = new QHBoxLayout();
    //   QLabel *prompt = new QLabel("Name of node:");
    //   QLineEdit *name_edit = new QLineEdit();
    //   name_edit->setToolTip("User identifyable name of the node");
    //   hlayout->addWidget(prompt);
    //   hlayout->addWidget(name_edit);
    //   QObject::connect(name_edit, &QLineEdit::textChanged,
    //                   [=](const QString &newValue ) {
    //                     this->m_node_name = newValue.toStdString();
    //                   });
    //   vlayout->addLayout(hlayout);

    // }
    // if(!m_curr_lib_spec->is_source_module) {
    //   QHBoxLayout *hlayout = new QHBoxLayout();
    //   QLabel *prompt = new QLabel("Select parent:");
    //   QLineEdit *parent_edit = new QLineEdit();
    //   parent_edit->setToolTip("User identifyable name of the node");
    //   hlayout->addWidget(prompt);
    //   hlayout->addWidget(parent_edit);
    //   QObject::connect(parent_edit, &QLineEdit::textChanged,
    //                   [=](const QString &newValue ) {
    //                     this->m_parent_node_name = newValue.toStdString();
    //                   });
    //   vlayout->addLayout(hlayout);
    // }

    // int index = 0;
    // for(auto option : *(m_curr_lib_spec->available_options)) {
    //   QHBoxLayout *hlayout = new QHBoxLayout();
    //   QLabel *prompt = new QLabel(option.option_prompt.c_str());
    //   hlayout->addWidget(prompt);
    //   switch(option.type) {
    //     case STRING:
    //       {
    //         QLineEdit *lineEdit = new QLineEdit(option.value.string_value);
    //         lineEdit->setToolTip(option.short_description.c_str());
    //         hlayout->addWidget(lineEdit);
    //         QObject::connect(lineEdit, &QLineEdit::textChanged,
    //                         [=](const QString &newValue ) {
    //                           (*m_curr_spec_handle)[index].value.string_value = strdup(newValue.toStdString().c_str());
    //                         }
    //         );
    //       }
    //       break;
    //     case INT:
    //       {
    //         QSpinBox *integer_box = new QSpinBox();
    //         integer_box->setValue(option.value.int_value);
    //         integer_box->setToolTip(option.short_description.c_str());
    //         hlayout->addWidget(integer_box);
    //       }
    //       break;
    //     case BOOL:
    //       {
    //         QCheckBox *checkbox = new QCheckBox();
    //         checkbox->setChecked(option.value.bool_value);
    //         checkbox->setToolTip(option.short_description.c_str());
    //         hlayout->addWidget(checkbox);
    //       }
    //       break;
    //   }
    //   vlayout->addLayout(hlayout);
    //   index++;
    // }

    // container_widget->setLayout(vlayout);
    // scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    // scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // scroll_area->setWidget(container_widget);
    // scroll_area->setEnabled(true);
    // vlayout->addStretch();
}

main_window::main_window() : m_curr_spec_handle(nullptr), m_curr_lib_spec(nullptr), m_dag() {
  main_ui_window.setupUi(this);
  list = main_ui_window.available_libs;

  QObject::connect(
    list, &QListWidget::itemClicked,
    this, &main_window::refresh_options_panel);
  QObject::connect( 
    main_ui_window.create_button, &QPushButton::released,
    this, &main_window::handle_create);

  QObject::connect( 
    main_ui_window.actionSave, &QAction::triggered,
    this, &main_window::save);
  
  QObject::connect( 
    main_ui_window.actionOpen, &QAction::triggered,
    this, &main_window::load);

  m_dag.initializeView(main_ui_window.current_dag);
}

main_window::~main_window() {}

void main_window::refresh_options_panel( QListWidgetItem *value ) {  
  main_ui_window.statusbar->showMessage(value->statusTip(), 3000); 
  QScrollArea *scroll_area = main_ui_window.options_pane;
  
  populate_options_panel  (value, scroll_area);
}

void main_window::handle_create() {
  if(m_curr_spec_handle != nullptr) {
    for(auto option : *m_curr_spec_handle) {
      std::cout << option.option_prompt << ": ";
      switch (option.type)
      {
      case STRING:
        std::cout << option.value.string_value << std::endl;
        break;
      case INT:
        std::cout << option.value.int_value << std::endl;
        break;
      case BOOL:
        std::cout << option.value.bool_value << std::endl;
        break;
      }
    }

    // shared_ptr<module> new_module = m_curr_lib_spec->instantiate(*m_curr_spec_handle);
    // if(new_module->get_type() == MODULE_TYPE::SOURCE) {
    //   module_source *src = new_module->get_handle_as_source();
    //   m_dag.vin_add_src(m_node_name, m_curr_lib_spec->serial_id_guid, m_curr_spec_handle, src);
    // } else {
    //   module_transmit *filter = new_module->get_slot_handle_as_mapping("no");
    //   m_dag.vin_add_node(m_node_name, m_curr_lib_spec->serial_id_guid, filter, m_curr_spec_handle, m_parent_node_name);
    // }
  }
}

void main_window::save() {
  std::cout << "Save\n";
  //TODO: File dialog
  m_dag.serialize("/Users/drobotnik/projects/oculator/build/test.json");
}

void main_window::load() {
  std::cout << "Load\n";
}


