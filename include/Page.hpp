#ifndef MINIDB_PAGE_HPP
#define MINIDB_PAGE_HPP
#include "DBRow.hpp"
#include "Buffer.hpp"

class Page {
public:
  PageHeader header;
  vector<DBColumn> columns;
  vector<DBRow> records;

  Page(const vector<DBColumn> &columns, Buffer &buffer);
  Page(const vector<DBColumn> columns, const vector<DBRow> &records,
       size_t pageSize);
  void Write(Buffer &buffer);
  void WriteRow(Buffer &buffer, const DBRow &row);
  bool AddRow(const DBRow &row);
};

#endif // MINIDB_PAGE_HPP