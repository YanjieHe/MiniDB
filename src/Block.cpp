#include "Block.hpp"
#include <bit_converter/bit_converter.hpp>
#include "DBException.hpp"

void WriteBlock(ofstream &stream, Block &block) {
  auto pos = stream.tellp();
  stream.write(reinterpret_cast<char *>(block.bytes.data()),
               block.bytes.size());
  stream.seekp(pos);
}

void LoadBlock(ifstream &stream, Block &block) {
  auto pos = stream.tellg();
  stream.read(reinterpret_cast<char *>(block.bytes.data()), block.bytes.size());
  stream.seekg(pos);
}

u8 Block::ReadU8() {
  u8 u = bytes.at(pos);
  pos++;
  return u;
}

u16 Block::ReadU16() {
  u16 u = bit_converter::bytes_to_u16(bytes.begin() + pos, true);
  pos = pos + sizeof(u16);
  return u;
}

i64 Block::ReadI64() {
  i64 i = bit_converter::bytes_to_i64(bytes.begin() + pos, true);
  pos = pos + sizeof(i64);
  return i;
}

f64 Block::ReadF64() {
  f64 f = bit_converter::bytes_to_f64(bytes.begin() + pos, true);
  pos = pos + sizeof(f64);
  return f;
}

void Block::WriteU8(u8 u) {
  bytes.at(pos) = u;
  pos++;
}

void Block::WriteU16(u16 u) {
  bit_converter::u16_to_bytes(u, true, bytes.begin() + pos);
  pos = pos + sizeof(u16);
}

void Block::WriteI64(i64 i) {
  bit_converter::i64_to_bytes(i, true, bytes.begin() + pos);
  pos = pos + sizeof(i64);
}

void Block::WriteF64(f64 f) {
  bit_converter::f64_to_bytes(f, true, bytes.begin() + pos);
  pos = pos + sizeof(f64);
}
void Block::WriteText(const std::string s) {
  std::transform(s.begin(), s.end(), bytes.begin() + pos,
                 [](char c) { return static_cast<u8>(c); });
  pos = pos + s.size();
}

DBRow Block::ReadRecord(vector<DBColumn> &columns) {
  DBRow record;
  auto ReadValue = [this, &record](const DBColumn &col) {
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
  };
  record.values.reserve(columns.size());
  for (const auto &col : columns) {
    if (col.nullable) {
      if (ReadU8()) {
        record.values.emplace_back(monostate());
      } else {
        ReadValue(col);
      }
    } else {
      ReadValue(col);
    }
  }
  return record;
}

void Block::SaveToFile(ofstream &stream) {
  stream.write(reinterpret_cast<char *>(bytes.data()), bytes.size());
}

void Block::LoadFromFile(ifstream &stream) {
  stream.read(reinterpret_cast<char *>(bytes.data()), bytes.size());
}