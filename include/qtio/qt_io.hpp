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
 * QT movie support for VIN.
 *
 * @author: ndepalma@alum.mit.edu
 * @license: MIT License
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <arrow/api.h>
#pragma GCC diagnostic pop
#include <QBuffer>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QMediaPlayer>
#include <QObject>
#include <QThread>
#include <QVideoSink>
#include <QtWidgets/QLabel>
#include <functional_dag/dag_interface.hpp>
#include <mutex>
#include <queue>
enum qt_source { VIDEO, CAMERA };

class qt_video_player : public QThread,
                        public fn_dag::dag_source<arrow::Tensor> {
  Q_OBJECT

 public:
  /** Constructor
   *
   * Constructor to capture from a movie file.
   *
   * @param uri A file path or URI to the capture device or file
   * @param width The width output image
   * @param height The height output image
   */
  qt_video_player(std::string uri, int32_t width, int32_t height);

  /** Deconstructor
   *
   * Deconstructor (used for cleanup)
   */
  ~qt_video_player();

  /** Thread run method
   *
   * The run method is overridden and executed by the QThread.
   * The QThread is needed because the video thread must be managed
   * by Qt's thread pool. The thread begins at construction.
   */
  void run() override;

  /**  DAG manager's update function for a source class.
   *
   * The required update from the dag thread pool. This is called
   * repeatedly and a mutex is used to pump images from the Qt thread
   * into the dag thread pool.
   *
   * @return The qt player's QtImage converted to a DLTensor.
   */
  std::unique_ptr<arrow::Tensor> update() override;

  /** Callback for Qt to set the next frame
   *
   * Qt callback to supply frames to the Qt thread.
   *
   * @param frame The new frame from the player object
   */
  void frame_changed(const QVideoFrame &frame);

 private:
  qt_source m_source_type;

  // Camera related objects
  QCamera *m_camera;  // Camera object for Qt
  QMediaCaptureSession *m_capture;

  // Video related objects
  QMediaPlayer *m_player;       // Player object for Qt
  std::string m_specified_url;  // The specified URL of the video file

  QVideoSink *m_surface_for_player;     // A surface for Qt to draw to
  uint32_t m_width;                     // The output width of the image
  uint32_t m_height;                    // The output height of the image
  std::mutex m_mutex;                   // A mutex to get/set the atomic image
  std::condition_variable m_condition;  // A condition variable for the mutex
  std::queue<std::unique_ptr<arrow::Tensor>>
      m_frame_queue;  // A queue for the last few frames
};
