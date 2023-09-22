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
typedef enum {
  RESIZE
} OP;
class qt_op : public fn_dag::module_transmit
{
public:
  qt_op(OP op, int32_t width, int32_t height);
  ~qt_op();

  virtual std::vector<std::string> const get_available_slots();
  DLTensor *update(const DLTensor *) override;
  
private:
  OP op_code;
  uint32_t m_width;
  uint32_t m_height;
};

