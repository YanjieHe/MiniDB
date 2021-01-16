#include <iostream>
#include <fstream>
#include "Page.hpp"

using namespace std;

int main() {

  vector<Column> columns{Column(false, TypeTag::TEXT, "name"),
                         Column(false, TypeTag::INTEGER, "age")};
  vector<Record> records{
      Record(vector<Record::Value>{Record::Value(string("Foo")),
                                   Record::Value(i64(34))}),
      Record(vector<Record::Value>{Record::Value(string("Bar")),
                                   Record::Value(i64(87))})};
  const size_t PAGE_SIZE = 65536;
  Page page(columns, records, PAGE_SIZE);
  Block block(PAGE_SIZE);
  page.Write(block);
  ofstream stream("data.bin", std::ios::binary);
  block.SaveToFile(stream);
  block.WriteF64(3.4);
  return 0;
}