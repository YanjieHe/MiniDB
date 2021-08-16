#include "BufferManager.hpp"
#include "Page.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;

const size_t PAGE_SIZE = 65536;

void SavePage();
void LoadPage();

void LoadPage() {
  vector<DBColumn> columns{DBColumn(false, TypeTag::TEXT, "name"),
                           DBColumn(false, TypeTag::INTEGER, "age")};
  Buffer buffer(PAGE_SIZE);
  BufferManager bufferManager("output/data.bin");
  bufferManager.LoadBuffer(0, buffer);
  Page page(columns, buffer, PAGE_SIZE);
  for (const auto &col : page.columns) {
    cout << std::setw(4) << DBColumnToJson(col) << endl;
  }
  cout << "page.NumOfRows() = " << page.NumOfRows() << endl;
  for (size_t i = 0; i < page.NumOfRows(); i++) {
    const auto &record = page.GetRow(buffer, i);
    cout << std::setw(4) << DBRowToJson(record) << endl;
  }
}

void SavePage() {
  vector<DBColumn> columns{DBColumn(false, TypeTag::TEXT, "name"),
                           DBColumn(false, TypeTag::INTEGER, "age")};
  vector<DBRow> records{DBRow(vector<DBRow::Value>{DBRow::Value(string("Foo")),
                                                   DBRow::Value(i64(34))}),
                        DBRow(vector<DBRow::Value>{DBRow::Value(string("Bar")),
                                                   DBRow::Value(i64(87))})};
  Buffer buffer(PAGE_SIZE);
  PageHeader pageHeader = EmptyTablePageHeader(PAGE_SIZE);
  PreserveBufferPos(buffer, [&]() { SaveHeader(buffer, pageHeader); });
  cout << std::setw(4) << PageHeaderToJson(pageHeader) << endl;

  PreserveBufferPos(buffer, [&]() { LoadHeader(buffer, pageHeader); });
  cout << std::setw(4) << PageHeaderToJson(pageHeader) << endl;

  Page page(columns, buffer, PAGE_SIZE);
  cout << std::setw(4) << PageHeaderToJson(page.header) << endl;

  for (auto record : records) {
    if (page.AddRow(buffer, record) == false) {
      cout << "out of space" << endl;
    }
  }
  if (page.AddRow(buffer, DBRow(vector<DBRow::Value>{
                              DBRow::Value(string("Foo Bar")),
                              DBRow::Value(i64(99))})) == false) {
    cout << "out of space" << endl;
  }
  cout << "page.NumOfRows() = " << page.NumOfRows() << endl;
  page.UpdateHeader(buffer);
  DatabaseHeader dbHeader;
  dbHeader.pageSize = PAGE_SIZE;
  CreateEmptyDatabaseFile("output/data.bin", dbHeader);
  BufferManager bufferManager("output/data.bin");
  bufferManager.SaveBuffer(0, buffer);
}

int main() {
  SavePage();
  LoadPage();
  return 0;
}