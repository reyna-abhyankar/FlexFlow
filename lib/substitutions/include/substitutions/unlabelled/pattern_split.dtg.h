// THIS FILE WAS AUTO-GENERATED BY proj. DO NOT MODIFY IT!
// If you would like to modify this datatype, instead modify
// lib/substitutions/include/substitutions/unlabelled/pattern_split.struct.toml
/* proj-data
{
  "generated_from": "8604edb5bd1a546ffa94ef496888e46d"
}
*/

#ifndef _FLEXFLOW_LIB_SUBSTITUTIONS_INCLUDE_SUBSTITUTIONS_UNLABELLED_PATTERN_SPLIT_DTG_H
#define _FLEXFLOW_LIB_SUBSTITUTIONS_INCLUDE_SUBSTITUTIONS_UNLABELLED_PATTERN_SPLIT_DTG_H

#include "fmt/format.h"
#include "nlohmann/json.hpp"
#include "substitutions/unlabelled/pattern_node.dtg.h"
#include "utils/graph.h"
#include <ostream>
#include <tuple>
#include <unordered_set>

namespace FlexFlow {
struct PatternSplit {
  PatternSplit() = delete;
  PatternSplit(std::unordered_set<::FlexFlow::PatternNode> const &first,
               std::unordered_set<::FlexFlow::PatternNode> const &second);

  bool operator==(PatternSplit const &) const;
  bool operator!=(PatternSplit const &) const;
  std::unordered_set<::FlexFlow::PatternNode> first;
  std::unordered_set<::FlexFlow::PatternNode> second;
};
} // namespace FlexFlow

namespace nlohmann {
template <>
struct adl_serializer<FlexFlow::PatternSplit> {
  static FlexFlow::PatternSplit from_json(json const &);
  static void to_json(json &, FlexFlow::PatternSplit const &);
};
} // namespace nlohmann

namespace FlexFlow {
std::string format_as(PatternSplit const &);
std::ostream &operator<<(std::ostream &, PatternSplit const &);
} // namespace FlexFlow

#endif // _FLEXFLOW_LIB_SUBSTITUTIONS_INCLUDE_SUBSTITUTIONS_UNLABELLED_PATTERN_SPLIT_DTG_H