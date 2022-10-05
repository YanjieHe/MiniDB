#ifndef MINIDB_PAGE_HPP
#define MINIDB_PAGE_HPP
#include "Buffer.hpp"

/* The size of every page is 8KB. */
const size_t PAGE_SIZE = 8192;

class IPage {
 public:
  virtual const PageHeader &Header() const = 0;
  virtual const vector<DBColumn> &Columns() const = 0;
};

class Page: public IPage {
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

  const PageHeader &Header() const override;
  const vector<DBColumn> &Columns() const override;
};

#endif  // MINIDB_PAGE_HPP