#ifndef MINIDB_PAGE_HPP
#define MINIDB_PAGE_HPP
#include "Buffer.hpp"

class Page {
public:
  PageHeader header;
  vector<DBColumn> columns;

  Page(const vector<DBColumn> &columns, Buffer &buffer, size_t pageSize);
  bool AddRow(Buffer &buffer, const DBRow &row);
  DBRow GetRow(Buffer &buffer, u16 index);
  void UpdateHeader(Buffer &buffer) const;
  size_t NumOfRows() const { return header.numOfEntries; }
};

#endif // MINIDB_PAGE_HPP