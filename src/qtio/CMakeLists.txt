##############################
###### QTIO library  #########
##############################
if (BUILD_QTIO)
  ###### QTIO Input video suppport  #########
  file(GLOB QTIO_HEADERS_TO_MOC ../../../include/vin/viz/qt_io.hpp)
  qt6_wrap_cpp(QTIO_PROCESSED_MOCS                                                                                                                                                                                                                                                                    
                  ${QTIO_HEADERS_TO_MOC}                                                   
                  TARGET qtio
                  OPTIONS --no-notes) # Don't display a note for the headers which don't produce a moc_*.cpp

  add_library(qtio SHARED qt_io.cpp ${QTIO_PROCESSED_MOCS} )
  target_include_directories(qtio PRIVATE "include/" Qt6::Core Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets)
  target_compile_options(qtio PRIVATE -fPIC -fvisibility=hidden)
  target_link_libraries(qtio PRIVATE Qt6::Core Qt6::Multimedia Qt6::MultimediaWidgets FunctionalDag::functional_dag)

  ###### QTIO Output image suppport  #########
  file(GLOB HEADERS_TO_MOC ../../../include/vin/viz/ImageView.hpp)
  qt6_wrap_cpp(PROCESSED_MOCS                                                                                                                                                                                                                                                                    
                  ${HEADERS_TO_MOC}                                                   
                  TARGET imageview
                  OPTIONS --no-notes) # Don't display a note for the headers which don't produce a moc_*.cpp

  add_library(imageview SHARED ../../viz/imageview_io.cpp  ../../viz/ImageView.cpp ${PROCESSED_MOCS})
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
  add_library(qtop SHARED qt_op.cpp)
  target_include_directories(qtop PRIVATE "include/" Qt6::Core Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets)
  target_compile_options(qtop PRIVATE -fPIC -fvisibility=hidden)
  target_link_libraries(qtop PRIVATE Qt6::Core Qt6::Multimedia Qt6::MultimediaWidgets FunctionalDag::functional_dag)

  install(TARGETS qtop 
      LIBRARY DESTINATION lib/vin_libs
      ARCHIVE DESTINATION lib/vin_libs
      RUNTIME DESTINATION bin)
endif ()