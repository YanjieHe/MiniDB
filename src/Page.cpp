#include "Page.hpp"
#include <iostream>
#include <bit_converter/bit_converter.hpp>
#include <algorithm>

using std::cout;
using std::endl;
using std::holds_alternative;

Record::Record(const vector<Value> &values) : values{values} {}

u16 Record::ComputeSize(const vector<Column> &columns) const {
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
    default: { throw "not supported"; }
    }
  }
  return totalSize;
}

Column::Column(bool nullable, TypeTag type, string name)
    : nullable{nullable}, type{type}, name{name} {}

bool IndexKeyLessThan(const Index::Key &x, const Index::Key &y) {
  if (holds_alternative<i64>(x) && holds_alternative<i64>(y)) {
    return std::get<i64>(x) < std::get<i64>(y);
  } else if (holds_alternative<string>(x) && holds_alternative<string>(y)) {
    return std::get<string>(x) < std::get<string>(y);
  } else {
    throw "the two index keys for comparision have two different types";
  }
}

void MakeBlock(ofstream &stream, vector<u8> &bytes) {
  auto pos = stream.tellp();
  stream.write(reinterpret_cast<char *>(bytes.data()), bytes.size());
  stream.seekp(pos);
}

void LoadBlock(ifstream &stream, Block &block) {
  auto pos = stream.tellg();
  stream.read(reinterpret_cast<char *>(block.bytes.data()), block.bytes.size());
  stream.seekg(pos);
}

Block::Block(size_t pageSize) : bytes(pageSize), pos{0} {}

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

Record Block::ReadRecord(vector<Column> &columns) {
  Record record;
  auto ReadValue = [this, &record](const Column &col) {
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
    default: { throw "not supported yet"; }
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

Page::Page(const vector<Column> &columns, Block &block) : columns{columns} {
  header.numOfEntries = block.ReadU16();
  header.endOfFreeSpace = block.ReadU16();
  header.recordInfoArray.reserve(header.numOfEntries);
  for (size_t i = 0; i < header.numOfEntries; i++) {
    u16 location = block.ReadU16();
    u16 size = block.ReadU16();
    header.recordInfoArray.emplace_back(location, size);
  }
  for (const auto &recordInfo : header.recordInfoArray) {
    block.pos = recordInfo.location;
    records.push_back(block.ReadRecord(this->columns));
  }
}

Page::Page(const vector<Column> columns, const vector<Record> &records,
           size_t pageSize)
    : columns{columns}, records{records} {
  header.numOfEntries = records.size();
  header.recordInfoArray.reserve(header.numOfEntries);
  size_t curPos = pageSize;
  for (size_t i = 0; i < header.numOfEntries; i++) {
    u16 size = records.at(i).ComputeSize(columns);
    u16 location = curPos - size;
    curPos = location;
    header.recordInfoArray.emplace_back(location, size);
  }
  header.endOfFreeSpace = curPos;
}

void Page::Write(Block &block) {
  block.WriteU16(header.numOfEntries);
  block.WriteU16(header.endOfFreeSpace);
  for (const auto &recordInfo : header.recordInfoArray) {
    block.WriteU16(recordInfo.location);
    block.WriteU16(recordInfo.size);
  }
  for (size_t i = 0; i < header.recordInfoArray.size(); i++) {
    const auto &recordInfo = header.recordInfoArray.at(i);
    block.pos = recordInfo.location;
    WriteRecord(block, records.at(i));
  }
}

void Page::WriteRecord(Block &block, const Record &record) {
  auto WriteValue = [this, &block, &record](int i) {
    switch (columns.at(i).type) {
    case TypeTag::INTEGER: {
      block.WriteI64(std::get<i64>(record.values.at(i)));
      break;
    }
    case TypeTag::REAL: {
      block.WriteF64(std::get<f64>(record.values.at(i)));
      break;
    }
    case TypeTag::TEXT: {
      auto text = std::get<string>(record.values.at(i));
      block.WriteU16(static_cast<u16>(text.size()));
      block.WriteText(text);
      break;
    }
    default: { throw "not supported yet"; }
    }
  };
  for (size_t i = 0; i < columns.size(); i++) {
    if (columns.at(i).nullable) {
      if (holds_alternative<monostate>(record.values.at(i))) {
        block.WriteU8(1);
      } else {
        block.WriteU8(0);
      }
      WriteValue(i);
    } else {
      WriteValue(i);
    }
  }
}

bool Page::AddRecord(const Record &record) {
  u16 size = record.ComputeSize(columns);
  u16 remainingSpace = header.endOfFreeSpace -
                       header.numOfEntries * 2 * sizeof(u16) + 2 * sizeof(u16);
  if (remainingSpace > size) {
    header.numOfEntries++;
    header.recordInfoArray.push_back(
        RecordInfo(header.endOfFreeSpace - size, size));
    header.endOfFreeSpace = header.endOfFreeSpace - size;
    records.push_back(record);
    return true;
  } else {
    return false;
  }
}