#include <catch2/catch.hpp>
#include <iomanip>
#include <iostream>

#include "BPlusTree.hpp"
#include "DBIndex.hpp"
#include "JsonSerializer.hpp"
#include "TestUtils.hpp"

using std::cout;
using std::endl;

const size_t B_PLUS_TREE_ORDER = 3;

TEST_CASE("Book Example B+ Tree Test", "[B+ Tree]") {
  auto columns = BookIndexColumn();
  auto buffer = CreateBookPage();
  DatabaseHeader dbHeader;
  dbHeader.nPages = 0;
  dbHeader.pageSize = PAGE_SIZE;

  SECTION("single index: book example") {
    string path = "output/book_b_plus_tree";
    BufferManager bufferManager(path, dbHeader);

    BPlusTree tree(B_PLUS_TREE_ORDER, bufferManager, PAGE_SIZE, columns);
    tree.Insert(DBIndex({StringKey("The Red and the Black")}), 10);
    tree.Insert(DBIndex({StringKey("The Million Pound Bank Note")}), 20);
    tree.Insert(DBIndex({StringKey("The Catcher in the Rye")}), 30);
    tree.Insert(DBIndex({StringKey("Pride and Prejudice")}), 40);

    REQUIRE(tree.Search(DBIndex({StringKey("The Red and the Black")})) ==
            std::make_optional<i64>(10));
    REQUIRE(tree.Search(DBIndex({StringKey("The Million Pound Bank Note")})) ==
            std::make_optional<i64>(20));
    REQUIRE(tree.Search(DBIndex({StringKey("The Catcher in the Rye")})) ==
            std::make_optional<i64>(30));
    REQUIRE(tree.Search(DBIndex({StringKey("Pride and Prejudice")})) ==
            std::make_optional<i64>(40));
  }
}

TEST_CASE("Integer Index B+ Tree Test", "[B+ Tree]") {
  vector<DBColumn> indexColumns{DBColumn(false, TypeTag::INTEGER, "index")};
  vector<DBColumn> dataColumns{DBColumn(false, TypeTag::INTEGER, "index"),
                               DBColumn(false, TypeTag::INTEGER, "value")};
  DatabaseHeader dbHeader;
  dbHeader.nPages = 0;
  dbHeader.pageSize = PAGE_SIZE;

  SECTION("single index: integer index example") {
    string path = "output/integer_b_plus_tree";
    BufferManager bufferManager(path, dbHeader);

    BPlusTree tree(10, bufferManager, PAGE_SIZE, indexColumns);
    vector<std::pair<int, int>> keyValuePairs;
    for (int i = 0; i < 100; i++) {
      int key = 56 + i;
      int value = (i + 1) * 10;
      tree.Insert(DBIndex({Int64Key(key)}), value);
      keyValuePairs.push_back({key, value});
    }

    PrintFormattedJson(
        JsonSerializer::DatabaseHeaderToJson(bufferManager.header));

    for (auto pair : keyValuePairs) {
      cout << "insert: { key : " << pair.first;
      cout << ", value : " << pair.second << " }" << endl;
    }

    DisplayBPlusTreePages(bufferManager, tree);

    bufferManager.UpdateDatabaseHeaderToDisk();

    OutputBPlusTreeGraphvizCode("output/integer_b_plus_tree/BPlusTree.dot",
                                bufferManager, tree);

    for (auto pair : keyValuePairs) {
      REQUIRE(tree.Search(DBIndex({Int64Key(pair.first)})) ==
              std::make_optional<i64>(pair.second));
    }
  }
}