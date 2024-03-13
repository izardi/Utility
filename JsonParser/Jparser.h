#pragma once

#include <string_view>

#include "Jelement.h"

namespace tinyjson {

class Parser {
  string m_str;
  size_t m_idx{};

 public:
  Parser() = default;

  static Jelement fromString(string_view src);

  template <typename T>
  static string toJson(T const &src) {
    if constexpr (IS_TYPE(T, int_t)) {
    }
  }

  void init(string_view src);
  void trim_right();
  void ignoreComment();
  bool isEscape(size_t pos);
  char get_next_token();

  Jelement parse();
  Jelement parse_null();
  Jelement parse_num();
  bool parse_bool();
  string parse_string();
  Jelement parse_list();
  Jelement parse_dict();
};
}  // namespace tinyjson