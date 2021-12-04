#include "Page.hpp"

Page::Page(const vector<DBColumn> &columns, Buffer &buffer, size_t pageSize)
    : header{EmptyTablePageHeader(pageSize)}, columns{columns} {
  PreserveBufferPos(buffer, [&]() { LoadHeader(buffer, header); });
}

bool Page::AddRow(Buffer &buffer, const DBRow &record) {
  u16 size = ComputeRowSize(record, columns);
  u16 remainingSpace = header.endOfFreeSpace - header.ByteSize();
  if (remainingSpace > size) {
    header.numOfEntries++;
    DBRowInfo dbRowInfo = DBRowInfo(header.endOfFreeSpace - size, size);
    header.recordInfoArray.push_back(dbRowInfo);
    header.endOfFreeSpace = header.endOfFreeSpace - size;
    PreserveBufferPos(buffer, [&]() {
      buffer.pos = dbRowInfo.location;
      buffer.WriteRecord(columns, record);
    });
    return true;
  } else {
    return false;
  }
}

bool Page::InsertRow(Buffer &buffer, const DBRow &record, size_t pos) {
  if (header.numOfEntries == 0 && pos == 0) {
    return AddRow(buffer, record);
  } else {
    u16 size = ComputeRowSize(record, columns);
    u16 remainingSpace = header.endOfFreeSpace - header.ByteSize();
    if (remainingSpace > size) {
      header.numOfEntries++;
      size_t location = header.recordInfoArray.at(pos).location;
      DBRowInfo dbRowInfo = DBRowInfo(location, size);
      size_t totalSize = 0;
      for (size_t i = pos; i < header.recordInfoArray.size(); i++) {
        totalSize = header.recordInfoArray.at(i).size;
      }
      header.recordInfoArray.insert(header.recordInfoArray.begin() + pos,
                                    dbRowInfo);
      buffer.MoveBlock(header.endOfFreeSpace, totalSize,
                       header.endOfFreeSpace - size);
      header.endOfFreeSpace = header.endOfFreeSpace - size;
      PreserveBufferPos(buffer, [&]() {
        buffer.pos = dbRowInfo.location;
        buffer.WriteRecord(columns, record);
      });
      return true;
    } else {
      return false;
    }
  }
}

DBRow Page::GetRow(Buffer &buffer, u16 index) {
  const auto &info = header.recordInfoArray.at(index);
  DBRow row;
  PreserveBufferPos(buffer, [&]() {
    buffer.pos = info.location;
    row = buffer.ReadRecord(columns);
  });
  return row;
}

void Page::UpdateHeader(Buffer &buffer) const {
  PreserveBufferPos(buffer, [&]() {
    buffer.ResetPosition();
    SaveHeader(buffer, header);
  });
}