#include "Page.hpp"
#include <iostream>
#include <algorithm>
#include <DBException.hpp>

using std::cout;
using std::endl;
using std::holds_alternative;

bool IndexKeyLessThan(const Index::Key &x, const Index::Key &y) {
  if (holds_alternative<i64>(x) && holds_alternative<i64>(y)) {
    return std::get<i64>(x) < std::get<i64>(y);
  } else if (holds_alternative<string>(x) && holds_alternative<string>(y)) {
    return std::get<string>(x) < std::get<string>(y);
  } else {
    throw DBException(
        "the two index keys for comparision have two different types");
  }
}

Page::Page(const vector<DBColumn> &columns, Block &block) : columns{columns} {
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

Page::Page(const vector<DBColumn> columns, const vector<DBRow> &records,
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

void Page::WriteRecord(Block &block, const DBRow &record) {
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

bool Page::AddRecord(const DBRow &record) {
  u16 size = record.ComputeSize(columns);
  u16 remainingSpace = header.endOfFreeSpace -
                       header.numOfEntries * 2 * sizeof(u16) + 2 * sizeof(u16);
  if (remainingSpace > size) {
    header.numOfEntries++;
    header.recordInfoArray.push_back(
        DBRowInfo(header.endOfFreeSpace - size, size));
    header.endOfFreeSpace = header.endOfFreeSpace - size;
    records.push_back(record);
    return true;
  } else {
    return false;
  }
}