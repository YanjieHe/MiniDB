#include "DBRow.hpp"
#include "DBException.hpp"

using std::holds_alternative;

json DBRowToJson(const DBRow &record) {
  json obj;
  for (auto value : record.values) {
    if (holds_alternative<monostate>(value)) {
      obj.push_back(nullptr);
    } else if (holds_alternative<i64>(value)) {
      obj.push_back(std::get<i64>(value));
    } else if (holds_alternative<f64>(value)) {
      obj.push_back(std::get<f64>(value));
    } else {
      obj.push_back(std::get<string>(value));
    }
  }
  return obj;
}