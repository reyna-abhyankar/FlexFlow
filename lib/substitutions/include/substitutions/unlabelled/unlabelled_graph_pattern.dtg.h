// THIS FILE WAS AUTO-GENERATED BY proj. DO NOT MODIFY IT!
// If you would like to modify this datatype, instead modify
// lib/substitutions/include/substitutions/unlabelled/unlabelled_graph_pattern.struct.toml
/* proj-data
{
  "generated_from": "f494ed79eb1ba4010155e456b452157f"
}
*/

#ifndef _FLEXFLOW_LIB_SUBSTITUTIONS_INCLUDE_SUBSTITUTIONS_UNLABELLED_UNLABELLED_GRAPH_PATTERN_DTG_H
#define _FLEXFLOW_LIB_SUBSTITUTIONS_INCLUDE_SUBSTITUTIONS_UNLABELLED_UNLABELLED_GRAPH_PATTERN_DTG_H

#include "utils/graph.h"

namespace FlexFlow {
struct UnlabelledGraphPattern {
  UnlabelledGraphPattern() = delete;
  UnlabelledGraphPattern(::FlexFlow::OpenMultiDiGraphView const &raw_graph);

  ::FlexFlow::OpenMultiDiGraphView raw_graph;
};
} // namespace FlexFlow

#endif // _FLEXFLOW_LIB_SUBSTITUTIONS_INCLUDE_SUBSTITUTIONS_UNLABELLED_UNLABELLED_GRAPH_PATTERN_DTG_H
