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
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float_t;
using f64 = double_t;

using std::ofstream;
using std::ifstream;
using std::vector;
using std::streampos;
using std::string;
using std::variant;
using std::monostate;

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

enum class TypeTag { INTEGER, REAL, TEXT, BLOB };

class Column {
public:
  bool nullable;
  TypeTag type;
  string name;
  Column() = default;
  Column(bool nullable, TypeTag type, string name);
};

class Record {
public:
  typedef variant<monostate, i64, f64, string, Blob> Value;

  vector<Value> values;
  Record() = default;
  explicit Record(const vector<Value> &values);

  u16 ComputeSize(const vector<Column> &columns) const;
};

class Block {
public:
  vector<u8> bytes;
  size_t pos;

  explicit Block(size_t pageSize);
  u8 ReadU8();
  u16 ReadU16();
  i64 ReadI64();
  f64 ReadF64();
  void WriteU8(u8 u);
  void WriteU16(u16 u);
  void WriteI64(i64 i);
  void WriteF64(f64 f);
  void WriteText(const std::string s);
  Record ReadRecord(vector<Column> &columns);
  void SaveToFile(ofstream &stream);
  void LoadFromFile(ifstream &stream);
};

class Page {
public:
  Header header;
  vector<Column> columns;
  vector<Record> records;

  Page(const vector<Column> &columns, Block &block);
  Page(const vector<Column> columns, const vector<Record> &records,
       size_t pageSize);
  void Write(Block &block);
  void WriteRecord(Block &block, const Record &record);
  bool AddRecord(const Record &record);
};

class Index {
public:
  typedef variant<i64, string> Key;
  Key key;
  u16 blockIndex;
  u16 index;
};

class IndexPage {
public:
  Header header;
  vector<Index> indexList;
};
bool IndexKeyLessThan(const Index::Key &x, const Index::Key &y);
void MakeBlock(ofstream &stream, vector<u8> &bytes);
void LoadBlock(ifstream &stream, Block &block);

#endif // MINIDB_PAGE_HPP