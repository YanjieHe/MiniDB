#ifndef DBCOLUMN_HPP
#define DBCOLUMN_HPP
#include "DBRow.hpp"

enum class TypeTag { INTEGER, REAL, TEXT, BLOB };

class DBColumn {
 public:
  bool nullable;
  TypeTag type;
  string name;

  DBColumn() = default;
  DBColumn(bool nullable, TypeTag type, string name)
      : nullable{nullable}, type{type}, name{name} {}
};

u16 ComputeRowSize(const DBRow &row, const vector<DBColumn> &columns);
#endif  // DBCOLUMN_HPP