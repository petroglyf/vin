#pragma once

#include <string>
#include "functional_dag/lib_utils.h"

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
  fn_dag::lib_options available_options;
  module_getter_fn module_handle;
  
  shared_ptr<fn_dag::module> instantiate(const fn_dag::lib_options &options);

private:
  void * lib_handle;
  void load_lib();
};