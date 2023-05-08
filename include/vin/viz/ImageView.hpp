#pragma once
/**
 *   ____  ____  _     _     ____  _____  ____  ____ 
 *  /  _ \/   _\/ \ /\/ \   /  _ \/__ __\/  _ \/  __\
 *  | / \||  /  | | ||| |   | / \|  / \  | / \||  \/|
 *  | \_/||  \_ | \_/|| |_/\| |-||  | |  | \_/||    /
 *  \____/\____/\____/\____/\_/ \|  \_/  \____/\_/\_\
 * 
 * ImageView displays RGB and heat maps for the data being streamed.
 * 
 * @author: ndepalma@alum.mit.edu
 * @license: MIT License
 */ 

#include <QtWidgets/QLabel>
#include <functional_dag/dlpack.h>

#define TEST_LIB_EXPORT Q_DECL_EXPORT

namespace vin
{
  typedef enum {
    VIZ_RGB, VIZ_HEATMAP
  } VizMode;

  class TEST_LIB_EXPORT ImageView : public QLabel
  {
    Q_OBJECT
    
  public:
    /**
     * Constructor for the visualizer
     * 
     * @param parent Parent widget (container)
     */
    ImageView(QWidget *parent = nullptr);

    /**
     * Deconstructor (used for cleanup)
     */
    ~ImageView();

    /**
     * setTensor
     * 
     * @param tensor The image to display
     * @param mode How to display it
     */
    void setTensor(const DLTensor &tensor, const VizMode mode);
  };
}

