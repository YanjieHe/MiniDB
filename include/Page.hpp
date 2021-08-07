#ifndef MINIDB_PAGE_HPP
#define MINIDB_PAGE_HPP
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
  const DBRow &GetRow(Buffer &buffer, u16 index);
  void ExtendRecords(u16 index);
  size_t NumOfRows() const { return header.numOfEntries; }

private:
  const DBRow &GetRowInternal(Buffer &buffer, u16 index);
};

#endif // MINIDB_PAGE_HPP