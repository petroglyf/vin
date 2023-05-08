#pragma once
#include <functional_dag/dlpack.h>

namespace image_utils {
  /** 
   * Load an image from a file and return a DLPack tensor.
   */
  DLTensor loadFile(const char * const filename);
}
