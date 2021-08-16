#ifndef BUFFER_HPP
#define BUFFER_HPP
#include "DBColumn.hpp"
#include "DBRow.hpp"
#include <functional>

class Buffer {
public:
  vector<u8> bytes;
  size_t pos;

  explicit Buffer(size_t pageSize) : bytes(pageSize), pos{0} {}
  u8 ReadU8();
  u16 ReadU16();
  i64 ReadI64();
  f64 ReadF64();
  void WriteU8(u8 u);
  void WriteU16(u16 u);
  void WriteI64(i64 i);
  void WriteF64(f64 f);
  void WriteText(const string s);
  DBRow ReadRecord(const vector<DBColumn> &columns);
  void ReadRecordFieldValue(DBRow &record, const DBColumn &col);
  void WriteRecord(const vector<DBColumn> &columns, const DBRow &record);
  void Reset() { pos = 0; }
};

void LoadHeader(Buffer &buffer, PageHeader &header);
void SaveHeader(Buffer &buffer, const PageHeader &header);
void PreserveBufferPos(Buffer &buffer, std::function<void()> action);

#endif // BUFFER_HPP