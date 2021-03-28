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
  BufferManager bufferManager("data.bin");
  bufferManager.LoadBuffer(0, PAGE_SIZE, buffer);
  Page page(columns, buffer);
  page.Write(buffer);
  for (const auto &col : page.columns) {
    cout << std::setw(4) << DBColumnToJson(col) << endl;
  }
  for (const auto &record : page.records) {
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
  Page page(columns, records, PAGE_SIZE);
  page.AddRow(DBRow(vector<DBRow::Value>{DBRow::Value(string("Foo Bar")),
                                         DBRow::Value(i64(99))}));
  Buffer buffer(PAGE_SIZE);
  page.Write(buffer);
  BufferManager bufferManager("data.bin");
  bufferManager.SaveBuffer(0, PAGE_SIZE, buffer);
}

int main() {
  SavePage();
  LoadPage();
  return 0;
}