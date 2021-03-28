#ifndef INDEX_HPP
#define INDEX_HPP
#include "Buffer.hpp"
#include "DBColumn.hpp"

using std::variant;

class PagePointer {
public:
  u16 bufferID;
  u16 posIndex;

  PagePointer() = default;

  PagePointer(u16 bufferID, u16 posIndex)
      : bufferID{bufferID}, posIndex{posIndex} {}
};

class Index {
public:
  typedef variant<i64, string> Key;

  vector<Key> keys;

  explicit Index(vector<Key> keys) : keys{keys} {}
};

class IndexPage {
public:
  Header header;
  vector<DBColumn> keyColumns;
  bool isLeafPage;
  vector<Index> indexList;
  vector<PagePointer> pagePointers;

  IndexPage(const vector<DBColumn> &keyColumns, Buffer &buffer);
};

int CompareIndexKey(const Index::Key &x, const Index::Key &y);
int CompareIndex(const Index &x, const Index &y);
Index RecordToIndex(const vector<DBColumn> &keyColumns, const DBRow &record);

#endif // INDEX_HPP