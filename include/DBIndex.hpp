#ifndef DB_INDEX_HPP
#define DB_INDEX_HPP
#include <memory>
#include <optional>

#include "Buffer.hpp"
#include "BufferManager.hpp"
#include "DBColumn.hpp"
#include "DBException.hpp"

using std::holds_alternative;
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
  size_t ComputeSize() const;
};

bool operator<(const DBIndex &left, const DBIndex &right);
bool operator>(const DBIndex &left, const DBIndex &right);
bool operator==(const DBIndex &left, const DBIndex &right);
bool operator!=(const DBIndex &left, const DBIndex &right);

DBIndex ReadDBIndex(const vector<DBColumn> &columns, Buffer &buffer);
void WriteDBIndex(const DBIndex &index, Buffer &buffer);

#endif  // DB_INDEX_HPP