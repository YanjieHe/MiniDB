#ifndef INDEX_HPP
#define INDEX_HPP
#include "Buffer.hpp"
#include "DBColumn.hpp"

using std::variant;

class RecordPointer {
public:
  u16 bufferID;
  u16 posIndex;

  RecordPointer() = default;

  RecordPointer(u16 bufferID, u16 posIndex)
      : bufferID{bufferID}, posIndex{posIndex} {}
};

class Index {
public:
  typedef variant<i64, string> Key;

  vector<Key> keys;

  explicit Index(vector<Key> keys) : keys{keys} {}
};

class NonLeafPage {
public:
  Header header;
  vector<DBColumn> keyColumns;
  vector<Index> indexList;
  vector<u16> pagePointers;

  NonLeafPage(const vector<DBColumn> &keyColumns, Buffer &buffer);
};

class LeafPage {
public:
  Header header;
  vector<DBColumn> keyColumns;
  vector<Index> indexList;
  vector<RecordPointer> recordPointers;
  vector<u16> pagePointers;

  LeafPage(const vector<DBColumn> &keyColumns, Buffer &buffer);
};

int CompareIndexKey(const Index::Key &x, const Index::Key &y);
int CompareIndex(const Index &x, const Index &y);
Index RecordToIndex(const vector<DBColumn> &keyColumns, const DBRow &record);

#endif // INDEX_HPP