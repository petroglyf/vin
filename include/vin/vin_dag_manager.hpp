#pragma once 

#include <QAbstractItemModel>
#include <filesystem>
#include <unordered_map>

namespace vin {
  class vin_dag {
  public:
    vin_dag();
    vin_dag(const fs::path file_name);
    
    ~vin_dag();

    int vin_add_node(const std::string &name, 
                      const uint32_t guid,
                      fn_dag::module_transmit *dag_node, 
                      shared_ptr<fn_dag::lib_options> options,
                      const std::string &parent_name);
    int vin_add_src(const std::string &name, 
                      const uint32_t guid,
                      shared_ptr<fn_dag::lib_options> options,  
                      fn_dag::module_source *dag_node);

    void serialize(fs::path file_name);
    void load_from_file(fs::path, const std::unordered_map<uint32_t, fn_dag::instantiate_fn> &);
    void initializeView(QTreeWidget *_view);
  
  private: 
    QTreeWidget *m_dag_tree;
    fn_dag::dag_manager<std::string> m_fn_manager;
    vector<fn_dag::library_spec> m_all_loaded_specs;
    QTreeWidgetItem *m_rootitem;
  };
}
