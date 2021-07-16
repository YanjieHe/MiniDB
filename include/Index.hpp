#ifndef INDEX_HPP
#define INDEX_HPP
#include "Block.hpp"

using std::variant;

class Index {
public:
  typedef variant<i64, string> Key;
  vector<Key> keys;
  u16 blockIndex; // block index of the child item in the next level
  u16 index;      // index of the child item in the next level
};

class IndexPage {
public:
  Header header;
  vector<Index> indexList;
};

template <typename T> int GetComparisonIntResult(const T &x, const T &y) {
  if (x < y) {
    return -1;
  } else if (x > y) {
    return 1;
  } else {
    return 0;
  }
}

int CompareIndexKey(const Index::Key &x, const Index::Key &y);
int CompareIndex(const Index &x, const Index &y);

#endif // INDEX_HPP