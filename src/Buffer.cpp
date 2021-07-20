#include "Buffer.hpp"
#include "DBException.hpp"
#include <bit_converter/bit_converter.hpp>

u8 Buffer::ReadU8() {
  u8 u = bytes.at(pos);
  pos++;
  return u;
}

u16 Buffer::ReadU16() {
  u16 u = bit_converter::bytes_to_u16(bytes.begin() + pos, true);
  pos = pos + sizeof(u16);
  return u;
}

i64 Buffer::ReadI64() {
  i64 i = bit_converter::bytes_to_i64(bytes.begin() + pos, true);
  pos = pos + sizeof(i64);
  return i;
}

f64 Buffer::ReadF64() {
  f64 f = bit_converter::bytes_to_f64(bytes.begin() + pos, true);
  pos = pos + sizeof(f64);
  return f;
}

void Buffer::WriteU8(u8 u) {
  bytes.at(pos) = u;
  pos++;
}

void Buffer::WriteU16(u16 u) {
  bit_converter::u16_to_bytes(u, true, bytes.begin() + pos);
  pos = pos + sizeof(u16);
}

void Buffer::WriteI64(i64 i) {
  bit_converter::i64_to_bytes(i, true, bytes.begin() + pos);
  pos = pos + sizeof(i64);
}

void Buffer::WriteF64(f64 f) {
  bit_converter::f64_to_bytes(f, true, bytes.begin() + pos);
  pos = pos + sizeof(f64);
}

void Buffer::WriteText(const string s) {
  std::transform(s.begin(), s.end(), bytes.begin() + pos,
                 [](char c) { return static_cast<u8>(c); });
  pos = pos + s.size();
}

DBRow Buffer::ReadRecord(vector<DBColumn> &columns) {
  DBRow record;
  record.values.reserve(columns.size());
  for (const auto &col : columns) {
    if (col.nullable) {
      if (ReadU8()) {
        record.values.emplace_back(monostate());
      } else {
        ReadRecordFieldValue(record, col);
      }
    } else {
      ReadRecordFieldValue(record, col);
    }
  }
  return record;
}

void Buffer::ReadRecordFieldValue(DBRow record, const DBColumn &col) {
  switch (col.type) {
  case TypeTag::INTEGER: {
    i64 i = ReadI64();
    record.values.emplace_back(i);
    break;
  }
  case TypeTag::REAL: {
    f64 f = ReadF64();
    record.values.emplace_back(f);
    break;
  }
  case TypeTag::TEXT: {
    u16 size = ReadU16();
    string s(bytes.begin() + pos, bytes.begin() + pos + size);
    pos = pos + size;
    record.values.emplace_back(s);
    break;
  }
  default: { throw DBException("BLOB type is not supported yet"); }
  }
}

void LoadHeader(Buffer &buffer, PageHeader &header) {
  header.pageType = static_cast<PageType>(buffer.ReadU8());
  header.numOfEntries = buffer.ReadU16();
  header.endOfFreeSpace = buffer.ReadU16();
  header.recordInfoArray.reserve(header.numOfEntries);
  for (size_t i = 0; i < header.numOfEntries; i++) {
    u16 location = buffer.ReadU16();
    u16 size = buffer.ReadU16();
    header.recordInfoArray.emplace_back(location, size);
  }
}

void SaveHeader(Buffer &buffer, const PageHeader &header) {
  buffer.WriteU8(static_cast<u8>(header.pageType));
  buffer.WriteU16(header.numOfEntries);
  buffer.WriteU16(header.endOfFreeSpace);
  for (const auto &recordInfo : header.recordInfoArray) {
    buffer.WriteU16(recordInfo.location);
    buffer.WriteU16(recordInfo.size);
  }
}