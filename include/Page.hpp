#ifndef MINIDB_PAGE_HPP
#define MINIDB_PAGE_HPP
#include "Buffer.hpp"

class Page {
public:
  PageHeader header;
  vector<DBColumn> columns;

  Page(const vector<DBColumn> &columns, Buffer &buffer, size_t pageSize);

  /************ Row Operations ************/
  bool AddRow(Buffer &buffer, const DBRow &record);
  bool InsertRow(Buffer &buffer, const DBRow &record, size_t pos);
  bool DeleteRow(Buffer& buffer, size_t pos);
  DBRow GetRow(Buffer &buffer, u16 index);

  void UpdateHeader(Buffer &buffer) const;
  size_t NumOfRows() const { return header.numOfEntries; }
};

#endif // MINIDB_PAGE_HPP