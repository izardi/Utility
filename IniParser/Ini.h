#pragma once

#include <map>
#include <sstream>
#include <string>
#include <string_view>
using std::map;
using std::string;
using std::string_view;
using std::stringstream;

class Value {
  using value_t = string;
  value_t m_value;

 public:
  Value() = default;

  Value(const char* value) : m_value(value) {}

  Value(string&& value) : m_value(std::move(value)) {}

  operator int();

  operator bool();

  operator string();

  operator double();

  template <class T>
  Value& operator=(T value) {
    stringstream ss;
    ss << value;
    ss >> m_value;
    return *this;
  }
};

class Ini {
  using Section = map<string, Value>;

  static string trim(string src);

  string m_filename;
  map<string, Section> m_inifile;

 public:
  Ini() = default;
  explicit Ini(string_view filename);

  bool Load(string_view filename);
  bool Save(string_view filename);
  void Show(FILE* fp = stdout);
  void Clear();

  Section& operator[](string_view name) { return m_inifile[name.data()]; }
};