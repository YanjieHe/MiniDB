#ifndef DBROW_HPP
#define DBROW_HPP
#include "Header.hpp"
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

  bool loaded;
  vector<Value> values;

  DBRow() : loaded{false} {}
  explicit DBRow(const vector<Value> &values) : loaded{true}, values{values} {}
};

json DBRowToJson(const DBRow &record);

#endif // DBROW_HPP