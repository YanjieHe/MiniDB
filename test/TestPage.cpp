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
  buffer.PreserveBufferPos([&]() { buffer.SaveHeader(pageHeader); });

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
  PrintFormattedJson(PageHeaderToJson(page.header));
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
  buffer.PreserveBufferPos([&]() { buffer.SaveHeader(pageHeader); });
  PrintFormattedJson(PageHeaderToJson(pageHeader));

  buffer.PreserveBufferPos([&]() { buffer.LoadHeader(pageHeader); });
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
  buffer.PreserveBufferPos([&]() { buffer.LoadHeader(pageHeader); });
  PrintFormattedJson(PageHeaderToJson(pageHeader));
  cout << endl;

  DatabaseHeader dbHeader;
  dbHeader.pageSize = PAGE_SIZE;
  CreateEmptyDatabaseFile("output/data.bin", dbHeader);
  BufferManager bufferManager("output/data.bin");
  bufferManager.SaveBuffer(0, buffer);
}

TEST_CASE("Test Page", "[Page]") {
  Buffer buffer = CreateExamplePage();
  auto columns = ExampleColumns();
  DatabaseHeader dbHeader;
  dbHeader.pageSize = PAGE_SIZE;

  SECTION("Save & Load Page") {
    SavePage();
    LoadPage();
  }
  SECTION("Insert a Row") {
    cout << "========== Insert a Row ==========" << endl;
    Page page(columns, buffer, PAGE_SIZE);
    cout << "page.NumOfRows() = " << page.NumOfRows() << endl;
    DBRow record({DBRow::Value(string("Baz")), DBRow::Value(i64(17))});
    page.InsertRow(buffer, record, 1);
    cout << "page.NumOfRows() = " << page.NumOfRows() << endl;
    page.UpdateHeader(buffer);

    CreateEmptyDatabaseFile("output/data.bin", dbHeader);
    BufferManager bufferManager("output/data.bin");
    bufferManager.SaveBuffer(0, buffer);

    LoadPage();
  }
  SECTION("Delete a Row") {
    cout << "========== Delete a Row ==========" << endl;
    Page page(columns, buffer, PAGE_SIZE);
    cout << "page.NumOfRows() = " << page.NumOfRows() << endl;
    page.DeleteRow(buffer, 1);
    cout << "page.NumOfRows() = " << page.NumOfRows() << endl;
    page.UpdateHeader(buffer);

    CreateEmptyDatabaseFile("output/data.bin", dbHeader);
    BufferManager bufferManager("output/data.bin");
    bufferManager.SaveBuffer(0, buffer);

    LoadPage();
  }
}