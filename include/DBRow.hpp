#ifndef DBROW_HPP
#define DBROW_HPP
#include <string>
#include <variant>

#include "Header.hpp"

using std::monostate;
using std::string;
using std::variant;

class Blob {
  // TO DO
};

class DBRow {
 public:
  typedef variant<monostate, i64, f64, string, Blob> Value;

  vector<Value> values;

  DBRow() = default;
  explicit DBRow(const vector<Value> &values) : values{values} {}
};

#endif  // DBROW_HPP