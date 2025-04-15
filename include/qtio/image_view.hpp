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
 * image_view displays RGB and heat maps for the data being streamed.
 *
 * @author: ndepalma@alum.mit.edu
 */

#include <QtWidgets/QLabel>
#include <mutex>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <arrow/api.h>
#pragma GCC diagnostic pop

#include "functional_dag/dag_interface.hpp"

typedef std::tuple<uint32_t, uint32_t> xy_pt;
namespace vin {
/** An enumeration to specify how to interpret the DLTensor data
 *
 * Vizmode specifies how to interpret the DLTensor data as it comes in
 */
typedef enum { VIZ_RGB, VIZ_HEATMAP } visualization_mode;

/** A class to view DLTensor images
 *
 * Imageview is a class used internally to interpret a DLTensor
 * and display an image in what looks like a QLabel to Qt.
 */
class image_view : public QLabel {
  Q_OBJECT

 private:
  QList<QRect>
      m_rectangles_to_draw;  // internal list of rectangles to draw per frame
  std::vector<xy_pt> m_gaze_pts;  // Points to render the rectangles
  std::mutex m_gaze_pts_mutex;    // A mutex to get/set the gaze points

 public:
  /** Constructor
   *
   * Constructor for the visualizer
   *
   * @param parent Parent widget (container)
   */
  image_view(QWidget *parent = nullptr);

  /** Deconstructor
   *
   * Deconstructor (used for cleanup)
   */
  ~image_view();

  /** A function to set camera locations to look at
   *
   * An external setter for other classes to set the next points to render
   * for the gaze points in the image.
   *
   * @param new_Pts a list of points in the image to render rectangles
   */
  void set_gaze_pts(std::vector<xy_pt> &new_pts);

  /** A function to set what image to display next
   *
   * set_tensor sets the next image to display in the widget
   *
   * @param tensor The image to display
   * @param mode How to display it
   */
  void set_tensor(const arrow::Tensor &tensor, const visualization_mode mode);

  /** A function to draw boxes where the cameras will center themselves.
   *
   * draw_box is a public method that will add a rectangle at a
   * specific location. It's helpful because it can be called asynchronously.
   *
   * @param x top-x location of the box on the image
   * @param y top-y location of the box on the image
   * @param width width of the box on the image
   * @param height height of the box on the image
   */
  void draw_box(int x, int y, int width, int height);

  /** Clears the points to look at
   *
   * clear_overlay will, asynchronously, clear out the
   * rectangles so that other methods can set new rectangles
   * later on.
   */
  void clear_overlay();
};

class QtViewer : public fn_dag::dag_node<arrow::Tensor, int> {
 public:
  QtViewer();
  ~QtViewer();

  void start();
  std::unique_ptr<int> update(const arrow::Tensor *image);

 private:
  QFrame *m_frame;
  image_view *m_imagePanel;
};

}  // namespace vin
