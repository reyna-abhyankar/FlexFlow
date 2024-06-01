/* Copyright 2023 CMU, Facebook, LANL, MIT, NVIDIA, and Stanford (alphabetical)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "reduction.h"
#include "kernels/reduction_kernels.h"
#include "op-attrs/get_output_shapes.h"
#include "utils/exception.h"
#include "utils/hash-utils.h"

namespace FlexFlow {
// declare Legion names

using namespace FlexFlow::Kernels::Reduction;

enum Slots { INPUT, OUTPUT, ATTRS, PROFILING };

OpTaskInvocation forward(ReductionAttrs const &attrs) {
  OpTaskBinding binding;

  binding.bind_arg(PROFILING, profiling_settings());
  binding.bind_arg(ATTRS, attrs);
  binding.bind(INPUT, input_tensor(0));
  binding.bind(OUTPUT, output_tensor(0));

  return {REDUCTION_FWD_TASK_ID, binding};
}

OpTaskInvocation backward(ReductionAttrs const &attrs) {
  OpTaskBinding binding = infer_bwd_binding(forward(attrs).binding);

  return {REDUCTION_BWD_TASK_ID, binding};
}

static std::optional<float> forward_task_impl(TaskArgumentAccessor const &acc) {
  ProfilingSettings profiling_settings =
      acc.get_argument<ProfilingSettings>(PROFILING);

  auto input = acc.get_tensor<Permissions::RO>(INPUT);
  auto output = acc.get_tensor<Permissions::WO>(OUTPUT);
  auto attrs = acc.get_argument<ReductionAttrs>(ATTRS);

  size_t num_replicas = attrs.reduction_degree;

  return profile(forward_kernel,
                 profiling_settings,
                 "[Reduction] forward_time = {:.2lf}ms\n",
                 input,
                 output,
                 num_replicas);
}

static std::optional<float>
    backward_task_impl(TaskArgumentAccessor const &acc) {
  ProfilingSettings profiling = acc.get_argument<ProfilingSettings>(PROFILING);

  auto input_grad = acc.get_tensor_grad<Permissions::WO>(INPUT);
  auto output_grad = acc.get_tensor_grad<Permissions::RO>(OUTPUT);
  return profile(backward_kernel,
                 profiling,
                 "[Reduction] backward_time = {:.2lf}ms\n",
                 input_grad,
                 output_grad);
}

CostMetrics measure_operator_cost(SimEnvFactory const &sim_factory,
                                  ReductionAttrs const &attrs,
                                  InputParallelTensorDesc const &input,
                                  ProfilingSettings const &settings,
                                  MachineView const &machine_view) {
  ParallelTensorShape output_shape = get_output_shape(attrs, input.shape);

  auto env = sim_factory.new_environment();

  SimTaskBinding fwd_binding;
  fwd_binding.bind_arg(PROFILING, settings);
  fwd_binding.bind_arg(ATTRS, attrs);
  fwd_binding.bind(INPUT, input.shape);
  fwd_binding.bind(OUTPUT, output_shape);

  auto fwd_accessor = env.get_fwd_accessor(REDUCTION_FWD_TASK_ID, fwd_binding);

  SimTaskBinding bwd_binding = infer_bwd_binding(fwd_binding);
  auto bwd_accessor = env.get_bwd_accessor(REDUCTION_BWD_TASK_ID, bwd_binding);

  float forward_time = forward_task_impl(fwd_accessor).value();
  float backward_time = backward_task_impl(bwd_accessor).value();

  float sync_time = default_estimate_sync_time(env);

  return make_metrics(forward_time, backward_time, sync_time, env);
}

template <>
void register_task<REDUCTION_FWD_TASK_ID>() {
  OpTaskSignature fwd(OpTaskType::FWD);

  fwd.add_arg_slot<ProfilingSettings>(PROFILING);
  fwd.add_arg_slot<ReductionAttrs>(ATTRS);

  fwd.add_input_slot(INPUT);
  fwd.add_output_slot(OUTPUT);

  register_task(REDUCTION_FWD_TASK_ID, "Reduction Fwd", fwd, forward_task_impl);
}

// TODO: OpTaskSignature

// template <>
// void register_task<REDUCTION_BWD_TASK_ID>() {
//   OpTaskSignature bwd =
//       infer_bwd_signature(get_op_signature(REDUCTION_FWD_TASK_ID));

//   register_task(REDUCTION_BWD_TASK_ID, "Reduction Bwd", bwd,
//   backward_task_impl);
// }

}; // namespace FlexFlow
