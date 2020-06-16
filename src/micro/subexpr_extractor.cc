/*
 * Author: zhengsize
 */

/*!
 * \file inline.cc
 */
#include <unordered_set>

#include <tvm/ir_mutator.h>
#include <tvm/expr_operator.h>
#include <tvm/ir_pass.h>

#include "utils.h"


namespace tvm {
namespace ir {


// take a subexpr
// eliminate unused index and dimension
class SubIndexExprExtractor final : public IRMutator {
 public:
  using IRMutator::Mutate;
  SubIndexExprExtractor(std::unordered_set<const Variable*> reserved_vars) : reserved_vars_(reserved_vars) {}
 protected:
  using IRMutator::Mutate_;
  Expr Mutate_(const Variable* op, const Expr& e) final {
    if (reserved_vars_.find(op) != reserved_vars_.end()) {
      return e;
    } else {
      return make_zero(e.type());
    }
  }
 private:
  std::unordered_set<const Variable*> reserved_vars_;
};


Expr SubIndexExpr(
      Expr expr,
      Array<Var> reserved_vars) {
  std::unordered_set<const Variable*> reserved;
  for (auto v : reserved_vars) {
    reserved.insert(v.get());
  }
  SubIndexExprExtractor siee(reserved);
  return Simplify(siee.Mutate(expr));
}

}  // namespace ir
}  // namespace tvm
