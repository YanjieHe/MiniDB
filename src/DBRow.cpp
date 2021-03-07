#include "DBRow.hpp"
#include "DBException.hpp"

using std::holds_alternative;

u16 DBRow::ComputeSize(const vector<DBColumn> &columns) const {
  size_t totalSize = 0;
  for (size_t i = 0; i < columns.size(); i++) {
    const auto &column = columns.at(i);
    const auto &value = values.at(i);
    switch (column.type) {
    case TypeTag::INTEGER: {
      if (column.nullable) {
        if (holds_alternative<monostate>(value)) {
          totalSize = totalSize + 1;
        } else {
          totalSize = totalSize + 1 + sizeof(i64);
        }
      } else {
        totalSize = totalSize + sizeof(i64);
      }
      break;
    }
    case TypeTag::REAL: {
      if (column.nullable) {
        if (holds_alternative<monostate>(value)) {
          totalSize = totalSize + 1;
        } else {
          totalSize = totalSize + 1 + sizeof(f64);
        }
      } else {
        totalSize = totalSize + sizeof(f64);
      }
      break;
    }
    case TypeTag::TEXT: {
      if (column.nullable) {
        if (holds_alternative<monostate>(value)) {
          totalSize = totalSize + 1;
        } else {
          const auto &s = std::get<string>(value);
          totalSize = totalSize + sizeof(u16) + s.size();
        }
      } else {
        const auto &s = std::get<string>(value);
        totalSize = totalSize + sizeof(u16) + s.size();
      }
      break;
    }
    default: { throw DBException("BLOB type is not supported"); }
    }
  }
  return totalSize;
}

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