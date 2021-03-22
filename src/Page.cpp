#include "Page.hpp"
#include <DBException.hpp>
#include <algorithm>
#include <iostream>


using std::cout;
using std::endl;
using std::holds_alternative;

Page::Page(const vector<DBColumn> &columns, Block &block) : columns{columns} {
  LoadHeader(block, header);
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
    u16 size = ComputeRowSize(records.at(i), columns);
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
    WriteRow(block, records.at(i));
  }
}

void Page::WriteRow(Block &block, const DBRow &record) {
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
    default: {
      throw "not supported yet";
    }
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

bool Page::AddRow(const DBRow &record) {
  u16 size = ComputeRowSize(record, columns);
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