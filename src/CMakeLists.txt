set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FUNCTIONALDAG_CXX_FLAGS} ${VIZ_CXX_FLAGS} -DVIN_LIB_DIR=${VIN_LIB_DIR}")

##########################################
###### Actual VIN specification  #########
##########################################
if (NOT CLI_ONLY)
  set(SOURCES vin/main.cpp
              vin/vin_dag_manager.cpp
              vin/vin_gui.cpp
              vin/utils/lib_specification.cpp
              vin/utils/vin_library.cpp)
else ()
  set(SOURCES vin/main.cpp
              vin/utils/lib_specification.cpp
              vin/utils/vin_library.cpp)
endif ()

qt_wrap_ui(SOURCES viz/main_dialog.ui)
qt_add_executable(vin ${SOURCES})

if (NOT CLI_ONLY)
  target_link_libraries(vin PUBLIC Qt6::Core FunctionalDag::functional_dag Qt6::Widgets) # ${JSONCPP_LIBRARIES}
else ()
  target_link_libraries(vin PUBLIC Qt6::Core FunctionalDag::functional_dag) # ${JSONCPP_LIBRARIES}
endif ()
set_target_properties(vin
        PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist")

########################################
####### Install specification ##########
########################################
install(TARGETS vin 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)

########################################
####### Lint command (optional) ########
########################################
add_custom_target(lint
  COMMAND clang-tidy vin/main.cpp vin/vin_dag_manager.cpp vin/vin_gui.cpp vin/utils/lib_specification.cpp vin/utils/vin_library.cpp -- -Isrc/ -I${CMAKE_OSX_SYSROOT}/usr/include
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMENT "Linting the source"
  VERBATIM
)
