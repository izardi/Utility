#pragma once

#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace tinyjson {

using std::is_same_v;
using std::map;
using std::string;
using std::string_view;
using std::variant;
using std::vector;

enum TYPE { T_NULL, T_BOOL, T_INT, T_DOUBLE, T_STR, T_LIST, T_DICT };

class Jelement;

using null_t = string;
using int_t = int32_t;
using bool_t = bool;
using double_t = double;
using str_t = string;
using list_t = vector<Jelement>;
using dict_t = map<string, Jelement>;

#define IS_TYPE(typea, typeb) is_same_v<typea, typeb>

template <typename T>
constexpr bool isBasicType() {
  if constexpr (IS_TYPE(T, str_t) || IS_TYPE(T, bool_t) ||
                IS_TYPE(T, double_t) || IS_TYPE(T, int_t))
    return true;
  return false;
}

class Jelement {
  using valute_t = variant<bool_t, int_t, double_t, str_t, list_t, dict_t>;

  valute_t m_value;
  TYPE m_type;

  /*
   * @brief 获取&m_value
   */
  void* value();

 public:
  /*
   * @brief 默认构造函数构造T_NULL对象
   */
  Jelement() {
    m_type = T_NULL;
    m_value = "null";
  }

  Jelement(int_t val) { Int(val); }
  Jelement(bool_t val) { Bool(val); }
  Jelement(double_t val) { Double(val); }
  Jelement(str_t const& val) { Str(val); }
  Jelement(list_t val) { List(std::move(val)); }
  Jelement(dict_t val) { Dict(std::move(val)); }

  void Null() {
    m_type = T_NULL;
    m_value = "null";
  }

  void Int(int_t value) {
    m_value = value;
    m_type = T_INT;
  }

  void Bool(bool_t value) {
    m_value = value;
    m_type = T_BOOL;
  }

  void Double(double_t value) {
    m_value = value;
    m_type = T_DOUBLE;
  }

  void Str(string_view value) {
    m_value = string(value);
    m_type = T_STR;
  }

  void List(list_t value) {
    m_value = std::move(value);
    m_type = T_LIST;
  }

  void Dict(dict_t value) {
    m_value = std::move(value);
    m_type = T_DICT;
  }

#define THROW_GET_ERROR(errno) \
  throw std::logic_error("type error in get " #errno " value!")

  /*
   *@breif 获取m_value的引用
   */
  template <typename T>
  T& Value() {
    if constexpr (IS_TYPE(T, str_t)) {
      if (m_type != T_STR) THROW_GET_ERROR(string);
    } else if constexpr (IS_TYPE(T, bool_t)) {
      if (m_type != T_BOOL) THROW_GET_ERROR(BOOL);
    } else if constexpr (IS_TYPE(T, int_t)) {
      if (m_type != T_INT) THROW_GET_ERROR(INT);
    } else if constexpr (IS_TYPE(T, double_t)) {
      if (m_type != T_DOUBLE) THROW_GET_ERROR(DOUBLE);
    } else if constexpr (IS_TYPE(T, list_t)) {
      if (m_type != T_LIST) THROW_GET_ERROR(LIST);
    } else if constexpr (IS_TYPE(T, dict_t)) {
      if (m_type != T_DICT) THROW_GET_ERROR(DICT);
    }

    void* v = value();
    if (v == nullptr)
      throw std::logic_error("unknown type in JObject::Value()");
    return *((T*)v);
  }

  TYPE Type() { return m_type; }

  string to_string();

  void push_back(Jelement item) {
    if (m_type == T_LIST) {
      auto& list = Value<list_t>();
      list.push_back(std::move(item));
      return;
    }
    throw std::logic_error("not a list type! JObjcct::push_back()");
  }

  void pop_back() {
    if (m_type == T_LIST) {
      auto& list = Value<list_t>();
      list.pop_back();
    } else
      throw std::logic_error("not list type! JObjcct::pop_back()");
  }

  Jelement& operator[](string const& key) {
    if (m_type == T_DICT) {
      auto& dict = Value<dict_t>();
      return dict[key];
    }
    throw std::logic_error("not dict type! JObject::opertor[]()");
  }
};

}  // namespace tinyjson