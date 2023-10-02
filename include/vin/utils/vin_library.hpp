#pragma once

#include <vector>
#include <filesystem>

#include "vin/utils/lib_specification.hpp"

namespace fs = std::filesystem;

namespace vin {
  class vin_library {
  public:
    vin_library();
    ~vin_library();

    int initialize();
    std::unordered_map<uint32_t, fn_dag::instantiate_fn> &get_library();
  private:
    std::vector<fs::directory_entry> m_location_of_libraries;
    std::vector<std::shared_ptr<lib_specification>> m_library_specs;
    std::unordered_map<uint32_t, fn_dag::instantiate_fn> m_library;
  };
}