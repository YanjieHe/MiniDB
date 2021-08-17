#ifndef DBINDEX_HPP
#define DBINDEX_HPP
#include "Buffer.hpp"
#include "BufferManager.hpp"
#include "DBColumn.hpp"
#include <memory>
#include <optional>

using std::variant;

class DataPointer {
public:
  u16 bufferID;
  u16 posIndex;

  DataPointer() = default;

  DataPointer(u16 bufferID, u16 posIndex)
      : bufferID{bufferID}, posIndex{posIndex} {}
};

class DBIndex {
public:
  typedef variant<i64, string> Key;

  vector<Key> keys;

  DBIndex() = default;
  explicit DBIndex(vector<Key> keys) : keys{keys} {}
};

template <typename T> int GetComparisonIntResult(const T &x, const T &y);

int CompareIndexKey(const DBIndex::Key &x, const DBIndex::Key &y);
int CompareIndex(const DBIndex &x, const DBIndex &y);

template <typename T> int GetComparisonIntResult(const T &x, const T &y) {
  if (x < y) {
    return -1;
  } else if (x > y) {
    return 1;
  } else {
    return 0;
  }
}

#endif // DBINDEX_HPP