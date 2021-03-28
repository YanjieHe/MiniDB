#include "Page.hpp"
#include <DBException.hpp>
#include <algorithm>
#include <iostream>

using std::cout;
using std::endl;
using std::holds_alternative;

Page::Page(const vector<DBColumn> &columns, Buffer &buffer) : columns{columns} {
  LoadHeader(buffer, header);
  for (const auto &recordInfo : header.recordInfoArray) {
    buffer.pos = recordInfo.location;
    records.push_back(buffer.ReadRecord(this->columns));
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

void Page::Write(Buffer &buffer) {
  buffer.WriteU16(header.numOfEntries);
  buffer.WriteU16(header.endOfFreeSpace);
  for (const auto &recordInfo : header.recordInfoArray) {
    buffer.WriteU16(recordInfo.location);
    buffer.WriteU16(recordInfo.size);
  }
  for (size_t i = 0; i < header.recordInfoArray.size(); i++) {
    const auto &recordInfo = header.recordInfoArray.at(i);
    buffer.pos = recordInfo.location;
    WriteRow(buffer, records.at(i));
  }
}

void Page::WriteRow(Buffer &buffer, const DBRow &record) {
  auto WriteValue = [this, &buffer, &record](int i) {
    switch (columns.at(i).type) {
    case TypeTag::INTEGER: {
      buffer.WriteI64(std::get<i64>(record.values.at(i)));
      break;
    }
    case TypeTag::REAL: {
      buffer.WriteF64(std::get<f64>(record.values.at(i)));
      break;
    }
    case TypeTag::TEXT: {
      auto text = std::get<string>(record.values.at(i));
      buffer.WriteU16(static_cast<u16>(text.size()));
      buffer.WriteText(text);
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
        buffer.WriteU8(1);
      } else {
        buffer.WriteU8(0);
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