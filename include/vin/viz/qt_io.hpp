#pragma once

#include <mutex>
#include <queue>

#include <QObject>
#include <QMediaPlayer>
#include <QVideoSink>
#include <QThread>
#include <QBuffer>
#include <QtWidgets/QLabel>
#include "functional_dag/lib_utils.h"


#define TEST_LIB_EXPORT Q_DECL_EXPORT
// TEST_LIB_EXPORT  player_event_thread
class player_event_thread : public QThread, public fn_dag::module_source
{
  Q_OBJECT

public:
  player_event_thread(std::string uri);
  ~player_event_thread();

  void run() override;
  DLTensor *update() override;
  void videoFrameChanged(const QVideoFrame &frame);
  void positionChanged(qint64 position);

private:
    QMediaPlayer *m_player;
    QVideoSink   *m_surface_for_player;
    std::string m_specified_url;
    std::mutex   m_mutex;
    std::condition_variable   m_condition;
    // torch::Tensor output_tensor;
    std::queue<DLTensor *> m_frame_queue;
};

