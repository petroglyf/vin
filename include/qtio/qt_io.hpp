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

class player_event_thread : public QThread, public fn_dag::module_source
{
  Q_OBJECT

public:
  player_event_thread(std::string uri, int32_t width, int32_t height);
  ~player_event_thread();

  void run() override;
  DLTensor *update() override;
  void videoFrameChanged(const QVideoFrame &frame);
  void positionChanged(qint64 position);

private:
    QMediaPlayer *m_player;
    QVideoSink   *m_surface_for_player;
    std::string m_specified_url;
    uint32_t m_width;
    uint32_t m_height;
    std::mutex   m_mutex;
    std::condition_variable   m_condition;
    // torch::Tensor output_tensor;
    std::queue<DLTensor *> m_frame_queue;
};
