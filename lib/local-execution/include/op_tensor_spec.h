#ifndef _FLEXFLOW_RUNTIME_SRC_TASK_SPEC_OP_TENSOR_SPEC_REF_H
#define _FLEXFLOW_RUNTIME_SRC_TASK_SPEC_OP_TENSOR_SPEC_REF_H

#include "op_task_signature.h"

namespace FlexFlow {

struct OpTensorSpec {
  TensorRole role;
  OpSlotOptions slot_option;
  req<int> idx;
};
FF_VISITABLE_STRUCT(OpTensorSpec, role, slot_option, idx);

OpTensorSpec input_tensor(int);
OpTensorSpec output_tensor(int);
OpTensorSpec weight_tensor(int);

} // namespace FlexFlow

#endif
