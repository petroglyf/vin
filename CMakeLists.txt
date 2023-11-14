cmake_minimum_required(VERSION 3.6)
project(vin LANGUAGES CXX VERSION 0.7)

#############################################
####### Setting up compiler options #########
#############################################
include(FindPkgConfig REQUIRED)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_PREFIX_PATH "/opt/homebrew/include")

cmake_path(APPEND_STRING CMAKE_INSTALL_PREFIX "/lib/vin_libs/" OUTPUT_VARIABLE VIN_LIB_DIR)
# get_filename_component(VIN_LIB_DIR "${VIN_LIB_DIR}" ABSOLUTE  )
get_filename_component(VIN_LIB_DIR "/opt/homebrew/Cellar/vin/${CMAKE_PROJECT_VERSION}/lib/vin_libs" ABSOLUTE  )

include_directories("include/" ${CMAKE_CURRENT_BINARY_DIR}/src)
add_compile_options("-stdlib=libc++" "-Wall" "-Wextra" "-Wpedantic" "-Werror" "-DVIN_VERSION=${CMAKE_PROJECT_VERSION}")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option (DEBUG "Build debug version" ON)
option (RELEASE "Build release version" OFF)

if (DEBUG)
  add_definitions(-g -pg)
endif ()

if (RELEASE)
  add_definitions(-Ofast)
endif ()

#############################################
####### User centered build options #########
#############################################
option (BUILD_QTIO "Build QTIO support" ON)
option (BUILD_QTOP "Build QT ops support" ON)
option (BUILD_GAZE_OPS "Build gaze ops support" ON)

##########################################
####### Find the needed packages #########
##########################################
find_package(PkgConfig REQUIRED)
find_package(functional_dag REQUIRED)

set(CMAKE_AUTOUIC ON)  
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)
find_package(Qt6 COMPONENTS Widgets Multimedia MultimediaWidgets REQUIRED)
find_package(Catch2 3 REQUIRED)

qt_standard_project_setup()

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

add_subdirectory(src)
add_subdirectory(src/saccade_policy)
add_subdirectory(src/qtio)

########################################
####### Build the testing files ########
########################################
if (DEFINED Catch2_DIR)
  add_executable(tests test/vin/vin_library_tests.cpp
                       src/vin/utils/vin_library.cpp
                       src/vin/utils/lib_specification.cpp)
  target_include_directories(tests PRIVATE "include/" ${CATCH_INCLUDE_DIR})
  target_link_directories(tests PRIVATE ${CATCH_LIBRARY_DIRS})
  target_link_libraries(tests FunctionalDag::functional_dag Catch2::Catch2WithMain)

  include(CTest)
  include(Catch)
  catch_discover_tests(tests)
endif()