// THIS FILE WAS AUTO-GENERATED BY proj. DO NOT MODIFY IT!
// If you would like to modify this datatype, instead modify
// lib/pcg/include/pcg/initializers/uniform_initializer_attrs.struct.toml
/* proj-data
{
  "generated_from": "dd9cbe65dc4495b031aef40d353db928"
}
*/

#ifndef _FLEXFLOW_LIB_PCG_INCLUDE_PCG_INITIALIZERS_UNIFORM_INITIALIZER_ATTRS_DTG_H
#define _FLEXFLOW_LIB_PCG_INCLUDE_PCG_INITIALIZERS_UNIFORM_INITIALIZER_ATTRS_DTG_H

#include "fmt/format.h"
#include "nlohmann/json.hpp"
#include <functional>
#include <ostream>
#include <tuple>

namespace FlexFlow {
struct UniformInitializerAttrs {
  UniformInitializerAttrs() = delete;
  explicit UniformInitializerAttrs(int const &seed,
                                   float const &min_val,
                                   float const &max_val);

  bool operator==(UniformInitializerAttrs const &) const;
  bool operator!=(UniformInitializerAttrs const &) const;
  bool operator<(UniformInitializerAttrs const &) const;
  bool operator>(UniformInitializerAttrs const &) const;
  bool operator<=(UniformInitializerAttrs const &) const;
  bool operator>=(UniformInitializerAttrs const &) const;
  int seed;
  float min_val;
  float max_val;
};
} // namespace FlexFlow

namespace std {
template <>
struct hash<::FlexFlow::UniformInitializerAttrs> {
  size_t operator()(::FlexFlow::UniformInitializerAttrs const &) const;
};
} // namespace std

namespace nlohmann {
template <>
struct adl_serializer<::FlexFlow::UniformInitializerAttrs> {
  static ::FlexFlow::UniformInitializerAttrs from_json(json const &);
  static void to_json(json &, ::FlexFlow::UniformInitializerAttrs const &);
};
} // namespace nlohmann

namespace FlexFlow {
std::string format_as(UniformInitializerAttrs const &);
std::ostream &operator<<(std::ostream &, UniformInitializerAttrs const &);
} // namespace FlexFlow

#endif // _FLEXFLOW_LIB_PCG_INCLUDE_PCG_INITIALIZERS_UNIFORM_INITIALIZER_ATTRS_DTG_H
