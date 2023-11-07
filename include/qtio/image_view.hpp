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
 */ 

#include <mutex>
#include <QtWidgets/QLabel>
#include <functional_dag/dlpack.h>

typedef std::tuple<uint32_t, uint32_t> xy_pt;
namespace vin
{
  /** An enumeration to specify how to interpret the DLTensor data
   * 
   * Vizmode specifies how to interpret the DLTensor data as it comes in
  */
  typedef enum {
    VIZ_RGB, VIZ_HEATMAP
  } VizMode;

  /** A class to view DLTensor images
   * 
   * Imageview is a class used internally to interpret a DLTensor
   * and display an image in what looks like a QLabel to Qt. 
  */
  class ImageView : public QLabel
  {
    Q_OBJECT
    
  private:
    QList<QRect> m_rectangles_to_draw;      // internal list of rectangles to draw per frame
    std::vector< xy_pt > m_gaze_pts;        // Points to render the rectangles
    std::mutex m_gaze_pts_mutex;            // A mutex to get/set the gaze points 

  public:
    /** Constructor
     * 
     * Constructor for the visualizer
     * 
     * @param parent Parent widget (container)
     */
    ImageView(QWidget *parent = nullptr);

    /** Deconstructor
     * 
     * Deconstructor (used for cleanup)
     */
    ~ImageView();

    /** A function to set camera locations to look at
     * 
     * An external setter for other classes to set the next points to render
     * for the gaze points in the image. 
     * 
     * @param new_Pts a list of points in the image to render rectangles
    */
    void setGazePts(std::vector< xy_pt > new_pts);

    /** A function to set what image to display next
     * 
     * setTensor sets the next image to display in the widget
     * 
     * @param tensor The image to display
     * @param mode How to display it
     */
    void setTensor(const DLTensor &tensor, const VizMode mode);

    /** A function to draw boxes where the cameras will center themselves.
     * 
     * drawBox is a public method that will add a rectangle at a
     * specific location. It's helpful because it can be called asynchronously.
     * 
     * @param x top-x location of the box on the image
     * @param y top-y location of the box on the image
     * @param width width of the box on the image
     * @param height height of the box on the image
     */
    void drawBox(int x, int y, int width, int height);

    /** Clears the points to look at
     * 
     * clearBoxOverlay will, asynchronously, clear out the 
     * rectangles so that other methods can set new rectangles
     * later on.
     */
    void clearBoxOverlay();
  };
}

