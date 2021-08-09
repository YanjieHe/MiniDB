#ifndef MINIDB_PAGE_HPP
#define MINIDB_PAGE_HPP
#include "Buffer.hpp"

class Page {
public:
  PageHeader header;
  vector<DBColumn> columns;
  vector<DBRow> records;
  size_t pageSize;

  Page(const vector<DBColumn> &columns, Buffer &buffer, size_t pageSize);
  Page(const vector<DBColumn> &columns, const vector<DBRow> &records,
       size_t pageSize);
  void Write(Buffer &buffer);
  void WriteRow(Buffer &buffer, const DBRow &row);
  bool AddRow(const DBRow &row);
  DBRow GetRow(Buffer &buffer, u16 index);
  size_t NumOfRows() const { return header.numOfEntries; }
};

#endif // MINIDB_PAGE_HPP