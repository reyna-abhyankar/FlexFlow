#include "local_allocator.h"
#include "local_model_training_instance.h"
#include "local_training_backing.h"
#include "op-attrs/tensor_shape.h"
#include "pcg/computation_graph_builder.h"
#include "pcg/optimizer.h"
#include "pcg/tensor.h"

int const BATCH_ITERS = 100;
int const BATCH_SIZE = 64;
int const HIDDEN_SIZE = 4096;
int const OUTPUT_SIZE = 10;
int const TRAINING_EPOCHS = 20;
double const DUMMY_FP_VAL = DUMMY_FP_VAL;

using namespace FlexFlow;

LocalModelTrainingInstance init_model_training_instance() {
  // construct computation graph
  ComputationGraphBuilder builder;
  int const dims[] = {BATCH_SIZE, HIDDEN_SIZE};
  TensorShape input_shape(dims, DataType::FLOAT);
  Tensor input_tensor = builder.create_tensor(input_shape, false);
  tensor_guid_t input_tensor_guid = builder.computation_graph.get_tensor_guid_t(input_tensor);
  Tensor dense_1 = builder.dense(input_tensor, HIDDEN_SIZE, Activation::RELU);
  Tensor dense_2 = builder.dense(dense_1, OUTPUT_SIZE, Activation::RELU);
  Tensor softmax = builder.softmax(dense_2);

  // pre-allocate input tensor
  Allocator allocator = get_local_memory_allocator();
  GenericTensorAccessorW input_tensor_backing =
      allocator.allocate(input_tensor);
  std::unordered_map<tensor_guid_t, GenericTensorAccessorW>
      pre_allocated_tensors;
  pre_allocated_tensors.insert({input_tensor_guid, input_tensor_backing});

  // optimizer
  double lr = DUMMY_FP_VAL;
  double momentum = DUMMY_FP_VAL;
  bool nesterov = false;
  double weight_decay = DUMMY_FP_VAL;
  SGDOptimizer optimizer = {lr, momentum, nesterov, weight_decay};

  // arguments
  EnableProfiling enable_profiling = EnableProfiling::NO;
  tensor_guid_t logit_tensor = builder.computation_graph.get_tensor_guid_t(softmax);
  tensor_guid_t label_tensor = logit_tensor; // how to get the label tensor?
  LossFunction loss_fn = LossFunction::CATEGORICAL_CROSSENTROPY;
  OtherLossAttrs loss_attrs = {loss_fn};
  // std::vector<Metric> metrics = {Metric::ACCURACY,
  //                                Metric::CATEGORICAL_CROSSENTROPY};
  // MetricsAttrs metrics_attrs(loss_fn, metrics);

  return LocalModelTrainingInstance(builder.computation_graph,
                            allocator,
                            optimizer,
                            enable_profiling,
                            logit_tensor,
                            label_tensor,
                            loss_attrs,
                            pre_allocated_tensors);
}

int main() {
  // TOOD: metrics and update
  LocalModelTrainingInstance ff_model = init_model_training_instance();
  for (int epoch = 0; epoch < TRAINING_EPOCHS; epoch++) {
    // ff_model.reset_metrics();
    for (int iter = 0; iter < BATCH_ITERS; iter++) {
      ff_model.forward();
      ff_model.backward();
      // ff_model.update();
    }
  }
}
