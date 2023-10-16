#include <functional_dag/dlpack.h>
#include <cstdlib>
#include <limits>

uint64_t gradient_next_state(uint64_t prev_state, const DLTensor *heatmap) {
  uint32_t x = prev_state >> 32;
  uint32_t y = prev_state & ((uint64_t)std::numeric_limits<std::uint32_t>::max() << 32);
  // Sample around it 
  const uint32_t n_samples = 30;
  uint32_t largest_x = x;
  uint32_t largest_y = y;
  float largest_val = reinterpret_cast<float*>(heatmap->data)[largest_y*heatmap->strides[0]+largest_x];

  for(uint32_t i = 0;i < n_samples;i++) {
    uint32_t dx = rand() % 10;
    dx *= (rand() % 2) == 0 ? -1 : 1; 

    uint32_t dy = rand() % 10;
    dy *= (rand() % 2) == 0 ? -1 : 1; 

    dx += x;
    dy += y;
    // samples[i][0] = x + dx;
    // samples[i][1] = y + dy;
    if(dy > 0 && dy < heatmap->shape[0] && dx > 0 && dx < heatmap->shape[1]) {
      float sample_val = reinterpret_cast<float*>(heatmap->data)[dy*heatmap->strides[0]+dx];
      if(sample_val > largest_val) {
        largest_val = sample_val;
        largest_x = dx;
        largest_y = dy;
      }
    }
  }

  // Take the largest value
  return ((uint64_t)largest_x << 32) + largest_y;
}