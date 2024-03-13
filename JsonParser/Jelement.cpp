#include "Jelement.h"

#include <sstream>
#include <variant>

using namespace tinyjson;

void* Jelement::value() {
  switch (m_type) {
    case T_NULL:
      return std::get_if<str_t>(&m_value);
    case T_BOOL:
      return std::get_if<bool_t>(&m_value);
    case T_INT:
      return std::get_if<int_t>(&m_value);
    case T_DOUBLE:
      return std::get_if<double_t>(&m_value);
    case T_LIST:
      return std::get_if<list_t>(&m_value);
    case T_STR:
      return std::get_if<str_t>(&m_value);
    default:
      return nullptr;
  }
}

/*
 *@breif 辅助宏,用来转换value为特定类型并且取*
 */
#define GET_VALUE(type) *((type*)value)

string Jelement::to_string() {
  void* value = this->value();
  std::ostringstream os;
  switch (m_type) {
    case T_NULL:
      os << "null";
      break;
    case T_BOOL:
      if (GET_VALUE(bool_t))
        os << "true";
      else
        os << "false";
      break;
    case T_INT:
      os << GET_VALUE(int_t);
      break;
    case T_DOUBLE:
      os << GET_VALUE(double_t);
      break;
    case T_STR:
      os << '\"' << GET_VALUE(str_t) << '\"';
      break;
    case T_LIST: {
      list_t& list = GET_VALUE(list_t);
      os << '[';
      for (auto i = 0; i < list.size(); ++i) {
        if (i != list.size() - 1) {
          os << ((list[i].to_string()));
          os << ',';
        } else
          os << ((list[i].to_string()));
      }
      os << ']';
      break;
    }
    case T_DICT: {
      dict_t& dict = GET_VALUE(dict_t);
      os << '{';
      for (auto it = dict.begin(); it != dict.end(); ++it) {
        if (it != dict.begin()) os << ',';
        os << '\"' << it->first << "\":" << it->second.to_string();
      }
      os << '}';
      break;
    }
    default:
      return "";
  }
  return os.str();
}