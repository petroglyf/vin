#include "vin/utils/lib_specification.hpp"
#include <dlfcn.h>

using namespace fn_dag;

lib_specification::lib_specification() : module_handle(nullptr), lib_handle(nullptr) {}

lib_specification::lib_specification(void * const handle) {
  lib_handle = handle;
  load_lib();
}

void lib_specification::load_lib() {
  using string_getter_fn = string (*)();
  using bool_getter_fn = bool (*)();
  using long_getter_fn = long (*)();
  using options_getter_fn = shared_ptr<lib_options> (*)();
  
  void *guid_get_fn = dlsym(lib_handle, "get_serial_guid");
  serial_id_guid = (long_getter_fn(guid_get_fn)());

  void *name_fn = dlsym(lib_handle, "get_name");
  lib_name = (string_getter_fn(name_fn)());
  
  void *desc_fn = dlsym(lib_handle, "get_simple_description");
  simple_description = (string_getter_fn(desc_fn)());

  desc_fn = dlsym(lib_handle, "get_detailed_description");
  detailed_description = (string_getter_fn(desc_fn)());

  void *is_src_fn = dlsym(lib_handle, "is_source");
  is_source_module = (bool_getter_fn(is_src_fn)());

  void *options_fn = dlsym(lib_handle, "get_options");
  available_options = (options_getter_fn(options_fn)());

  void *get_module_fn = dlsym(lib_handle, "get_module");
  module_handle = module_getter_fn(get_module_fn);
}

shared_ptr<module> lib_specification::instantiate(const lib_options &options) {
  module *new_module = module_handle(&options);
  return shared_ptr<module>(new_module);
}

lib_specification::~lib_specification() {
  if(lib_handle != nullptr)
    dlclose(lib_handle);
  lib_handle = nullptr;
}