#include <iostream>
#include <fstream>
#include "Page.hpp"
#include <nlohmann/json.hpp>
#include <iomanip>

using namespace std;
using nlohmann::json;

const size_t PAGE_SIZE = 65536;

json RecordToJson(const Record &record) {
  json obj;
  for (auto value : record.values) {
    if (holds_alternative<monostate>(value)) {
      obj.push_back(nullptr);
    } else if (holds_alternative<i64>(value)) {
      obj.push_back(std::get<i64>(value));
    } else if (holds_alternative<f64>(value)) {
      obj.push_back(std::get<f64>(value));
    } else {
      obj.push_back(std::get<string>(value));
    }
  }
  return obj;
}

void SavePage();
void LoadPage();
void LoadPage() {
  vector<Column> columns{Column(false, TypeTag::TEXT, "name"),
                         Column(false, TypeTag::INTEGER, "age")};
  Block block(PAGE_SIZE);
  ifstream stream("data.bin", std::ios::binary);
  block.LoadFromFile(stream);
  Page page(columns, block);
  page.Write(block);
  for (const auto &record : page.records) {
    cout << std::setw(4) << RecordToJson(record) << endl;
  }
}
void SavePage() {
  vector<Column> columns{Column(false, TypeTag::TEXT, "name"),
                         Column(false, TypeTag::INTEGER, "age")};
  vector<Record> records{
      Record(vector<Record::Value>{Record::Value(string("Foo")),
                                   Record::Value(i64(34))}),
      Record(vector<Record::Value>{Record::Value(string("Bar")),
                                   Record::Value(i64(87))})};
  Page page(columns, records, PAGE_SIZE);
  page.AddRecord(Record(vector<Record::Value>{Record::Value(string("Foo Bar")),
                                              Record::Value(i64(99))}));
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