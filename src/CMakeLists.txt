set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FUNCTIONALDAG_CXX_FLAGS} ${VIZ_CXX_FLAGS} -DVIN_LIB_DIR=${VIN_LIB_DIR}")

##########################################
###### Actual VIN specification  #########
##########################################
set(SOURCES main.cpp
              vin_dag_manager.cpp
              vin_gui.cpp
              utils/lib_specification.cpp
              utils/vin_library.cpp)

qt_wrap_ui(SOURCES viz/main_dialog.ui)
qt_add_executable(vin1 ${SOURCES})

target_link_libraries(vin1 PUBLIC Qt6::Core Qt6::Widgets FunctionalDag::functional_dag)


########################################
####### Install specification ##########
########################################
install(TARGETS vin1 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)