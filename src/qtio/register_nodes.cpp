#include <functional_dag/guid_generated.h>
#include <functional_dag/lib_spec_generated.h>
#include <functional_dag/libutils.h>
#include <glog/logging.h>

#include <functional_dag/guid_impl.hpp>
#ifndef CLI_ONLY
#include "qtio/image_view.hpp"
#endif
#include "qtio/qt_io.hpp"

const static fn_dag::GUID<fn_dag::node_prop_spec> __qt_input_guid =
    *fn_dag::GUID<fn_dag::node_prop_spec>::from_uuid(
        "eee7be8e-1124-4ed9-a4c6-0d11886da403");

const static fn_dag::GUID<fn_dag::node_prop_spec> __qt_output_guid =
    *fn_dag::GUID<fn_dag::node_prop_spec>::from_uuid(
        "5322d8fa-be5c-4007-980c-1cd85d0a6887");

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wreturn-type-c-linkage"
#endif

#define DL_EXPORT __attribute__((visibility("default")))

extern "C" DL_EXPORT fn_dag::library_spec get_library_details() {
  // All UUID conversions should always work if it ever worked at all.
  fn_dag::library_spec spec{
      .guid = *fn_dag::GUID<fn_dag::library>::from_uuid(
          "9eab5116-8550-4f0e-ad01-f2ab91367491"),
      .available_nodes = {},
  };

  // Add the input node
  spec.available_nodes.push_back(fn_dag::node_prop_spec{
      .guid = __qt_input_guid,
      .name = "QT Multimedia input",
      .description =
          "This library will load video files with QT multimedia and push it "
          "through the DAG.",
      .module_type = fn_dag::NODE_TYPE::NODE_TYPE_SOURCE,
      .construction_types =
          {
              fn_dag::option_spec{
                  .type = fn_dag::OPTION_TYPE::OPTION_TYPE_STRING,
                  .name = "source_uri",
                  .option_prompt = "Local location of file.",
                  .short_description =
                      "This specifies the source of the video. Reserved "
                      "keyword \"camera\" represents capture from camera.",
              },
              fn_dag::option_spec{
                  .type = fn_dag::OPTION_TYPE::OPTION_TYPE_INT,
                  .name = "width",
                  .option_prompt = "Width of the capture image.",
                  .short_description =
                      "Width of the output image. Set either of these to zero "
                      "to keep the source resolution.",
              },
              fn_dag::option_spec{
                  .type = fn_dag::OPTION_TYPE::OPTION_TYPE_INT,
                  .name = "height",
                  .option_prompt = "Height of the capture image.",
                  .short_description =
                      "Height of the output image. Set either of these to zero "
                      "to keep the source resolution.",
              },
          },
  });
#ifndef CLI_ONLY
  // Add the output node if we support GUIs
  spec.available_nodes.push_back(fn_dag::node_prop_spec{
      .guid = __qt_output_guid,
      .name = "Image viewer output",
      .description =
          "This package will take an image from the DAG and visualize just the "
          "image in a simple window.",
      .module_type = fn_dag::NODE_TYPE::NODE_TYPE_FILTER,
      .construction_types = {},
  });
#endif
  return spec;
}

extern "C" DL_EXPORT bool construct_node(
    fn_dag::dag_manager<std::string> &manager, const fn_dag::node_spec &spec) {
  // Check to see if we are creating a capture node
  if (__qt_input_guid.m_id.bits1() == spec.target_id()->bits1() &&
      __qt_input_guid.m_id.bits2() == spec.target_id()->bits2()) {
    // Get the name and options for instantiation
    std::string_view name = spec.name()->string_view();

    int width = -1;
    int height = -1;
    std::string source_uri;
    for (auto option : *spec.options()) {
      if (option->value()->type() == fn_dag::OPTION_TYPE_INT) {
        if (option->name()->string_view() == "width") {
          width = option->value()->int_value();
        } else if (option->name()->string_view() == "height") {
          height = option->value()->int_value();
        }
      } else if (option->name()->string_view() == "source_uri") {
        source_uri = option->value()->string_value()->string_view();
      }
    }

    // Now get the parent node name
    qt_video_player *qt_source = new qt_video_player(source_uri, width, height);
    auto dag_created = manager.add_dag(std::string(name), qt_source, true);
    return dag_created.has_value();
  }

#ifndef CLI_ONLY
  // check to see if we're creating a visualizer node
  if (__qt_output_guid.m_id.bits1() == spec.target_id()->bits1() &&
      __qt_output_guid.m_id.bits2() == spec.target_id()->bits2()) {
    // Get the name and options for instantiation
    std::string_view name = spec.name()->string_view();

    // Now get the parent node name
    if (spec.wires()->size() > 0) {
      std::string_view parent_node_name =
          spec.wires()->Get(0)->value()->string_view();
      vin::QtViewer *new_filter = new vin::QtViewer();
      if (auto parent_ret = manager.add_node(std::string(name), new_filter,
                                             std::string(parent_node_name));
          parent_ret.has_value()) {
        return parent_node_name == *parent_ret;
      } else {
        delete new_filter;
        LOG(ERROR) << "Error: Could not add node to the dag. Error code: "
                   << parent_ret.error() << std::endl;
      }
    } else {
      LOG(ERROR) << "Error: No parent node name found for in options for "
                 << name << std::endl;
    }
  }
#endif
  return false;
}
