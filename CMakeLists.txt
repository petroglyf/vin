cmake_minimum_required(VERSION 3.6)
project(vin LANGUAGES CXX )

#############################################
####### Setting up compiler options #########
#############################################
include(FindPkgConfig REQUIRED)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_path(APPEND_STRING CMAKE_INSTALL_PREFIX "/lib/vin_libs/" OUTPUT_VARIABLE VIN_LIB_DIR)
get_filename_component(VIN_LIB_DIR "${VIN_LIB_DIR}" ABSOLUTE  )
message("${VIN_LIB_DIR}")

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
option (BUILD_QTIO "Build QTIO support" ON)
option (BUILD_QTOP "Build QTOP support" ON)

##########################################
####### Find the needed packages #########
##########################################
find_package(PkgConfig REQUIRED)
find_package(functional_dag REQUIRED)

set(CMAKE_AUTOUIC ON)  
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

file (GLOB_RECURSE SOURCES src/main.cpp 
                           src/vin_gui.cpp 
                           src/vin_dag_manager.cpp)

find_package(Qt6 COMPONENTS Widgets Multimedia MultimediaWidgets REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FUNCTIONALDAG_CXX_FLAGS} ${VIZ_CXX_FLAGS} -DVIN_LIB_DIR=${VIN_LIB_DIR}")

##########################################
###### Actual VIN specification  #########
##########################################
file(GLOB HEADERS_TO_MOC_VIN include/vin/viz/ui_putitup.hpp)
qt6_wrap_cpp(PROCESSED_MOCS_VIN                                                                                                                                                                                                                                                                    
            ${HEADERS_TO_MOC_VIN}                                                   
            TARGET vin
            OPTIONS --no-notes)
add_executable(vin ${SOURCES})

target_link_libraries(vin PUBLIC Qt6::Core Qt6::Widgets FunctionalDag::functional_dag)


##############################
###### QTIO library  #########
##############################
if (BUILD_QTIO)
  ###### QTIO Input video suppport  #########
  file(GLOB QTIO_HEADERS_TO_MOC include/vin/viz/qt_io.hpp)
  qt6_wrap_cpp(QTIO_PROCESSED_MOCS                                                                                                                                                                                                                                                                    
                  ${QTIO_HEADERS_TO_MOC}                                                   
                  TARGET qtio
                  OPTIONS --no-notes) # Don't display a note for the headers which don't produce a moc_*.cpp

  add_library(qtio SHARED src/vin/source_tools/qt_io.cpp ${QTIO_PROCESSED_MOCS})
  target_include_directories(qtio PRIVATE "include/" Qt6::Core Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets)
  target_compile_options(qtio PRIVATE -fPIC -fvisibility=hidden)
  target_link_libraries(qtio PRIVATE Qt6::Core Qt6::Multimedia Qt6::MultimediaWidgets FunctionalDag::functional_dag)

  ###### QTIO Output image suppport  #########
  file(GLOB HEADERS_TO_MOC include/vin/viz/ImageView.hpp)
  qt6_wrap_cpp(PROCESSED_MOCS                                                                                                                                                                                                                                                                    
                  ${HEADERS_TO_MOC}                                                   
                  TARGET imageview
                  OPTIONS --no-notes) # Don't display a note for the headers which don't produce a moc_*.cpp

  add_library(imageview SHARED src/viz/putitup.ui src/viz/ImageView.cpp src/viz/imageview_io.cpp ${PROCESSED_MOCS})
  target_include_directories(imageview PRIVATE "include/" Qt6::Widgets)
  target_compile_options(imageview PRIVATE -fPIC -fvisibility=hidden)
  target_link_libraries(imageview PRIVATE Qt6::Core Qt6::Widgets FunctionalDag::functional_dag)

  install(TARGETS imageview 
      LIBRARY DESTINATION lib/vin_libs
      ARCHIVE DESTINATION lib/vin_libs
      RUNTIME DESTINATION bin)
  install(TARGETS qtio 
      LIBRARY DESTINATION lib/vin_libs
      ARCHIVE DESTINATION lib/vin_libs
      RUNTIME DESTINATION bin)
endif ()

##############################
###### QTOP library  #########
##############################
if (BUILD_QTOP)
  add_library(qtop SHARED src/vin/source_tools/qt_op.cpp)
  target_include_directories(qtop PRIVATE "include/" Qt6::Core Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets)
  target_compile_options(qtop PRIVATE -fPIC -fvisibility=hidden)
  target_link_libraries(qtop PRIVATE Qt6::Core Qt6::Multimedia Qt6::MultimediaWidgets FunctionalDag::functional_dag)

  install(TARGETS qtop 
      LIBRARY DESTINATION lib/vin_libs
      ARCHIVE DESTINATION lib/vin_libs
      RUNTIME DESTINATION bin)
endif ()

########################################
####### Install specification ##########
########################################
install(TARGETS vin 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)