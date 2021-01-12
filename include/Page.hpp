#ifndef MINIDB_PAGE_HPP
#define MINIDB_PAGE_HPP
#include <fstream>
#include <vector>
#include <cstdint>
#include <string>
#include <variant>
#include <cmath>

using i32 = int32_t;
using i64 = int64_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using byte = uint8_t;
using f32 = float_t;
using f64 = double_t;

using std::ofstream;
using std::ifstream;
using std::vector;
using std::streampos;
using std::string;
using std::variant;

class RecordInfo {
public:
  u16 location;
  u16 size;
  RecordInfo() = default;
  RecordInfo(u16 location, u16 size) : location{location}, size{size} {}
};

class Header {
public:
  u16 numOfEntries;
  u16 endOfFreeSpace;
  vector<RecordInfo> recordInfoArray;
};

class Blob {
  // TO DO
};

class Record {
public:
  typedef variant<i64, f64, string, Blob> Value;

  vector<Value> values;

  u16 ComputeSize(const vector<Column>& columns);
};

enum class TypeTag { INTEGER, REAL, TEXT, BLOB };

class Column {
public:
  TypeTag type;
  string name;
};

class Block {
public:
  vector<byte> bytes;
  size_t pos;

  explicit Block(size_t pageSize);
  u16 ReadU16();
  i64 ReadI64();
  void WriteU16(u16 u);
  void WriteI64(i64 i);
  Record ReadRecord(vector<Column> &columns);
  inline byte Get() {
    byte b = bytes.at(pos);
    pos++;
    return b;
  }
  inline void Put(byte b) {
    bytes.at(pos) = b;
    pos++;
  }
};

class Page {
public:
  Header header;
  vector<Column> columns;
  vector<Record> records;

  explicit Page(Block &block);
  Page(const vector<Column> columns, const vector<Record> &recordCollection, size_t pageSize);
  void Write(Block &block);
  void WriteRecord(Block &block, const Record &record);
};

void MakeBlock(ofstream &stream, vector<byte> &bytes);
void LoadBlock(ifstream &stream, Block &block);

#endif // MINIDB_PAGE_HPP