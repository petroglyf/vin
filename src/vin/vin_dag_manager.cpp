#include "vin/vin_dag_manager.hpp"

#include <functional_dag/libutils.h>
#include <glog/logging.h>

#include <QList>
#include <QTreeWidget>
#include <functional_dag/guid_impl.hpp>

#include "vin/error_codes.h"

namespace vin {
using namespace fn_dag;

error_codes vin_dag_manager::add_node(dag_manager<std::string> &_manager,
                                      const node_spec *_node_spec) {
  if (!_create_node(_manager, _node_spec)) {
    LOG(ERROR) << "Failed to create node " << _node_spec->name() << std::endl;
    return error_codes::PARENT_ABSENT;
  }

  return error_codes::NO_ERROR;
}

}  // namespace vin
