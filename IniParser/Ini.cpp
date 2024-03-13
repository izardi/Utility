#include "Ini.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

Value::operator int() { return std::stoi(m_value); }

Value::operator bool() {
  if (m_value == "ture")
    return true;
  else if (m_value == "false")
    return false;
  throw std::logic_error("value type error in bool()");
}

Value::operator double() {
  double ret;
  std::stringstream ss;
  ss << m_value;
  ss >> ret;
  return ret;
}

Value::operator string() { return m_value; }

Ini::Ini(string_view filename) { Load(filename); }

bool Ini::Load(string_view filename) {
  m_inifile.clear();
  m_filename = filename;

  string name;
  string line;
  std::ifstream fin(filename.data());

  if (!fin) {
    fprintf(stderr, "ERROR load file %s\n", filename.data());
    return false;
  }

  while (std::getline(fin, line)) {
    line = trim(line);
    auto view = string_view(line);
    if (view.empty()) continue;

    if (view[0] == '[') {
      auto pos = view.find_first_of(']');

      if (pos != -1) {
        name = trim(string(view.substr(1, pos - 1)));
        m_inifile[name];
      }
    } else if (view[0] == '#') {
      continue;
    } else {
      auto pos = view.find_first_of('=');
      if (pos != -1) {
        string key = trim(string(view.substr(0, pos)));
        string value =
            trim(string(view.substr(pos + 1, view.size() - pos - 1)));
        auto it = m_inifile.find(name);
        if (it == m_inifile.end()) {
          fprintf(stderr, "parsing error: section=%s key=%s\n", name.c_str(),
                  key.c_str());
          return false;
        }
        m_inifile[name][key] = value;
      }
    }
  }
  return true;
}

bool Ini::Save(string_view filename) {
  FILE* fp;
  if ((fp = fopen(filename.data(), "w+")) == nullptr) {
    std::logic_error("Ini::Save() open file error");
  }
  Show(fp);
  return true;
}

void Ini::Show(FILE* fp) {
  for (auto&& [name, section] : m_inifile) {
    fprintf(fp, "[%s]\n", name.c_str());
    for (auto&& [k, v] : section) {
      fprintf(fp, "%s = %s\n", k.c_str(), string(v).c_str());
    }
  }
}

void Ini::Clear() { m_inifile.clear(); }

string Ini::trim(string src) {
  src.erase(src.begin(), std::find_if(src.begin(), src.end(),
                                      [](char ch) { return ch != ' '; }));

  src.erase(std::find_if(src.rbegin(), src.rend(),
                         [](char ch) { return !std::isspace(ch); })
                .base(),
            src.end());

  return std::move(src);
}