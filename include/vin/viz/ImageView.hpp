#pragma once
/**
 *           _________ _       
 *  |\     /|\__   __/( (    /|
 *  | )   ( |   ) (   |  \  ( |
 *  ( (   ) )   | |   | (\ \) |
 *   \ \_/ /    | |   | | \   |
 *    \   /  ___) (___| )  \  |
 *     \_/   \_______/|/    )_)
 *                             
 * ImageView displays RGB and heat maps for the data being streamed.
 * 
 * @author: ndepalma@alum.mit.edu
 * @license: MIT License
 */ 

#include <QtWidgets/QLabel>

#include <mutex>

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
    
  private:
    QList<QRect> m_rectangles_to_draw; // internal list of rectangles to draw per frame
    std::vector< std::tuple<uint32_t, uint32_t> > m_gaze_pts;
    std::mutex m_gaze_pts_mutex;

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

    void setGazePts(std::vector< std::tuple<uint32_t, uint32_t> > new_pts);

    /**
     * setTensor
     * 
     * @param tensor The image to display
     * @param mode How to display it
     */
    void setTensor(const DLTensor &tensor, const VizMode mode);

    /**
     * drawBox
     * 
     * @param x top-x location of the box on the image
     * @param y top-y location of the box on the image
     * @param width width of the box on the image
     * @param height height of the box on the image
     */
    void drawBox(int x, int y, int width, int height);

    /**
     * clearBoxOverlay
     * 
     */
    void clearBoxOverlay();
  };
}

