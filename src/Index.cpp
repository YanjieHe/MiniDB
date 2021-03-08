#include "Index.hpp"
#include "DBException.hpp"

using std::holds_alternative;

bool IndexKeyLessThan(const Index::Key &x, const Index::Key &y) {
  if (holds_alternative<i64>(x) && holds_alternative<i64>(y)) {
    return std::get<i64>(x) < std::get<i64>(y);
  } else if (holds_alternative<string>(x) && holds_alternative<string>(y)) {
    return std::get<string>(x) < std::get<string>(y);
  } else {
    throw DBException(
        "the two index keys for comparision have two different types");
  }
}

bool IndexLessThan(const Index &x, const Index &y) {
  // TO DO
  if (x.keys.size() == y.keys.size()) {
    for (size_t i = 0; i < x.keys.size(); i++) {
      if (IndexKeyLessThan(x.keys[i], y.keys[i])) {
        return true;
      }
    }
  }
}