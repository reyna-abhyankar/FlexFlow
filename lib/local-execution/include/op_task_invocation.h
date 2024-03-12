#ifndef _FLEXFLOW_RUNTIME_OP_TASK_SPEC_H
#define _FLEXFLOW_RUNTIME_OP_TASK_SPEC_H

#include "accessor.h"
#include "op_arg_ref.h"
#include "op_task_signature.h"
#include "profiling.h"
#include "runtime_arg_ref.h"
#include "serialization.h"
#include "tasks.h"
#include "utils/bidict.h"
#include "utils/optional.h"
#include "utils/stack_map.h"
#include <variant>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>

namespace FlexFlow {

enum class IsTrainable { YES, NO };

struct OpTensorSpec {
  TensorRole role;
  OpSlotOptions slot_option;
  req<int> idx;
};
FF_VISITABLE_STRUCT(OpTensorSpec, role, slot_option, idx);

OpTensorSpec input_tensor(int);
OpTensorSpec output_tensor(int);
OpTensorSpec weight_tensor(int);

using OpArgSpec = variant<ConcreteArgSpec,
                          OpArgRefSpec,
                          RuntimeArgRefSpec>;

struct OpArgSpecTypeAccessor {
  std::type_index operator()(OpArgSpec &spec) {
    return std::visit(
        [](auto &&arg) -> std::type_index {
          return arg.get_type_tag().get_type_idx()
        },
        spec);
  }
};

struct OpTaskBinding {
  OpTaskBinding() = default;

  void bind(slot_id, OpTensorSpec const &);
  void bind_grad(slot_id, OpTensorSpec const &);

  template <typename T>
  void bind_device_specific_arg(slot_id name, T const &t) {
    NOT_IMPLEMENTED();
  }

  template <typename T>
  void bind_device_specific_arg(slot_id name, OpArgRef<T> const &t) {
    NOT_IMPLEMENTED();
  }

  template <typename T>
  void bind_arg(slot_id name, T const &t) {
    this->insert_arg_spec(name, ConcreteArgSpec::create(t));
  }

  template <typename T>
  void bind_arg(slot_id name, RuntimeArgRef<T> const &ref) {
    this->insert_arg_spec(name, RuntimeArgRefSpec::create(ref));
  }

  template <typename T>
  void bind_arg(slot_id name, OpArgRef<T> const &ref) {
    this->insert_arg_spec(name, OpArgRefSpec::create(ref));
  }

  void bind_args_from_fwd(OpTaskBinding const &fwd) {
    this->arg_bindings = fwd.get_arg_bindings();
  }

  void bind_tensors_from_fwd(OpTaskBinding const &fwd) {
    this->tensor_bindings = fwd.get_tensor_bindings();
  }

  std::unordered_map<std::pair<slot_id, IsGrad>, OpTensorSpec> const &
      get_tensor_bindings() const;
  std::unordered_map<slot_id, OpArgSpec> const &get_arg_bindings() const;

private:
  void insert_arg_spec(slot_id name, OpArgSpec const &arg_spec) {
    assert(!contains_key(this->arg_bindings, name));
    this->arg_bindings.insert({name, arg_spec});
  }

  std::unordered_map<slot_id, OpArgSpec> arg_bindings;
  std::unordered_map<std::pair<slot_id, IsGrad>, OpTensorSpec> tensor_bindings;
};
FF_VISITABLE_STRUCT_NONSTANDARD_CONSTRUCTION_EMPTY(OpTaskBinding);

struct OpTaskInvocation {
public:
  OpTaskInvocation() = delete;
  OpTaskInvocation(task_id_t const &task_id, OpTaskBinding const &binding)
      : task_id(task_id), binding(binding) {}

public:
  task_id_t task_id;
  OpTaskBinding binding;
};
FF_VISITABLE_STRUCT_NONSTANDARD_CONSTRUCTION(OpTaskInvocation,
                                             task_id,
                                             binding);

OpTaskSignature infer_bwd_signature(OpTaskSignature const &fwd);
OpTaskBinding infer_bwd_binding(OpTaskBinding const &fwd);

} // namespace FlexFlow

#endif
