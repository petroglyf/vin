#include <cstdlib>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <arrow/api.h>
#pragma GCC diagnostic pop

uint64_t gradient_next_state(uint64_t prev_state,
                             const arrow::Tensor *heatmap) {
  uint32_t x = prev_state >> 32;
  uint32_t y = (prev_state << 32) >> 32;
  // Sample around it
  const uint32_t n_samples = 30;
  uint32_t largest_x = x;
  uint32_t largest_y = y;
  uint8_t largest_val = heatmap->Value<arrow::UInt8Type>(
      std::vector<int64_t>{largest_y, largest_x});

  for (uint32_t i = 0; i < n_samples; i++) {
    uint32_t dx = rand() % 10;
    dx *= (rand() % 2) == 0 ? -1 : 1;

    uint32_t dy = rand() % 10;
    dy *= (rand() % 2) == 0 ? -1 : 1;

    dx += x;
    dy += y;

    if (dy > 0 && dy < heatmap->shape()[1] && dx > 0 &&
        dx < heatmap->shape()[2]) {
      uint8_t sample_val =
          heatmap->Value<arrow::UInt8Type>(std::vector<int64_t>{dy, dx});

      if (sample_val > largest_val) {
        largest_val = sample_val;
        largest_x = dx;
        largest_y = dy;
      }
    }
  }

  // Take the largest value
  return ((uint64_t)largest_x << 32) + largest_y;
}