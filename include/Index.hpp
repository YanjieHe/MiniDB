#ifndef INDEX_HPP
#define INDEX_HPP
#include "Buffer.hpp"
#include "BufferManager.hpp"
#include "DBColumn.hpp"
#include <memory>
#include <optional>

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

  Index() = default;
  explicit Index(vector<Key> keys) : keys{keys} {}
};

template <typename T> int GetComparisonIntResult(const T &x, const T &y);

int CompareIndexKey(const Index::Key &x, const Index::Key &y);
int CompareIndex(const Index &x, const Index &y);

template <typename T> int GetComparisonIntResult(const T &x, const T &y) {
  if (x < y) {
    return -1;
  } else if (x > y) {
    return 1;
  } else {
    return 0;
  }
}
#endif // INDEX_HPP