#include "TestUtils.hpp"

#include <catch2/catch.hpp>
#include <iomanip>
#include <iostream>
#include <fstream>

#include "Page.hpp"
#include "JsonSerializer.hpp"

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

void DisplayBPlusTreePages(BufferManager &bufferManager, BPlusTree &tree) {
  Json json;

  for (int pageNumber = 0; pageNumber < bufferManager.header.nPages;
       pageNumber++) {
    bufferManager.LoadBuffer(pageNumber, tree.sharedData.buffer);
    IndexPage indexPage(tree.sharedData.columns, tree.sharedData.buffer,
                        PAGE_SIZE);
    Json pageJson =
        JsonSerializer::BPlusTreePageToJson(indexPage, tree.sharedData.buffer);
    pageJson["page"] = pageNumber;
    json[pageNumber] = pageJson;
  }

  PrintFormattedJson(json);
}

void OutputBPlusTreeGraphvizCode(string outputPath,
                                 BufferManager &bufferManager,
                                 BPlusTree &tree) {
  std::ofstream stream(outputPath);
  stream << "digraph G {" << endl;
  stream << "node [shape = record,height=.1];" << endl;
  for (int pageNumber = 0; pageNumber < bufferManager.header.nPages;
       pageNumber++) {
    bufferManager.LoadBuffer(pageNumber, tree.sharedData.buffer);
    IndexPage indexPage(tree.sharedData.columns, tree.sharedData.buffer,
                        PAGE_SIZE);
    Json pageJson =
        JsonSerializer::BPlusTreePageToJson(indexPage, tree.sharedData.buffer);

    int pointerCount = 0;
    stream << "node" << pageNumber << "[label = \"";
    for (auto item : pageJson["rows"]) {
      if (item.is_number_integer()) {
        stream << "<f" << pointerCount << "> ";
        pointerCount++;
      } else {
        stream << "|" << item["keys"] << "|";
      }
    }
    stream << "\"];" << endl;

    pointerCount = 0;
    for (auto item : pageJson["rows"]) {
      if (item.is_number_integer()) {
        int pointer = static_cast<int>(item);
        if (pointer >= 0 &&
            pageJson["header"]["pageType"] == "B_PLUS_TREE_NON_LEAF") {
          stream << "\"node" << pageNumber << "\":f" << pointerCount
                 << " -> \"node" << pointer << "\"" << endl;
        }
        pointerCount++;
      } else {
        /* pass */
      }
    }
  }

  stream << "}" << endl;
}