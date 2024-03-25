cmake_minimum_required(VERSION 3.6)
project(vin LANGUAGES CXX VERSION 0.7)

#############################################
####### Setting up compiler options #########
#############################################
include(FindPkgConfig REQUIRED)
include(CheckFunctionExists)

if (NOT DEFINED ARCHITECTURE_ID)
  set (ARCHITECTURE_ID "apple")
endif ()

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE)
  set (CMAKE_PREFIX_PATH "/opt/homebrew/include")
endif ()

cmake_path(APPEND_STRING CMAKE_INSTALL_PREFIX "/lib/vin_libs/" OUTPUT_VARIABLE VIN_LIB_DIR)

if (APPLE)
  get_filename_component(VIN_LIB_DIR "/opt/homebrew/Cellar/vin/${CMAKE_PROJECT_VERSION}/lib/vin_libs" ABSOLUTE  )
else ()
  get_filename_component(VIN_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/vin_libs" ABSOLUTE)
endif ()

include_directories("include/" ${CMAKE_CURRENT_BINARY_DIR}/src)
add_compile_options("-Wall" "-Wextra" "-Wpedantic" "-Werror" "-DVIN_VERSION=${CMAKE_PROJECT_VERSION}")
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
pkg_check_modules(JSONCPP REQUIRED IMPORTED_TARGET jsoncpp)

set(CMAKE_AUTOUIC ON)  
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)
find_package(Qt6 COMPONENTS Widgets Multimedia MultimediaWidgets REQUIRED)
find_package(Catch2 3)

check_function_exists(qt_standard_project_setup QT_STANDARD_PROJECT_EXISTS)
if (QT_STANDARD_PROJECT_EXISTS)
  qt_standard_project_setup()
endif ()

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

add_subdirectory(src)
add_subdirectory(src/saccade_policy)
add_subdirectory(src/qtio)

########################################
####### Build the testing files ########
########################################
if (DEFINED Catch2_DIR AND DEFINED ${Catch2_DIR})
  add_executable(tests test/vin/vin_library_tests.cpp
                       src/vin/utils/vin_library.cpp
                       src/vin/utils/lib_specification.cpp)
  target_include_directories(tests PRIVATE "include/" ${CATCH_INCLUDE_DIR})
  target_link_directories(tests PRIVATE ${CATCH_LIBRARY_DIRS})
  target_link_libraries(tests FunctionalDag::functional_dag Catch2::Catch2WithMain -lstdc++)

  include(CTest)
  include(Catch)
  catch_discover_tests(tests)
endif()


########################################
##### Packaging specifications #########
########################################
set(CPACK_PACKAGE_DIRECTORY ${PROJECT_BINARY_DIR}/package)
set(CPACK_SOURCE_IGNORE_FILES .git/ .github/ .vscode/ .mypy_cache/ _CPack_Packages/
${CMAKE_BINARY_DIR}/ ${PROJECT_BINARY_DIR}/
)

set(CPACK_PACKAGE_CONTACT "ndepalma@alum.mit.edu")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Nick DePalma <${CPACK_PACKAGE_CONTACT}>")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${ARCHITECTURE_ID}")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libjsoncpp25 (>= 1.9), libc6 (>= 2), libqt6multimedia6 (>= 6.2), libqt6core6 (>= 6.2), functional-dag (>= 0.1)")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "VIN is a set of tools to integrate simple video into minimal DAGs for processing."
    CACHE STRING "This tool uses the minimal functional dag and uses Qt to read video streams for later processing. There is a set of Qt ops for operating on the video like resizing and viewing. VIN is mostly agnostic about what happens in the middle.")

set(CPACK_PACKAGE_NAME ${PROJECT_NAME}
    CACHE STRING "VIN is a tool for reading and processing video streams. Minimally with the fewest smallest dependencies.")
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CMAKE_PROJECT_VERSION}.${ARCHITECTURE_ID})


include(CPack) 
