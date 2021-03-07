#ifndef DBROW_HPP
#define DBROW_HPP
#include <cstdint>
#include <string>
#include <cmath>
#include <vector>
#include <variant>
#include <nlohmann/json.hpp>

using i32 = int32_t;
using i64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float_t;
using f64 = double_t;

using std::vector;
using std::string;
using std::variant;
using std::monostate;
using nlohmann::json;

class DBRowInfo {
public:
  u16 location;
  u16 size;
  DBRowInfo() = default;
  DBRowInfo(u16 location, u16 size) : location{location}, size{size} {}
};

class Header {
public:
  u16 numOfEntries;
  u16 endOfFreeSpace;
  vector<DBRowInfo> recordInfoArray;
};

class Blob {
  // TO DO
};

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

class DBRow {
public:
  typedef variant<monostate, i64, f64, string, Blob> Value;

  vector<Value> values;

  DBRow() = default;
  explicit DBRow(const vector<Value> &values) : values{values} {}

  u16 ComputeSize(const vector<DBColumn> &columns) const;
};

json DBRowToJson(const DBRow &record);

#endif // DBROW_HPP