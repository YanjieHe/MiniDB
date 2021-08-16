#include "Page.hpp"

Page::Page(const vector<DBColumn> &columns, Buffer &buffer, size_t pageSize)
    : header{EmptyTablePageHeader(pageSize)}, columns{columns} {
  PreserveBufferPos(buffer, [&]() { LoadHeader(buffer, header); });
}

bool Page::AddRow(Buffer &buffer, const DBRow &record) {
  u16 size = ComputeRowSize(record, columns);
  u16 remainingSpace =
      header.endOfFreeSpace -
      (header.numOfEntries * 2 * sizeof(u16) + 2 * sizeof(u16));
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
    buffer.Reset();
    SaveHeader(buffer, header);
  });
}