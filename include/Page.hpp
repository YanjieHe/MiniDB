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
  void WriteRecord(Block &block, const DBRow &record);
  bool AddRecord(const DBRow &record);
};

class Index {
public:
  typedef variant<i64, string> Key;
  Key key;
  u16 blockIndex;
  u16 index;
};

class IndexPage {
public:
  Header header;
  vector<Index> indexList;
};
bool IndexKeyLessThan(const Index::Key &x, const Index::Key &y);

#endif // MINIDB_PAGE_HPP