#include "Buffer.hpp"

#include <bit_converter/bit_converter.hpp>
#include <cstring>

#include "DBException.hpp"

using std::holds_alternative;

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

DBRow Buffer::ReadRecord(const vector<DBColumn> &columns) {
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

void Buffer::ReadRecordFieldValue(DBRow &record, const DBColumn &col) {
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
    default: {
      throw DBException("BLOB type is not supported yet");
    }
  }
}

void Buffer::WriteRecord(const vector<DBColumn> &columns, const DBRow &record) {
  auto WriteValue = [this, &columns, &record](int i) {
    switch (columns.at(i).type) {
      case TypeTag::INTEGER: {
        WriteI64(std::get<i64>(record.values.at(i)));
        break;
      }
      case TypeTag::REAL: {
        WriteF64(std::get<f64>(record.values.at(i)));
        break;
      }
      case TypeTag::TEXT: {
        auto text = std::get<string>(record.values.at(i));
        WriteU16(static_cast<u16>(text.size()));
        WriteText(text);
        break;
      }
      default: {
        throw DBException("BLOB type is not supported yet");
      }
    }
  };
  for (size_t i = 0; i < columns.size(); i++) {
    if (columns.at(i).nullable) {
      if (holds_alternative<monostate>(record.values.at(i))) {
        WriteU8(1);
      } else {
        WriteU8(0);
      }
      WriteValue(i);
    } else {
      WriteValue(i);
    }
  }
}

void Buffer::LoadHeader(PageHeader &header) {
  header.pageType = static_cast<PageType>(ReadU8());
  header.numOfEntries = ReadU16();
  header.endOfFreeSpace = ReadU16();
  header.recordInfoArray.reserve(header.numOfEntries);
  for (size_t i = 0; i < header.numOfEntries; i++) {
    u16 location = ReadU16();
    u16 size = ReadU16();
    header.recordInfoArray.emplace_back(location, size);
  }
}

void Buffer::SaveHeader(const PageHeader &header) {
  WriteU8(static_cast<u8>(header.pageType));
  WriteU16(header.numOfEntries);
  WriteU16(header.endOfFreeSpace);
  for (const auto &recordInfo : header.recordInfoArray) {
    WriteU16(recordInfo.location);
    WriteU16(recordInfo.size);
  }
}

void Buffer::MoveBlock(size_t srcStart, size_t size, size_t destStart) {
  vector<u8> temporaryBytes(size);
  std::memcpy(temporaryBytes.data(), bytes.data() + srcStart, size);
  std::memcpy(bytes.data() + destStart, temporaryBytes.data(), size);
}

void Buffer::PreserveBufferPos(std::function<void()> action) {
  auto currentPos = pos;
  action();
  pos = currentPos;
}

void Buffer::Clear() {
  pos = 0;
  for (auto &b : bytes) {
    b = 0;
  }
}