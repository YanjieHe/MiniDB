#include <iostream>
#include <fstream>
#include "Page.hpp"
#include <nlohmann/json.hpp>
#include <iomanip>

using namespace std;

const size_t PAGE_SIZE = 65536;

void SavePage();
void LoadPage();

void LoadPage() {
  vector<DBColumn> columns{DBColumn(false, TypeTag::TEXT, "name"),
                           DBColumn(false, TypeTag::INTEGER, "age")};
  Block block(PAGE_SIZE);
  ifstream stream("data.bin", std::ios::binary);
  block.LoadFromFile(stream);
  Page page(columns, block);
  page.Write(block);
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
  page.AddRecord(DBRow(vector<DBRow::Value>{DBRow::Value(string("Foo Bar")),
                                            DBRow::Value(i64(99))}));
  Block block(PAGE_SIZE);
  page.Write(block);
  ofstream stream("data.bin", std::ios::binary);
  block.SaveToFile(stream);
}

int main() {
  SavePage();
  LoadPage();
  return 0;
}