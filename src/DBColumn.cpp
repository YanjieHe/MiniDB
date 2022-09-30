#include "DBColumn.hpp"

#include "DBException.hpp"

using std::holds_alternative;

u16 ComputeRowSize(const DBRow &row, const vector<DBColumn> &columns) {
  size_t totalSize = 0;
  for (size_t i = 0; i < columns.size(); i++) {
    const auto &column = columns.at(i);
    const auto &value = row.values.at(i);
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
      default: {
        throw DBException("BLOB type is not supported");
      }
    }
  }
  return totalSize;
}