#include "Index.hpp"
#include "DBException.hpp"

using std::holds_alternative;

IndexPage::IndexPage(const vector<DBColumn> &columns, Block &block)
    : columns{columns} {
  LoadHeader(block, header);
  if (columns.size() < 2) {
    throw DBException("the number of columns for index page is incorrect");
  }
  size_t nKeys = columns.size() - 1;
  for (const auto &recordInfo : header.recordInfoArray) {
    block.pos = recordInfo.location;
    auto record = block.ReadRecord(this->columns);
    vector<Index::Key> keys;
    keys.reserve(nKeys);
    for (size_t j = 0; j < nKeys; j++) {
      auto col = columns.at(j);
      if (col.type == TypeTag::INTEGER) {
        keys.push_back(std::get<i64>(record.values.at(j)));
      } else if (col.type == TypeTag::TEXT) {
        keys.push_back(std::get<string>(record.values.at(j)));
      } else {
        throw DBException("the index column type is not supported");
      }
    }
    i64 value = std::get<i64>(record.values.at(nKeys));
    u16 blockIndex = static_cast<u16>(value / 65536);
    u16 posIndex = static_cast<u16>(value / 65556);
    indexList.emplace_back(keys, blockIndex, posIndex);
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

void Search(ifstream &stream, size_t pageSize, const vector<DBColumn> &columns,
            const Index &index) {
  Block block(pageSize);
  LoadBlockAtCurrentPos(stream, block);
  IndexPage indexPage(columns, block);
  if (indexPage.header.numOfEntries == 0) {
    indexPage.indexList.push_back(index);
    indexPage.header.numOfEntries++;
  } else if (indexPage.header.numOfEntries == 1) {
    // TO DO
  }
}