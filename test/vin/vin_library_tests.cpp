#include <catch2/catch_test_macros.hpp>
#include "vin/utils/vin_library.hpp"


TEST_CASE( "Test default libraries exist", "[vin_library.defaults_exist]" ) {
  vin::vin_library library;
  library.initialize();

  REQUIRE(library.get_specs().size() == 4);
}

TEST_CASE( "Test libraries named", "[vin_library.lib_named]" ) {
  vin::vin_library library;
  library.initialize();

  for(auto spec : library.get_specs()) {
    std::cout << "Checking library name : " << spec->lib_name << std::endl;
    REQUIRE(spec->lib_name.length() > 0);
  }
}

TEST_CASE( "Test libraries have descriptions", "[vin_library.have_desc]" ) {
  vin::vin_library library;
  library.initialize();

  for(auto spec : library.get_specs()) {
    REQUIRE(spec->simple_description.length() > 0);
    REQUIRE(spec->detailed_description.length() > 0);
  }
}

TEST_CASE( "Test libraries have unique GUIDs", "[vin_library.unique_guids]" ) {
  unordered_set<long> all_guids;
  vin::vin_library library;
  library.initialize();

  for(auto spec : library.get_specs()) {
    all_guids.emplace(spec->serial_id_guid);
  }
  
  REQUIRE(all_guids.size() == 4);
}