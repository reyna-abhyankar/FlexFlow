#include "local-execution/local_training_backing.h"
#include "utils/exception.h"

namespace FlexFlow {

LocalTrainingBacking::LocalTrainingBacking(
    Allocator const &allocator,
    ComputationGraph const &computation_graph,
    TensorBackingMap const &tensor_backing_mapping,
    RuntimeArgConfig const &runtime_arg_config)
    : allocator(allocator), computation_graph(computation_graph),
      local_slots_backing(tensor_backing_mapping, runtime_arg_config) {
  std::vector<operator_guid_t> layers = topological_ordering(computation_graph);
  for (operator_guid_t const &node : layers) {
    CompGraphOperatorAttrs attrs = get_layer_attrs(computation_graph, node);

    // register tasks
    std::vector<task_id_t> task_ids = get_task_ids(attrs);
    for (task_id_t task_id : task_ids) {
      this->task_registry.register_task(task_id, node, attrs);
    }

    // insert pre-allocated tensors
    this->local_slots_backing.input_tensor_slots.insert(
        {node, get_incoming_tensors(computation_graph, node)});
    this->local_slots_backing.output_tensor_slots.insert(
        {node, get_outgoing_tensors(computation_graph, node)});

    // allocate new tensors
    for (tensor_guid_t const &edge :
         get_outgoing_tensors(computation_graph, node)) {
      if (!this->local_slots_backing.is_tensor_allocated(edge)) {
        Tensor tensor = computation_graph.at(edge);
        GenericTensorAccessorW tensor_backing =
            this->allocator.allocate_tensor(tensor.get_shape());
        this->local_slots_backing.tensor_mapping.insert({edge, tensor_backing});
      }
    }
  }
}

DeviceSpecific<DeviceStates>
    LocalTrainingBacking::call_init_task_impl(task_id_t task_id,
                                              TaskArgumentAccessor const &acc) {
  TaskSignatureAndImpl task_sig_impl =
      this->task_registry.task_mapping.at(task_id);
  auto fn = std::get<std::function<DeviceSpecific<DeviceStates>(
      TaskArgumentAccessor const &)>>(task_sig_impl.impl_function);
  return fn(acc);
}

void LocalTrainingBacking::call_task_impl(task_id_t task_id,
                                          TaskArgumentAccessor acc) {
  TaskSignatureAndImpl task_sig_impl =
      this->task_registry.task_mapping.at(task_id);
  auto fn = std::get<
      std::function<std::optional<float>(TaskArgumentAccessor const &)>>(
      task_sig_impl.impl_function);
  fn(acc);
}

void LocalTrainingBacking::execute_init() {
  for (operator_guid_t const &operator_node :
       topological_ordering(this->computation_graph)) {
    CompGraphOperatorAttrs attrs =
        get_layer_attrs(this->computation_graph, operator_node);
    OpTaskInvocation invocation = init(attrs);
    TaskArgumentAccessor accessor =
        this->get_task_arg_accessor(invocation, operator_node);
    DeviceSpecific<DeviceStates> device_state =
        this->call_init_task_impl(invocation.task_id, accessor);
    this->local_slots_backing.add_per_device_op_state(operator_node,
                                                      device_state);
  }
}

void LocalTrainingBacking::execute_forward() {
  for (operator_guid_t operator_node :
       topological_ordering(this->computation_graph)) {
    auto attrs = get_layer_attrs(this->computation_graph, operator_node);
    OpTaskInvocation invocation = forward(attrs);
    TaskArgumentAccessor accessor =
        this->get_task_arg_accessor(invocation, operator_node);
    this->call_task_impl(invocation.task_id, accessor);
  }
}

void LocalTrainingBacking::execute_backward() {
  for (operator_guid_t operator_node :
       reverse_topological_ordering(computation_graph)) {
    auto attrs = get_layer_attrs(this->computation_graph, operator_node);
    OpTaskInvocation invocation = backward(attrs);
    TaskArgumentAccessor accessor =
        this->get_task_arg_accessor(invocation, operator_node);
    this->call_task_impl(invocation.task_id, accessor);
  }
}

void LocalTrainingBacking::execute_update() {
  NOT_IMPLEMENTED();
}

TaskArgumentAccessor LocalTrainingBacking::get_task_arg_accessor(
    OpTaskInvocation const &invocation, operator_guid_t const &op_guid) const {
  TensorSlotsBacking tensor_slots_backing =
      this->local_slots_backing.construct_tensor_slots_backing(
          invocation.binding, op_guid);
  ArgSlotsBacking arg_slots_backing =
      this->local_slots_backing.construct_arg_slots_backing(invocation.binding,
                                                            op_guid);

  return TaskArgumentAccessor::create<LocalTaskArgumentAccessor>(
      this->allocator, tensor_slots_backing, arg_slots_backing);
}

} // namespace FlexFlow