#include <iostream>
#include <cstdlib>
#include <chrono>

#include <functional_dag/dlpack.h>
#include <functional_dag/lib_utils.h>
#include <stdlib.h>
#include <time.h>

uint64_t gradient_next_state(uint64_t prev_state, const DLTensor *heatmap);

enum POLICY_TYPE {
  CONSTANT, RANDOM, SALIENCY
};

static uint64_t timeSinceEpochMillisec() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

template <typename T> 
int sgn(T val) {
  return (T(0) < val) - (val < T(0));
}

class saccade_op : public fn_dag::module_transmit {
public:
  saccade_op(POLICY_TYPE _op_code, const uint8_t _n_points) 
                            : m_op_code(_op_code),
                              m_npts(_n_points),
                              m_is_initialized(false)
  {
    srand (time(NULL));

    prev_gpoints = new uint32_t[2*_n_points];
    prev_points = new uint32_t[2*_n_points];

    memset(prev_gpoints, 0, 2*_n_points*sizeof(uint32_t));
    memset(prev_points, 0, 2*_n_points*sizeof(uint32_t));
  }

  ~saccade_op() {
    delete[] prev_gpoints;
    delete[] prev_points;
  }

  DLTensor *update(const DLTensor *_input_dltensor) {
    const uint32_t max_y = _input_dltensor->shape[1];
    const uint32_t max_x = _input_dltensor->shape[2];

    uint32_t *coords_out = nullptr;
    DLTensor *output_tensor = new DLTensor;

    output_tensor->device.device_type = DLDeviceType::kDLCPU;
    output_tensor->ndim = 2;
    output_tensor->shape = new int64_t[]{m_npts, 2};
    output_tensor->strides = NULL;
    output_tensor->byte_offset = 0;
    output_tensor->dtype.code = DLDataTypeCode::kDLUInt;
    output_tensor->dtype.bits = 8;
    output_tensor->dtype.lanes = 2;
    output_tensor->data = coords_out = new uint32_t[m_npts*2];

    // Update the goals
    if(!m_is_initialized) {
      for(uint32_t i = 0;i < m_npts;i++) {
        prev_gpoints[i*2] = random() % max_x;
        prev_gpoints[i*2+1] = random() % max_y;

        prev_points[i*2] = random() % max_x;
        prev_points[i*2+1] = random() % max_y;
      }
      m_last_update_ms = timeSinceEpochMillisec();
      m_is_initialized = true;
    }

    switch(m_op_code) {
      case RANDOM:
        if(timeSinceEpochMillisec() - m_last_update_ms > 3000) {
          for(uint32_t i = 0;i < m_npts;i++) {
            prev_gpoints[i*2] = random() % max_x;
            prev_gpoints[i*2+1] = random() % max_y;
          }
          m_last_update_ms = timeSinceEpochMillisec();
        }
      case CONSTANT:
        break;
      case SALIENCY:
        for(uint32_t i = 0;i < m_npts;i++) {
          uint64_t prev_state = ((uint64_t)prev_points[i*2] << 32) + prev_points[i*2+1];
          
          uint64_t next_state = gradient_next_state(prev_state, _input_dltensor);
          
          prev_gpoints[i*2] = next_state >> 32;
          prev_gpoints[i*2+1] = (next_state << 32) >> 32;
        }
        break;
      default:
        std::cout << "skipping, defaulting to constant\n";
        return nullptr;
    }

    // move all of the points toward the goal points
    for(uint32_t i = 0;i < m_npts;i++) {
      std::function<uint32_t(uint32_t, uint32_t)> get_delt = [](uint32_t prev_coord, uint32_t goal_coord) {
        int32_t dx = (int32_t)goal_coord-(int32_t)prev_coord;
        return std::max(std::min(abs(dx) / 2, 13), 1)*sgn(dx);
      };
      prev_points[i*2] += get_delt(prev_points[i*2], prev_gpoints[i*2]);
      prev_points[i*2+1] += get_delt(prev_points[i*2+1], prev_gpoints[i*2+1]);
    }
    
    memcpy(coords_out, prev_points, sizeof(uint32_t)*2*m_npts);

    return output_tensor;
  }

private:
  POLICY_TYPE m_op_code;
  uint32_t m_npts;
  uint32_t *prev_gpoints;
  uint32_t *prev_points;
  bool m_is_initialized;
  uint64_t m_last_update_ms;
};

#pragma GCC diagnostic ignored "-Wreturn-type-c-linkage"

#define DL_EXPORT __attribute__ ((visibility ("default")))

extern "C" DL_EXPORT std::string get_simple_description() {
  return std::string("This provides attention and fixation points to the agent");
}

extern "C" DL_EXPORT std::string get_detailed_description() {
  return std::string("Agents fixate on specific points in the world. "
                     "Many of the algorithms we use look randomly or use saliency. "
                     "Gaze is communicative however. "
                     "This library provides a few routines to help pick points out of heat maps or selects them randomly.");
}

extern "C" DL_EXPORT std::string get_name() {
  return std::string("Gaze ops");
}

extern "C" DL_EXPORT long get_serial_guid() {
  return 24682;
}

extern "C" DL_EXPORT bool is_source() {
  return false;
}

extern "C" DL_EXPORT shared_ptr<fn_dag::lib_options> get_options() {
  shared_ptr<fn_dag::lib_options> options(new fn_dag::lib_options());
  return options;
}

extern "C" DL_EXPORT fn_dag::module *get_module(const fn_dag::lib_options *options) {
  (void)options;
  fn_dag::module_handler *vlc_out = new fn_dag::module_handler(new saccade_op(POLICY_TYPE::SALIENCY, 3));
  return (fn_dag::module *)vlc_out;
}
