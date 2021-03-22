#ifndef INDEX_HPP
#define INDEX_HPP
#include "Block.hpp"
#include "DBColumn.hpp"

using std::variant;

class Index {
public:
  typedef variant<i64, string> Key;

  vector<Key> keys;
  u16 blockIndex;
  u16 posIndex;

  Index(vector<Key> keys, u16 blockIndex, u16 posIndex)
      : keys{keys}, blockIndex{blockIndex}, posIndex{posIndex} {}
};

/*
keys | position
*/

class IndexPage {
public:
  Header header;
  vector<DBColumn> columns;
  vector<Index> indexList;

  IndexPage(const vector<DBColumn> &columns, Block &block);
};

int CompareIndexKey(const Index::Key &x, const Index::Key &y);
int CompareIndex(const Index &x, const Index &y);
void Search(ifstream &stream, size_t pageSize, const vector<DBColumn> &columns,
            const vector<Index::Key> &keys);

#endif // INDEX_HPP