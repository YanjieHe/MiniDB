#ifndef MINIDB_PAGE_HPP
#define MINIDB_PAGE_HPP
#include "DBRow.hpp"
#include "Block.hpp"

class Page {
public:
  Header header;
  vector<DBColumn> columns;
  vector<DBRow> records;

  Page(const vector<DBColumn> &columns, Block &block);
  Page(const vector<DBColumn> columns, const vector<DBRow> &records,
       size_t pageSize);
  void Write(Block &block);
  void WriteRow(Block &block, const DBRow &row);
  bool AddRow(const DBRow &row);
};

#endif // MINIDB_PAGE_HPP