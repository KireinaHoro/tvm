/*
 * Author: zhengsize
 */

#ifndef TVM_MICRO_UTILS_H_
#define TVM_MICRO_UTILS_H_

#include <tvm/ir.h>
#include <tvm/expr.h>

namespace tvm {

namespace ir {

Expr SubIndexExpr(
      Expr expr,
      Array<Var> reserved_vars);

}  // namespace ir

}  // namespace tvm
#endif  // TVM_MICRO_UTILS_H_
