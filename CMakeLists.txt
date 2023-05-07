cmake_minimum_required(VERSION 3.6)
project(vin LANGUAGES CXX )

set(VERBOSE ON)
# TODO: Check to see if Qt can be used with homebrew. Also try this with aptitude 
set(_QTOSX_CMAKE_DIR "/Users/drobotnik/Qt/6.4.2/macos/lib/cmake")

#####################################################
####### Handling Qt if installed in Hombrew #########
#####################################################

if(DEFINED ENV{HOMEBREW_CELLAR})
  message("Found homebrew. Adding Qt libraries since pkg-config Qt support doesn't work with homebrew.")
  set(_CELLAR_CMAKE_DIRS "$ENV{HOMEBREW_CELLAR}/qt@6/6.2.3/lib/cmake")
else()
  
endif() 

set(CMAKE_PREFIX_PATH "${_CELLAR_CMAKE_DIRS};${_QTOSX_CMAKE_DIR}")


#############################################
####### Setting up compiler options #########
#############################################
#### bin2c for models during build
include(FindPkgConfig REQUIRED)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories("include/")
add_compile_options("-stdlib=libc++")

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

option (BUILD_VIN_FRONTEND "Build the frontend to VIN" ON)

option (BUILD_GST "Build GST support" OFF)
option (BUILD_VLC "Build VLC support" OFF)
option (BUILD_QTIO "Build QTIO support" ON)

option (BUILD_IMVIEWER "Build image viewer support" ON)
option (BUILD_3DVIEWER "Build 3D viewer support" OFF)

##########################################
####### Find the needed packages #########
##########################################
find_package(PkgConfig REQUIRED)
find_package(functional_dag REQUIRED)

# pkg_check_modules(GST REQUIRED IMPORTED_TARGET gstreamer-1.0)

set(CMAKE_AUTOUIC ON)  
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

file (GLOB_RECURSE SOURCES src/main.cpp src/vin_gui.cpp vin_dag_manager.cpp)

find_package(Qt6 COMPONENTS Widgets Multimedia MultimediaWidgets REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FUNCTIONALDAG_CXX_FLAGS} ${VIZ_CXX_FLAGS}")


if (BUILD_VIN_FRONTEND)
  file(GLOB HEADERS_TO_MOC_VIN include/oculator/viz/*.hpp)
  qt6_wrap_cpp(PROCESSED_MOCS_VIN                                                                                                                                                                                                                                                                    
              ${HEADERS_TO_MOC_VIN}                                                   
              TARGET vin
              OPTIONS --no-notes)
  add_executable(vin ${SOURCES})
  
  target_include_directories(vin PRIVATE /opt/homebrew/include)
  target_link_libraries(vin PUBLIC Qt6::Core Qt6::Widgets FunctionalDag::functional_dag)
endif()

#####################################
####### G Streamer library  #########
#####################################
if (BUILD_GST)
  add_library(gst SHARED src/oculator/source_tools/gstio.cpp)
  target_include_directories(gst PRIVATE "include/" ${GST_INCLUDE_DIRS})
  target_compile_options(gst PRIVATE -fPIC -fvisibility=hidden)
  target_link_directories(gst PRIVATE "/opt/homebrew/Cellar/gstreamer/1.22.0/lib" "/opt/homebrew/Cellar/glib/2.74.5/lib" "/opt/homebrew/Cellar/gettext/0.21.1/lib")
  target_link_libraries(gst PRIVATE ${GST_LIBRARIES} filterdag)

  add_custom_command(
          TARGET gst POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E make_directory
                  ${CMAKE_CURRENT_BINARY_DIR}/lib)

  add_custom_command(
          TARGET gst POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy
                  ${CMAKE_CURRENT_BINARY_DIR}/libgst.dylib
                  ${CMAKE_CURRENT_BINARY_DIR}/lib/)

  add_custom_command(
          TARGET gst POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E rm
                  ${CMAKE_CURRENT_BINARY_DIR}/libgst.dylib)
  set_property(
          TARGET gst APPEND
          PROPERTY ADDITIONAL_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/lib/libgst.dylib)
endif ()


#######################################
####### Image Viewer library  #########
#######################################

if (BUILD_IMVIEWER)
  file(GLOB HEADERS_TO_MOC include/oculator/viz/ImageView.hpp)
  qt6_wrap_cpp(PROCESSED_MOCS                                                                                                                                                                                                                                                                    
                  ${HEADERS_TO_MOC}                                                   
                  TARGET imageview
                  OPTIONS --no-notes) # Don't display a note for the headers which don't produce a moc_*.cpp


  add_library(imageview SHARED src/viz/putitup.ui src/viz/ImageView.cpp src/viz/imageview_io.cpp ${PROCESSED_MOCS})
  target_include_directories(imageview PRIVATE "include/" Qt6::Widgets /opt/homebrew/include)
  target_compile_options(imageview PRIVATE -fPIC -fvisibility=hidden)
  target_link_libraries(imageview PRIVATE Qt6::Core Qt6::Widgets FunctionalDag::functional_dag)

  add_custom_command(
          TARGET imageview POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E make_directory
                  ${CMAKE_CURRENT_BINARY_DIR}/lib)

  add_custom_command(
          TARGET imageview POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy
                  ${CMAKE_CURRENT_BINARY_DIR}/libimageview.dylib
                  ${CMAKE_CURRENT_BINARY_DIR}/lib/)

  add_custom_command(
          TARGET imageview POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E rm
                  ${CMAKE_CURRENT_BINARY_DIR}/libimageview.dylib)
  set_property(
          TARGET imageview APPEND
          PROPERTY ADDITIONAL_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/lib/libimageview.dylib)
endif ()

##############################
####### VLC library  #########
##############################
if (BUILD_VLC)
  add_library(vlct SHARED src/oculator/source_tools/vlcio.cpp)
  target_include_directories(vlct PRIVATE "include/" "/opt/homebrew/Cellar/libvlc-3.0.16-arm/64/include")
  target_compile_options(vlct PRIVATE -fPIC -fvisibility=hidden)
  target_link_directories(vlct PRIVATE "/opt/homebrew/Cellar/libvlc-3.0.16-arm/64/lib")
  target_link_libraries(vlct PRIVATE filterdag vlc)

  add_custom_command(
          TARGET vlct POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E make_directory
                  ${CMAKE_CURRENT_BINARY_DIR}/lib)

  add_custom_command(
          TARGET vlct POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy
                  ${CMAKE_CURRENT_BINARY_DIR}/libvlct.dylib
                  ${CMAKE_CURRENT_BINARY_DIR}/lib/)

  add_custom_command(
          TARGET vlct POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E rm
                  ${CMAKE_CURRENT_BINARY_DIR}/libvlct.dylib)
  set_property(
          TARGET vlct APPEND
          PROPERTY ADDITIONAL_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/lib/libvlct.dylib)
          
endif ()

##############################
###### QTIO library  #########
##############################
if (BUILD_QTIO)
  file(GLOB QTIO_HEADERS_TO_MOC include/oculator/viz/QTVideoPlayer.hpp)
  qt6_wrap_cpp(QTIO_PROCESSED_MOCS                                                                                                                                                                                                                                                                    
                  ${QTIO_HEADERS_TO_MOC}                                                   
                  TARGET qtio
                  OPTIONS --no-notes) # Don't display a note for the headers which don't produce a moc_*.cpp

  add_library(qtio SHARED src/oculator/source_tools/qtio.cpp ${QTIO_PROCESSED_MOCS})
  target_include_directories(qtio PRIVATE "include/" Qt6::Core Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets /opt/homebrew/include)
  target_compile_options(qtio PRIVATE -fPIC -fvisibility=hidden)
  target_link_libraries(qtio PRIVATE Qt6::Core Qt6::Multimedia Qt6::MultimediaWidgets FunctionalDag::functional_dag)

  add_custom_command(
          TARGET qtio POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E make_directory
                  ${CMAKE_CURRENT_BINARY_DIR}/lib)

  add_custom_command(
          TARGET qtio POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy
                  ${CMAKE_CURRENT_BINARY_DIR}/libqtio.dylib
                  ${CMAKE_CURRENT_BINARY_DIR}/lib/)

  add_custom_command(
          TARGET qtio POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E rm
                  ${CMAKE_CURRENT_BINARY_DIR}/libqtio.dylib)
  set_property(
          TARGET qtio APPEND
          PROPERTY ADDITIONAL_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/lib/libqtio.dylib)
endif ()
