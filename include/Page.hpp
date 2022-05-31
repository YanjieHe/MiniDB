#ifndef MINIDB_PAGE_HPP
#define MINIDB_PAGE_HPP
#include "Buffer.hpp"

class Page {
 public:
  PageHeader header;
  vector<DBColumn> columns;

  Page(const vector<DBColumn> &columns, Buffer &buffer, size_t pageSize);
  Page(PageHeader pageHeader, const vector<DBColumn> &columns);

  /************ Row Operations ************/
  bool AddRow(Buffer &buffer, const DBRow &record);
  bool InsertRow(Buffer &buffer, const DBRow &record, size_t pos);
  bool DeleteRow(Buffer &buffer, size_t pos);
  DBRow GetRow(Buffer &buffer, u16 index) const;

  void UpdateHeader(Buffer &buffer) const;
  size_t NumOfRows() const { return header.numOfEntries; }
  bool IsLeaf() const { return header.pageType == PageType::B_PLUS_TREE_LEAF; }
};

#endif  // MINIDB_PAGE_HPP