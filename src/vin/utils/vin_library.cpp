#include "vin/utils/vin_library.hpp"
#include "vin/error_codes.hpp"
#include "vin/utils/lib_specification.hpp"
#include <dlfcn.h>

#define XSTR(x) STR(x)
#define STR(x) #x

namespace vin {
  shared_ptr<fn_dag::module> __instantiate_fn_prototype(std::shared_ptr<lib_specification> lib_handle, 
                                            const fn_dag::lib_options * const opts) {
    shared_ptr<fn_dag::module> module_ptr = lib_handle->instantiate(*opts);
    return module_ptr;
  }

  shared_ptr<lib_specification> fsys_load_lib(fs::path vin_lib) {
    shared_ptr<lib_specification> spec(new lib_specification{dlopen(vin_lib.c_str(), RTLD_NOW)});
    return spec;
  }

  vin_library::vin_library() {}
  vin_library::~vin_library() {}

  int vin_library::initialize() {
    const char *lib_list[] = {XSTR(VIN_LIB_DIR),
                              "./lib",
                              "./"};

    for(const char *lib_dir : lib_list) {
      std::cout << "Searching for modules in directory " << lib_dir << " -> ";

      auto all_libs = get_all_available_libs(fs::directory_entry(lib_dir));
      if(all_libs->size() > 0) {
        m_location_of_libraries.insert( m_location_of_libraries.end(), all_libs->begin(), all_libs->end() );
        std::cout << "found " << all_libs->size() << " candidates.\n";
      }   
    }

    if(m_location_of_libraries.size() == 0)
      return ERR_LIBRARY_EMPTY;
    std::cout << "\nLoading library of modules.. !\n\t          <library-name>:<guid>\n";

    
    for(auto fs_path: m_location_of_libraries) {
      if(preflight_lib(fs_path)) {
        std::shared_ptr<lib_specification> lib_handle = fsys_load_lib(fs_path);
        m_library_specs.push_back(lib_handle);
        fn_dag::instantiate_fn create_fn = std::bind(__instantiate_fn_prototype, lib_handle, std::placeholders::_1);
        std::cout << "\tSuccess : " << lib_handle->lib_name << ":" << lib_handle->serial_id_guid << std::endl;
        m_library.emplace(lib_handle->serial_id_guid, create_fn);
      } else {
        std::cout << "\tFailure : " << fs_path << std::endl;
      }
    }
    return ERR_NO_ERROR;
  }

  std::unordered_map<uint32_t, fn_dag::instantiate_fn> &vin_library::get_library() {
    return m_library;
  }
} // namespace vin
