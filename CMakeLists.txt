cmake_minimum_required(VERSION 3.6)
project(vin LANGUAGES CXX )

#############################################
####### Setting up compiler options #########
#############################################
include(FindPkgConfig REQUIRED)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_PREFIX_PATH "/opt/homebrew/include")

cmake_path(APPEND_STRING CMAKE_INSTALL_PREFIX "/lib/vin_libs/" OUTPUT_VARIABLE VIN_LIB_DIR)
# get_filename_component(VIN_LIB_DIR "${VIN_LIB_DIR}" ABSOLUTE  )
get_filename_component(VIN_LIB_DIR "/opt/homebrew/Cellar/vin/1.0/lib/vin_libs" ABSOLUTE  )

message("${VIN_LIB_DIR}")

include_directories("include/" ${CMAKE_CURRENT_BINARY_DIR}/src)
add_compile_options("-stdlib=libc++" "-Wall" "-Wextra" "-Wpedantic" "-Werror")
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

qt_standard_project_setup()

add_subdirectory(src)
add_subdirectory(src/vin/saccade_policy)
add_subdirectory(src/vin/source_tools)


########################################
####### Install specification ##########
########################################
install(TARGETS vin1 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)