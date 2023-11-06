#include <functional_dag/dlpack.h>
#include <cstdlib>
#include <limits>
#include <iostream>

uint64_t gradient_next_state(uint64_t prev_state, const DLTensor *heatmap) {
  uint32_t x = prev_state >> 32;
  uint32_t y = (prev_state << 32) >> 32;
  // Sample around it 
  const uint32_t n_samples = 30;
  uint32_t largest_x = x;
  uint32_t largest_y = y;
  uint8_t largest_val = reinterpret_cast<uint8_t*>(heatmap->data)[largest_y*heatmap->shape[2]+largest_x];
  // std::cout << "largest initial val: " << (uint32_t)largest_val << std::endl;
  for(uint32_t i = 0;i < n_samples;i++) {
    uint32_t dx = rand() % 10;
    dx *= (rand() % 2) == 0 ? -1 : 1; 

    uint32_t dy = rand() % 10;
    dy *= (rand() % 2) == 0 ? -1 : 1; 

    dx += x;
    dy += y;
    // samples[i][0] = x + dx;
    // samples[i][1] = y + dy;
    // std::cout << "Sample xy: " << dx << " " << dy << " / " << heatmap->shape[1] << " " << heatmap->shape[2] << std::endl;
    if(dy > 0 && dy < heatmap->shape[1] && dx > 0 && dx < heatmap->shape[2]) {
      uint8_t sample_val = reinterpret_cast<uint8_t*>(heatmap->data)[dy*heatmap->shape[2]+dx];
      // std::cout << "sample " << i << ": " << (uint32_t)sample_val << std::endl;
      if(sample_val > largest_val) {
        largest_val = sample_val;
        // std::cout << "New largest!\n";
        largest_x = dx;
        largest_y = dy;
      }
    }
  }

  // std::cout << "  state out --> " << largest_x << " , " << largest_y << std::endl; 
  // Take the largest value
  return ((uint64_t)largest_x << 32) + largest_y;
}