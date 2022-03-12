#include "TestUtils.hpp"
#include <iomanip>
#include <iostream>


using std::cout;
using std::endl;

void PrintFormattedJson(json jsonObj) {
  cout << std::setw(4) << jsonObj << endl;
}

DBRow::Value DBString(string s) { return DBRow::Value(s); }

DBRow::Value DBInt64(i64 i) { return DBRow::Value(i); }