##############################
###### Gaze library  #########
##############################
if (BUILD_GAZE_OPS)
  add_library(gaze_ops SHARED saccade_policy_io.cpp
                              grad_heatmap.cpp)
  target_include_directories(gaze_ops PRIVATE "include/")
  target_compile_options(gaze_ops PRIVATE -fPIC -fvisibility=hidden)
  target_link_libraries(gaze_ops PRIVATE Qt6::Core FunctionalDag::functional_dag -lstdc++)

  install(TARGETS gaze_ops 
      LIBRARY DESTINATION lib/vin_libs
      ARCHIVE DESTINATION lib/vin_libs
      RUNTIME DESTINATION bin)
endif ()
