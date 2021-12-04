#include "BufferManager.hpp"
#include "Page.hpp"
#include <catch2/catch.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;

const size_t PAGE_SIZE = 65536;

void SavePage();
void LoadPage();
vector<DBColumn> ExampleColumns();
vector<DBRow> ExampleRows();
Buffer CreateExamplePage();

vector<DBColumn> ExampleColumns() {
  return vector<DBColumn>{DBColumn(false, TypeTag::TEXT, "name"),
                          DBColumn(false, TypeTag::INTEGER, "age")};
}

vector<DBRow> ExampleRows() {
  return vector<DBRow>{
      DBRow({DBRow::Value(string("Foo")), DBRow::Value(i64(34))}),
      DBRow({DBRow::Value(string("Bar")), DBRow::Value(i64(87))})};
}

Buffer CreateExamplePage() {
  auto columns = ExampleColumns();
  vector<DBRow> records = ExampleRows();
  Buffer buffer(PAGE_SIZE);
  PageHeader pageHeader = EmptyTablePageHeader(PAGE_SIZE);
  PreserveBufferPos(buffer, [&]() { SaveHeader(buffer, pageHeader); });

  Page page(columns, buffer, PAGE_SIZE);

  for (auto record : records) {
    if (page.AddRow(buffer, record) == false) {
      cerr << "out of space" << endl;
    }
  }
  if (page.AddRow(buffer, DBRow({DBRow::Value(string("Foo Bar")),
                                 DBRow::Value(i64(99))})) == false) {
    cout << "out of space" << endl;
  }
  page.UpdateHeader(buffer);
  return buffer;
}

void PrintFormattedJson(json jsonObj) {
  cout << std::setw(4) << jsonObj << endl;
}

void LoadPage() {
  cout << __FUNCTION__ << endl;
  vector<DBColumn> columns{DBColumn(false, TypeTag::TEXT, "name"),
                           DBColumn(false, TypeTag::INTEGER, "age")};
  Buffer buffer(PAGE_SIZE);
  BufferManager bufferManager("output/data.bin");
  bufferManager.LoadBuffer(0, buffer);
  Page page(columns, buffer, PAGE_SIZE);
  for (const auto &col : page.columns) {
    PrintFormattedJson(DBColumnToJson(col));
  }
  cout << "page.NumOfRows() = " << page.NumOfRows() << endl;
  for (size_t i = 0; i < page.NumOfRows(); i++) {
    const auto &record = page.GetRow(buffer, i);
    PrintFormattedJson(DBRowToJson(record));
  }
}

void SavePage() {
  cout << __FUNCTION__ << endl;
  vector<DBColumn> columns{DBColumn(false, TypeTag::TEXT, "name"),
                           DBColumn(false, TypeTag::INTEGER, "age")};
  vector<DBRow> records{
      DBRow({DBRow::Value(string("Foo")), DBRow::Value(i64(34))}),
      DBRow({DBRow::Value(string("Bar")), DBRow::Value(i64(87))})};

  Buffer buffer(PAGE_SIZE);
  PageHeader pageHeader = EmptyTablePageHeader(PAGE_SIZE);
  PreserveBufferPos(buffer, [&]() { SaveHeader(buffer, pageHeader); });
  PrintFormattedJson(PageHeaderToJson(pageHeader));

  PreserveBufferPos(buffer, [&]() { LoadHeader(buffer, pageHeader); });
  PrintFormattedJson(PageHeaderToJson(pageHeader));

  Page page(columns, buffer, PAGE_SIZE);
  PrintFormattedJson(PageHeaderToJson(pageHeader));

  for (auto record : records) {
    if (page.AddRow(buffer, record) == false) {
      cerr << "out of space" << endl;
    }
  }
  if (page.AddRow(buffer, DBRow({DBRow::Value(string("Foo Bar")),
                                 DBRow::Value(i64(99))})) == false) {
    cout << "out of space" << endl;
  }
  cout << "page.NumOfRows() = " << page.NumOfRows() << endl;
  page.UpdateHeader(buffer);
  cout << "updated header: " << endl;
  PreserveBufferPos(buffer, [&]() { LoadHeader(buffer, pageHeader); });
  PrintFormattedJson(PageHeaderToJson(pageHeader));
  cout << endl;

  DatabaseHeader dbHeader;
  dbHeader.pageSize = PAGE_SIZE;
  CreateEmptyDatabaseFile("output/data.bin", dbHeader);
  BufferManager bufferManager("output/data.bin");
  bufferManager.SaveBuffer(0, buffer);
}

TEST_CASE("Test Page", "[Page]") {
  SECTION("Save & Load Page") {
    SavePage();
    LoadPage();
  }
  SECTION("Insert Row") {
    Buffer buffer = CreateExamplePage();
    auto columns = ExampleColumns();
    Page page(columns, buffer, PAGE_SIZE);
    DBRow record({DBRow::Value(string("Baz")), DBRow::Value(i64(17))});
    page.InsertRow(buffer, record, 1);

    DatabaseHeader dbHeader;
    dbHeader.pageSize = PAGE_SIZE;
    CreateEmptyDatabaseFile("output/data.bin", dbHeader);
    BufferManager bufferManager("output/data.bin");
    bufferManager.SaveBuffer(0, buffer);

    LoadPage();
  }
}