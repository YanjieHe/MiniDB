#include "Page.hpp"

u16 Record::ComputeSize(const vector<Column> &columns) {
  size_t totalSize = 0;
  for (size_t i = 0; i < columns.size(); i++) {
    switch (columns.at(i).type) {
    case TypeTag::INTEGER: {
      totalSize = totalSize + sizeof(i64);
      break;
    }
    default: { throw "not supported"; }
    }
  }
}
void MakeBlock(ofstream &stream, vector<byte> &bytes) {
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

u16 Block::ReadU16() {
  u16 b1 = bytes.at(pos);
  u16 b2 = bytes.at(pos + 1);
  pos = pos + 2;
  return (b1 << 8) + b2;
}

i64 Block::ReadI64() {
  i64 i = 0;
  for (int k = 0; k < static_cast<int>(sizeof(i64)); k++) {
    i = i + (Get() << (56 - k * 8));
  }
  return i;
}

void Block::WriteU16(u16 u) {
  Put(static_cast<byte>((u >> 8) & 0xFF));
  Put(static_cast<byte>(u & 0xFF));
}
void Block::WriteI64(i64 i) {
  for (int k = 0; k < static_cast<int>(sizeof(i64)); k++) {
    Put(static_cast<byte>((i >> (56 - k * 8)) & 0xFF));
  }
}

Record Block::ReadRecord(vector<Column> &columns) {
  Record record;
  record.values.reserve(columns.size());
  for (const auto &col : columns) {
    switch (col.type) {
    case TypeTag::INTEGER: {
      i64 i = ReadI64();
      record.values.emplace_back(i);
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
  }
  return record;
}

Page::Page(Block &block) {
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
    records.push_back(block.ReadRecord(columns));
  }
}
Page::Page(const vector<Column> columns, const vector<Record> &recordCollection,
           size_t pageSize)
    : columns{columns} {
  header.numOfEntries = recordCollection.size();
  header.recordInfoArray.reserve(header.numOfEntries);
  size_t curPos = pageSize;
  for (size_t i = 0; i < header.numOfEntries; i++) {
    u16 location = curPos - 0;
  }
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
  for (size_t i = 0; i < columns.size(); i++) {
    switch (columns.at(i).type) {
    case TypeTag::INTEGER: {
      block.WriteI64(std::get<i64>(record.values.at(i)));
      break;
    }
    case TypeTag::TEXT: {
      auto text = std::get<string>(record.values.at(i));
      block.WriteU16(static_cast<u16>(text.size()));
      for (char c : text) {
        block.Put(static_cast<byte>(c));
      }
      break;
    }
    default: { throw "not supported yet"; }
    }
  }
}