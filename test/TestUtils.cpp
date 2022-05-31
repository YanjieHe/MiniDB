#include "TestUtils.hpp"

#include <catch2/catch.hpp>
#include <iomanip>
#include <iostream>

#include "Page.hpp"

using std::cout;
using std::endl;

void PrintFormattedJson(json jsonObj) {
  cout << std::setw(4) << jsonObj << endl;
}

DBRow::Value DBString(string s) { return DBRow::Value(s); }

DBRow::Value DBInt64(i64 i) { return DBRow::Value(i); }

DBIndex::Key StringKey(string s) { return DBIndex::Key(s); }

DBIndex::Key Int64Key(i64 i) { return DBIndex::Key(i); }

vector<DBColumn> BookDataColumns() {
  return vector<DBColumn>{DBColumn(false, TypeTag::TEXT, "book"),
                          DBColumn(false, TypeTag::INTEGER, "published_year")};
}

vector<DBColumn> BookIndexColumn() {
  return vector<DBColumn>{DBColumn(false, TypeTag::TEXT, "book")};
}

vector<DBRow> BookRows() {
  return vector<DBRow>{
      DBRow({DBString("The Red and the Black"), DBInt64(1830)}),
      DBRow({DBString("The Million Pound Bank Note"), DBInt64(1893)})};
}

Buffer CreateBookPage() {
  auto columns = BookDataColumns();
  vector<DBRow> records = BookRows();
  Buffer buffer(PAGE_SIZE);
  PageHeader pageHeader = EmptyTablePageHeader(PAGE_SIZE);
  buffer.PreserveBufferPos([&]() { buffer.SaveHeader(pageHeader); });
  Page page(columns, buffer, PAGE_SIZE);
  for (auto record : records) {
    REQUIRE(page.AddRow(buffer, record));
  }
  page.UpdateHeader(buffer);
  return buffer;
}
