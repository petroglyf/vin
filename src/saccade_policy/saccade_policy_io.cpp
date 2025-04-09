#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <arrow/api.h>
#pragma GCC diagnostic pop

#include <stdlib.h>
#include <time.h>

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

#include "functional_dag/libutils.h"

const static fn_dag::GUID<fn_dag::node_prop_spec> __saccade_guid =
    *fn_dag::GUID<fn_dag::node_prop_spec>::from_uuid(
        "30c66408-f2f7-459c-abdd-d0b5f8d1e052");

uint64_t gradient_next_state(uint64_t prev_state, const arrow::Tensor *heatmap);

enum policy_type { UNDEFINED, CONSTANT, RANDOM, SALIENCY };

static uint64_t epoch_time_ms() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch())
      .count();
}

template <typename T>
int sgn(T val) {
  return (T(0) < val) - (val < T(0));
}

class saccade_op : public fn_dag::dag_node<arrow::Tensor, arrow::Tensor> {
 public:
  saccade_op(policy_type _op_code, const uint8_t _n_points)
      : m_op_code(_op_code), m_npts(_n_points), m_is_initialized(false) {
    srand(time(NULL));

    prev_gpoints = new uint32_t[2 * _n_points];
    prev_points = new uint32_t[2 * _n_points];

    memset(prev_gpoints, 0, 2 * _n_points * sizeof(uint32_t));
    memset(prev_points, 0, 2 * _n_points * sizeof(uint32_t));
  }

  ~saccade_op() {
    delete[] prev_gpoints;
    delete[] prev_points;
  }

  std::unique_ptr<arrow::Tensor> update(const arrow::Tensor *_input_tensor) {
    const uint32_t max_y = _input_tensor->shape()[1];
    const uint32_t max_x = _input_tensor->shape()[2];

    auto local_buffer = arrow::Buffer::Wrap(prev_points, 2 * m_npts);
    auto output_tensor = std::make_unique<arrow::Tensor>(
        arrow::uint32(), local_buffer, std::vector<int64_t>(m_npts, 2));

    // Update the goals
    if (!m_is_initialized) {
      for (uint32_t i = 0; i < m_npts; i++) {
        prev_gpoints[i * 2] = random() % max_x;
        prev_gpoints[i * 2 + 1] = random() % max_y;

        prev_points[i * 2] = random() % max_x;
        prev_points[i * 2 + 1] = random() % max_y;
      }
      m_last_update_ms = epoch_time_ms();
      m_is_initialized = true;
    }

    switch (m_op_code) {
      case RANDOM:
        if (epoch_time_ms() - m_last_update_ms > 3000) {
          for (uint32_t i = 0; i < m_npts; i++) {
            prev_gpoints[i * 2] = random() % max_x;
            prev_gpoints[i * 2 + 1] = random() % max_y;
          }
          m_last_update_ms = epoch_time_ms();
        }
      case CONSTANT:
        break;
      case SALIENCY:
        for (uint32_t i = 0; i < m_npts; i++) {
          uint64_t prev_state =
              ((uint64_t)prev_points[i * 2] << 32) + prev_points[i * 2 + 1];

          uint64_t next_state = gradient_next_state(prev_state, _input_tensor);

          prev_gpoints[i * 2] = next_state >> 32;
          prev_gpoints[i * 2 + 1] = (next_state << 32) >> 32;
        }
        break;
      default:
        std::cout << "skipping, defaulting to constant\n";
        return nullptr;
    }

    // move all of the points toward the goal points
    for (uint32_t i = 0; i < m_npts; i++) {
      std::function<uint32_t(uint32_t, uint32_t)> get_delt =
          [](uint32_t prev_coord, uint32_t goal_coord) {
            int32_t dx = (int32_t)goal_coord - (int32_t)prev_coord;
            return std::max(std::min(abs(dx) / 2, 13), 1) * sgn(dx);
          };
      prev_points[i * 2] += get_delt(prev_points[i * 2], prev_gpoints[i * 2]);
      prev_points[i * 2 + 1] +=
          get_delt(prev_points[i * 2 + 1], prev_gpoints[i * 2 + 1]);
    }

    return output_tensor;
  }

 private:
  policy_type m_op_code;
  uint32_t m_npts;
  uint32_t *prev_gpoints;
  uint32_t *prev_points;
  bool m_is_initialized;
  uint64_t m_last_update_ms;
};

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wreturn-type-c-linkage"
#endif

#define DL_EXPORT __attribute__((visibility("default")))

extern "C" DL_EXPORT fn_dag::library_spec get_library_details() {
  // All UUID conversions should always work if it ever worked at all.
  fn_dag::library_spec spec{
      .guid = *fn_dag::GUID<fn_dag::library>::from_uuid(
          "de3df94f-837a-4c36-a9bf-3ebfb98f37bd"),
      .available_nodes = {},
  };

  // This is the operators that we support
  spec.available_nodes.push_back(fn_dag::node_prop_spec{
      .guid = __saccade_guid,
      .name = "Saccade tools",
      .description = "This provides attention and fixation points to the agent",
      .module_type = fn_dag::NODE_TYPE::NODE_TYPE_FILTER,
      .construction_types =
          {
              fn_dag::option_spec{
                  .type = fn_dag::OPTION_TYPE::OPTION_TYPE_INT,
                  .name = "policy",
                  .option_prompt = "Policy type to use from the heatmap.",
                  .short_description = "Think of this as a look-at strategy."
                                       "Supported options are (1) CONSTANT, "
                                       "(2) RANDOM, (3) SALIENCY.",
              },
              fn_dag::option_spec{
                  .type = fn_dag::OPTION_TYPE::OPTION_TYPE_INT,
                  .name = "n_points",
                  .option_prompt = "The number of look at points to generate.",
                  .short_description =
                      "Sometimes the robot has multiple eyes. This allows us "
                      "to generate multiple look at points. Value must be "
                      "greater than zero.",
              },
          },
  });
  return spec;
}

extern "C" DL_EXPORT bool construct_node(
    fn_dag::dag_manager<std::string> &manager, const fn_dag::node_spec &spec) {
  // Make sure the node is what we expected
  if (__saccade_guid.m_id.bits1() == spec.target_id()->bits1() &&
      __saccade_guid.m_id.bits2() == spec.target_id()->bits2()) {
    // Get the name and options for instantiation
    std::string_view name = spec.name()->string_view();
    policy_type policy_code = policy_type::UNDEFINED;
    int n_points = -1;

    for (auto option : *spec.options()) {
      if (option->value()->type() == fn_dag::OPTION_TYPE_INT) {
        if (option->name()->string_view() == "policy") {
          policy_code = static_cast<policy_type>(option->value()->int_value());
        } else if (option->name()->string_view() == "n_points") {
          n_points = option->value()->int_value();
        }
      }
    }

    // Now get the parent node name
    if (spec.wires()->size() > 0) {
      std::string_view parent_node_name =
          spec.wires()->Get(0)->value()->string_view();
      saccade_op *new_filter = new saccade_op(policy_code, n_points);
      if (auto parent_ret = manager.add_node(std::string(name), new_filter,
                                             std::string(parent_node_name));
          parent_ret.has_value()) {
        return parent_node_name == *parent_ret;
      } else {
        delete new_filter;
        std::cout << "Error: Could not add node to the dag. Error code: "
                  << parent_ret.error() << std::endl;
      }
    } else {
      std::cout << "Error: No parent node name found for in options for "
                << name << std::endl;
    }
  }
  return false;
}
