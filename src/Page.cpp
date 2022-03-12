#include "Page.hpp"

Page::Page(const vector<DBColumn> &columns, Buffer &buffer, size_t pageSize)
    : header{EmptyTablePageHeader(pageSize)}, columns{columns} {
  buffer.PreserveBufferPos([&]() { buffer.LoadHeader(header); });
}

Page::Page(PageHeader pageHeader, const vector<DBColumn> &columns)
    : header{pageHeader}, columns{columns} {}

bool Page::AddRow(Buffer &buffer, const DBRow &record) {
  u16 size = ComputeRowSize(record, columns);
  u16 remainingSpace = header.endOfFreeSpace - header.ByteSize();
  if (remainingSpace > size) {
    header.numOfEntries++;
    DBRowInfo dbRowInfo = DBRowInfo(header.endOfFreeSpace - size, size);
    header.recordInfoArray.push_back(dbRowInfo);
    header.endOfFreeSpace = header.endOfFreeSpace - size;
    buffer.PreserveBufferPos([&]() {
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

      // aggregate the total size of the block that needs to be moved
      size_t totalSize = 0;
      for (size_t i = pos; i < header.recordInfoArray.size(); i++) {
        totalSize = totalSize + header.recordInfoArray.at(i).size;
      }

      // adjust record information array
      header.recordInfoArray.insert(header.recordInfoArray.begin() + pos,
                                    dbRowInfo);
      for (size_t i = pos + 1; i < header.recordInfoArray.size(); i++) {
        header.recordInfoArray.at(i).location =
            header.recordInfoArray.at(i).location - size;
      }

      buffer.MoveBlock(header.endOfFreeSpace, totalSize,
                       header.endOfFreeSpace - size);
      header.endOfFreeSpace = header.endOfFreeSpace - size;
      buffer.PreserveBufferPos([&]() {
        buffer.pos = dbRowInfo.location;
        buffer.WriteRecord(columns, record);
      });
      return true;
    } else {
      return false;
    }
  }
}

bool Page::DeleteRow(Buffer &buffer, size_t pos) {
  if (header.numOfEntries == 0 || pos >= header.recordInfoArray.size()) {
    return false;
  } else {
    header.numOfEntries--;
    size_t size = header.recordInfoArray.at(pos).size;
    header.recordInfoArray.erase(header.recordInfoArray.begin() + pos);

    // aggregate the total size of the block that needs to be moved
    size_t totalSize = 0;
    for (size_t i = pos; i < header.recordInfoArray.size(); i++) {
      totalSize = totalSize + header.recordInfoArray.at(i).size;
    }

    // adjust record information array
    for (size_t i = pos; i < header.recordInfoArray.size(); i++) {
      header.recordInfoArray.at(i).location =
          header.recordInfoArray.at(i).location + size;
    }

    buffer.MoveBlock(header.endOfFreeSpace, totalSize,
                     header.endOfFreeSpace + size);
    header.endOfFreeSpace = header.endOfFreeSpace + size;
    return true;
  }
}

DBRow Page::GetRow(Buffer &buffer, u16 index) const {
  const auto &info = header.recordInfoArray.at(index);
  DBRow row;
  buffer.PreserveBufferPos([&]() {
    buffer.pos = info.location;
    row = buffer.ReadRecord(columns);
  });
  return row;
}

void Page::UpdateHeader(Buffer &buffer) const {
  buffer.PreserveBufferPos([&]() {
    buffer.ResetPosition();
    buffer.SaveHeader(header);
  });
}