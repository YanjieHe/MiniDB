#include "Index.hpp"
#include "DBException.hpp"

using std::holds_alternative;

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