/*
 * Author: zhengsize
 */

/*!
 * \file inline.cc
 */
#include <unordered_map>

#include <tvm/tensor.h>
#include <tvm/operation.h>
#include <tvm/ir_functor_ext.h>
#include <tvm/ir_pass.h>
#include <tvm/logging.h>

#include "utils.h"

namespace tvm {
namespace ir {

#define UNEXPECTED {LOG(FATAL) << "unexpected visit of " << op->GetTypeKey() << "."; throw; }


class IntrinsicMatch final : public ExprFunctor<bool(const Expr &, const Expr &)> {
 public:
  using ExprFunctor::VisitExpr;
 private:
  std::unordered_map<const Variable*, const Variable*> mapping;
 protected:
  using ExprFunctor::VisitExpr_;
  #define MATCH(T)                            \
    const T* another = expr.as<T>();          \
    if (another == nullptr) {                 \
      return false;                           \
    }                                         \
    if (another->type != op->type) {          \
      return false;                           \
    }
  
  bool VisitExpr_(const Variable* op, const Expr &expr) final {
    MATCH(Variable)
    if (mapping.find(op) != mapping.end()) {
      // already mapped
      return another == mapping[op];
    } else {
      // not mapped
      mapping[op] = another;
      return true;
    }
  }

  bool VisitExpr_(const Load* op, const Expr &expr) final {
    MATCH(Load)
    return VisitExpr(op->index, another->index) \
          && VisitExpr(op->predicate, another->predicate) \
          && VisitExpr(op->buffer_var, another->buffer_var);
  }

  bool VisitExpr_(const Let* op, const Expr &expr) final {
    MATCH(Let)
    return VisitExpr(op->var, another->var) \
          && VisitExpr(op->value, another->value) \
          && VisitExpr(op->body, another->body);
  }

  bool VisitExpr_(const Call* op, const Expr &expr) final {
    MATCH(Call)
    bool args = true;
    int num_args = (int)another->args.size();
    Array<Expr> op_args;
    // eliminate prologue zero dim
    bool first = true;
    for (auto e : op->args) {
      if (first) {
        if (e.as<IntImm>() != nullptr) {
          if (e.as<IntImm>()->value == 0) {
            continue;
          }
        }
        if (e.as<UIntImm>() != nullptr) {
          if (e.as<UIntImm>()->value == 0) {
            continue;
          }
        }
      }
      first = false;
      op_args.push_back(e);
    }
    if (num_args != (int)op_args.size()) {
      return false;
    }
    for (int i = 0; i < num_args; ++i) {
      args = args && VisitExpr(op_args[i], another->args[i]);
    }
    if (!args) {
      return false;
    }

    // do not check function ref
    return op->call_type == another->call_type \
          && op->value_index == another->value_index;
  }

  template <typename T>
  bool VisitBinary(const T* op, const Expr &expr) {
    MATCH(T)
    return VisitExpr(op->a, another->a) && VisitExpr(op->b, another->b);
  }

  bool VisitExpr_(const Add* op, const Expr &expr) final {
    return VisitBinary(op, expr);
  }

  bool VisitExpr_(const Sub* op, const Expr &expr) final {
    return VisitBinary(op, expr);
  }

  bool VisitExpr_(const Mul* op, const Expr &expr) final {
    return VisitBinary(op, expr);
  }

  bool VisitExpr_(const Div* op, const Expr &expr) final {
    return VisitBinary(op, expr);
  }

  bool VisitExpr_(const Mod* op, const Expr &expr) final {
    return VisitBinary(op, expr);
  }

  bool VisitExpr_(const FloorDiv* op, const Expr &expr) final {
    return VisitBinary(op, expr);
  }

  bool VisitExpr_(const FloorMod* op, const Expr &expr) final {
    return VisitBinary(op, expr);
  }

  bool VisitExpr_(const Min* op, const Expr &expr) final {
    return VisitBinary(op, expr);
  }

  bool VisitExpr_(const Max* op, const Expr &expr) final {
    return VisitBinary(op, expr);
  }

  bool VisitExpr_(const EQ* op, const Expr &expr) final {
    return VisitBinary(op, expr);
  }

  bool VisitExpr_(const NE* op, const Expr &expr) final {
    return VisitBinary(op, expr);
  }

  bool VisitExpr_(const LT* op, const Expr &expr) final {
    return VisitBinary(op, expr);
  }

  bool VisitExpr_(const LE* op, const Expr &expr) final {
    return VisitBinary(op, expr);
  }

  bool VisitExpr_(const GT* op, const Expr &expr) final {
    return VisitBinary(op, expr);
  }

  bool VisitExpr_(const GE* op, const Expr &expr) final {
    return VisitBinary(op, expr);
  }

  bool VisitExpr_(const And* op, const Expr &expr) final {
    return VisitBinary(op, expr);
  }

  bool VisitExpr_(const Or* op, const Expr &expr) final {
    return VisitBinary(op, expr);
  }

  bool VisitExpr_(const Reduce* op, const Expr &expr) final {
    MATCH(Reduce)
    int num_lhs = op->combiner->lhs.size();
    if (num_lhs != (int)another->combiner->lhs.size()) {
      return false;
    }
    for (int i = 0; i < num_lhs; ++i) {
      if (!VisitExpr(op->combiner->lhs[i], another->combiner->lhs[i])) {
        return false;
      }
    }

    int num_rhs = op->combiner->rhs.size();
    if (num_rhs != (int)another->combiner->rhs.size()) {
      return false;
    }
    for (int i = 0; i < num_rhs; ++i) {
      if (!VisitExpr(op->combiner->rhs[i], another->combiner->rhs[i])) {
        return false;
      }
    }

    int num_res = op->combiner->result.size();
    if (num_res != (int)another->combiner->result.size()) {
      return false;
    }
    for (int i = 0; i < num_res; ++i) {
      if (!VisitExpr(op->combiner->result[i], another->combiner->result[i])) {
        return false;
      }
    }

    int num_src = op->source.size();
    if (num_src != (int)another->source.size()) {
      return false;
    }
    for (int i = 0; i < num_src; ++i) {
      if (!VisitExpr(op->source[i], another->source[i])) {
        return false;
      }
    }
    // do not check axis
    return VisitExpr(op->condition, another->condition) \
          && op->value_index == another->value_index;
  }

  bool VisitExpr_(const Cast* op, const Expr &expr) final {
    MATCH(Cast)
    return VisitExpr(op->value, another->value);
  }

  bool VisitExpr_(const Not* op, const Expr &expr) final {
    MATCH(Not)
    return VisitExpr(op->a, another->a);
  }

  bool VisitExpr_(const Select* op, const Expr &expr) final {
    MATCH(Select)
    return VisitExpr(op->condition, another->condition) \
          && VisitExpr(op->true_value, another->true_value) \
          && VisitExpr(op->false_value, another->false_value);
  }

  bool VisitExpr_(const Ramp* op, const Expr &expr) final {
    MATCH(Ramp)
    return VisitExpr(op->base, another->base) \
          && VisitExpr(op->stride, another->stride) \
          && op->lanes == another->lanes;  
  }

  bool VisitExpr_(const Broadcast* op, const Expr &expr) final {
    MATCH(Broadcast)
    return VisitExpr(op->value, another->value) \
          && op->lanes == another->lanes;
  }

  bool VisitExpr_(const Shuffle* op, const Expr &expr) final {
    MATCH(Shuffle)
    int num_vec = op->vectors.size();
    if (num_vec != (int)another->vectors.size()) {
      return false;
    }
    for (int i = 0; i < num_vec; ++i) {
      if (!VisitExpr(op->vectors[i], another->vectors[i])) {
        return false;
      }
    }

    int num_ind = op->indices.size();
    if (num_ind != (int)another->indices.size()) {
      return false;
    }
    for (int i = 0; i < num_ind; ++i) {
      if (!VisitExpr(op->indices[i], another->indices[i])) {
        return false;
      }
    }

    return true;
  }

  bool VisitExpr_(const IntImm* op, const Expr &expr) final {
    MATCH(IntImm)
    return op->value == another->value;
  }

  bool VisitExpr_(const UIntImm* op, const Expr &expr) final {
    MATCH(UIntImm)
    return op->value == another->value;
  }

  bool VisitExpr_(const FloatImm* op, const Expr &expr) final {
    MATCH(FloatImm)
    return op->value == another->value;
  }

  bool VisitExpr_(const StringImm* op, const Expr &expr) final {
    MATCH(StringImm)
    return true;
  }
};


bool intrinsic_match(
  const Tensor &target,
  const Tensor &intrinsic,
  Array<Var> spatial,
  Array<Var> reduce) {
  const ComputeOpNode* target_op = target->op.as<ComputeOpNode>();
  const ComputeOpNode* intrin_op = intrinsic->op.as<ComputeOpNode>();
  CHECK(target_op != nullptr) << "Target tensor is not from a ComputeOp " << target << ".";
  CHECK(intrin_op != nullptr) << "Intrinsic tensor is not from a ComputeOp " << intrinsic << ".";

  Expr subexpr = target_op->body[target->value_index];
  Array<Var> reserved;
  for (auto v : spatial) {
    reserved.push_back(v);
  }
  for (auto v : reduce) {
    reserved.push_back(v);
  }
  subexpr = SubIndexExpr(subexpr, reserved);

  IntrinsicMatch im;
  return im.VisitExpr(subexpr, intrin_op->body[intrinsic->value_index]);
}

}  // namespace ir
}  // namespace tvm
