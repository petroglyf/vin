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
 * Library interface for VIN
 *
 * @author: ndepalma@alum.mit.edu
 * @license: MIT License
 */
#include <functional_dag/libutils.h>

#include "vin/error_codes.h"

namespace vin {
class vin_dag_manager : public fn_dag::library {
 public:
  vin_dag_manager() = default;
  ~vin_dag_manager() = default;

  error_codes add_node(fn_dag::dag_manager<std::string> &_manager,
                       const fn_dag::node_spec *_node_spec);
};
}  // namespace vin