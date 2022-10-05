#include "DBIndex.hpp"

#include "DBException.hpp"

using std::holds_alternative;

size_t DBIndex::ComputeSize() const {
  size_t totalSize = 0;
  for (auto &key : keys) {
    if (holds_alternative<i64>(key)) {
      totalSize += sizeof(i64);
    } else if (holds_alternative<string>(key)) {
      totalSize += sizeof(u16) + std::get<string>(key).size();
    }
  }
  return totalSize;
}

bool operator<(const DBIndex &left, const DBIndex &right) {
  if (left.keys.size() == right.keys.size()) {
    for (size_t i = 0; i < left.keys.size(); i++) {
      if (holds_alternative<i64>(left.keys[i]) &&
          holds_alternative<i64>(right.keys[i])) {
        i64 leftValue = std::get<i64>(left.keys[i]);
        i64 rightValue = std::get<i64>(right.keys[i]);
        if (leftValue < rightValue) {
          return true;
        } else if (leftValue > rightValue) {
          return false;
        } else {
          /* continue comparing */
        }
      } else if (holds_alternative<string>(left.keys[i]) &&
                 holds_alternative<string>(right.keys[i])) {
        const string &leftValue = std::get<string>(left.keys[i]);
        const string &rightValue = std::get<string>(right.keys[i]);
        if (leftValue < rightValue) {
          return true;
        } else if (leftValue > rightValue) {
          return false;
        } else {
          /* continue comparing */
        }
      } else {
        throw DBException(
            "the two index keys for comparision have two different types");
      }
    }
    return false;
  } else {
    throw DBException(
        "the two index keys for comparision have two different types");
  }
}
bool operator>(const DBIndex &left, const DBIndex &right) {
  if (left.keys.size() == right.keys.size()) {
    for (size_t i = 0; i < left.keys.size(); i++) {
      if (holds_alternative<i64>(left.keys[i]) &&
          holds_alternative<i64>(right.keys[i])) {
        i64 leftValue = std::get<i64>(left.keys[i]);
        i64 rightValue = std::get<i64>(right.keys[i]);
        if (leftValue < rightValue) {
          return false;
        } else if (leftValue > rightValue) {
          return true;
        } else {
          /* continue comparing */
        }
      } else if (holds_alternative<string>(left.keys[i]) &&
                 holds_alternative<string>(right.keys[i])) {
        const string &leftValue = std::get<string>(left.keys[i]);
        const string &rightValue = std::get<string>(right.keys[i]);
        if (leftValue < rightValue) {
          return false;
        } else if (leftValue > rightValue) {
          return true;
        } else {
          /* continue comparing */
        }
      } else {
        throw DBException(
            "the two index keys for comparision have two different types");
      }
    }
    return false;
  } else {
    throw DBException(
        "the two index keys for comparision have two different types");
  }
}
bool operator==(const DBIndex &left, const DBIndex &right) {
  if (left.keys.size() == right.keys.size()) {
    for (size_t i = 0; i < left.keys.size(); i++) {
      if (holds_alternative<i64>(left.keys[i]) &&
          holds_alternative<i64>(right.keys[i])) {
        i64 leftValue = std::get<i64>(left.keys[i]);
        i64 rightValue = std::get<i64>(right.keys[i]);
        if (leftValue != rightValue) {
          return false;
        } else {
          /* continue comparing */
        }
      } else if (holds_alternative<string>(left.keys[i]) &&
                 holds_alternative<string>(right.keys[i])) {
        const string &leftValue = std::get<string>(left.keys[i]);
        const string &rightValue = std::get<string>(right.keys[i]);
        if (leftValue != rightValue) {
          return false;
        } else {
          /* continue comparing */
        }
      } else {
        throw DBException(
            "the two index keys for (==) comparision have two different types");
      }
    }
    return true;
  } else {
    throw DBException(
        "the two index keys for (==) comparision have two different types");
  }
}
bool operator!=(const DBIndex &left, const DBIndex &right) {
  if (left.keys.size() == right.keys.size()) {
    for (size_t i = 0; i < left.keys.size(); i++) {
      if (holds_alternative<i64>(left.keys[i]) &&
          holds_alternative<i64>(right.keys[i])) {
        i64 leftValue = std::get<i64>(left.keys[i]);
        i64 rightValue = std::get<i64>(right.keys[i]);
        if (leftValue != rightValue) {
          return true;
        } else {
          /* continue comparing */
        }
      } else if (holds_alternative<string>(left.keys[i]) &&
                 holds_alternative<string>(right.keys[i])) {
        const string &leftValue = std::get<string>(left.keys[i]);
        const string &rightValue = std::get<string>(right.keys[i]);
        if (leftValue != rightValue) {
          return true;
        } else {
          /* continue comparing */
        }
      } else {
        throw DBException(
            "the two index keys for (!=) comparision have two different types");
      }
    }
    return false;
  } else {
    throw DBException(
        "the two index keys for (!=) comparision have two different types");
  }
}

DBIndex ReadDBIndex(const vector<DBColumn> &columns, Buffer &buffer) {
  DBRow row = buffer.ReadRecord(columns);
  vector<DBIndex::Key> keys;
  keys.reserve(row.values.size());
  for (DBRow::Value item : row.values) {
    if (holds_alternative<i64>(item)) {
      keys.push_back(std::get<i64>(item));
    } else if (holds_alternative<string>(item)) {
      keys.push_back(std::get<string>(item));
    } else {
      throw DBException("index key can only be integer or string");
    }
  }
  return DBIndex(keys);
}

void WriteDBIndex(const DBIndex &index, Buffer &buffer) {
  for (const auto &key : index.keys) {
    if (holds_alternative<i64>(key)) {
      buffer.WriteI64(std::get<i64>(key));
    } else if (holds_alternative<string>(key)) {
      const string &text = std::get<string>(key);
      buffer.WriteU16(text.size());
      buffer.WriteText(text);
    } else {
      throw DBException("index key type error");
    }
  }
}