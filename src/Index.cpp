#include "Index.hpp"
#include "DBException.hpp"

using std::holds_alternative;

IndexPage::IndexPage(const vector<DBColumn> &keyColumns, Buffer &buffer)
    : keyColumns{keyColumns} {
  LoadHeader(buffer, header);
  for (size_t i = 0; i < header.recordInfoArray.size(); i++) {
    const auto &recordInfo = header.recordInfoArray.at(i);
    buffer.pos = recordInfo.location;
    if (i % 2 == 0) {
      // page pointer
      u16 bufferID = buffer.ReadU16();
      u16 posIndex = buffer.ReadU16();
      pagePointers.emplace_back(bufferID, posIndex);
    } else {
      // primary key
      auto record = buffer.ReadRecord(this->keyColumns);
      indexList.push_back(RecordToIndex(this->keyColumns, record));
    }
  }
}

int CompareIndexKey(const Index::Key &x, const Index::Key &y) {
  if (holds_alternative<i64>(x) && holds_alternative<i64>(y)) {
    if (std::get<i64>(x) < std::get<i64>(y)) {
      return -1;
    } else if (std::get<i64>(x) > std::get<i64>(y)) {
      return 1;
    } else {
      return 0;
    }
  } else if (holds_alternative<string>(x) && holds_alternative<string>(y)) {
    if (std::get<string>(x) < std::get<string>(y)) {
      return -1;
    } else if (std::get<string>(x) > std::get<string>(y)) {
      return 1;
    } else {
      return 0;
    }
  } else {
    throw DBException(
        "the two index keys for comparision have two different types");
  }
}

int CompareIndex(const Index &x, const Index &y) {
  if (x.keys.size() == y.keys.size()) {
    for (size_t i = 0; i < x.keys.size(); i++) {
      if (int result = CompareIndexKey(x.keys[i], y.keys[i]); result != 0) {
        return result;
      }
    }
    return 0;
  } else {
    throw DBException("two indices have different number of keys");
  }
}

Index RecordToIndex(const vector<DBColumn> &keyColumns, const DBRow &record) {
  vector<Index::Key> keys;
  for (size_t j = 0; j < keyColumns.size(); j++) {
    auto col = keyColumns.at(j);
    if (col.type == TypeTag::INTEGER) {
      keys.push_back(std::get<i64>(record.values.at(j)));
    } else if (col.type == TypeTag::TEXT) {
      keys.push_back(std::get<string>(record.values.at(j)));
    } else {
      throw DBException("the index column type is not supported");
    }
  }
  return Index(keys);
}