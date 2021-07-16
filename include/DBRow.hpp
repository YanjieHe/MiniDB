#ifndef DBROW_HPP
#define DBROW_HPP
#include "Header.hpp"
#include <cmath>
#include <nlohmann/json.hpp>
#include <string>
#include <variant>

using nlohmann::json;
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

json DBRowToJson(const DBRow &record);

#endif // DBROW_HPP