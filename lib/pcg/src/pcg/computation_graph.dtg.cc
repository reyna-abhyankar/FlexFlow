// THIS FILE WAS AUTO-GENERATED BY proj. DO NOT MODIFY IT!
// If you would like to modify this datatype, instead modify
// lib/pcg/include/pcg/computation_graph.struct.toml
/* proj-data
{
  "generated_from": "bf8996bea2e022265a372d692c2db8ed"
}
*/

#include "pcg/computation_graph.dtg.h"

#include "pcg/dataflow_graph/dataflow_graph.h"
#include "pcg/layer_attrs.dtg.h"
#include "pcg/tensor_attrs.dtg.h"

namespace FlexFlow {
ComputationGraph::ComputationGraph(
    ::FlexFlow::DataflowGraph<::FlexFlow::LayerAttrs,
                              ::FlexFlow::TensorAttrs> const &raw_graph)
    : raw_graph(raw_graph) {}
} // namespace FlexFlow
