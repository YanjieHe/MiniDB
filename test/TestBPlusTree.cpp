#include <catch2/catch.hpp>
#include <iomanip>
#include <iostream>

#include "BPlusTree.hpp"
#include "DBIndex.hpp"
#include "JsonSerializer.hpp"
#include "TestUtils.hpp"

using std::cout;
using std::endl;

const size_t order = 3;

TEST_CASE("Test B+ Tree", "[B+ Tree]") {
  auto columns = BookIndexColumn();
  auto buffer = CreateBookPage();
  DatabaseHeader dbHeader;
  dbHeader.pageSize = PAGE_SIZE;

  SECTION("single index") {
    string path = "output/b_plus_tree";
    CreateEmptyDatabaseFile(path, dbHeader);
    BufferManager bufferManager(path, dbHeader);

    BPlusTree tree(order, bufferManager, PAGE_SIZE, columns);
    tree.Insert(DBIndex({StringKey("The Red and the Black")}), 10);
    tree.Insert(DBIndex({StringKey("The Million Pound Bank Note")}), 20);
    tree.Insert(DBIndex({StringKey("The Catcher in the Rye")}), 30);
    tree.Insert(DBIndex({StringKey("Pride and Prejudice")}), 40);

    // cout << "columns size = " << tree.sharedData.columns.size() << endl;

    for (int pageNumber = 0; pageNumber < 3; pageNumber++) {
      bufferManager.LoadBuffer(pageNumber, tree.sharedData.buffer);
      IndexPage indexPage(tree.sharedData.columns, tree.sharedData.buffer,
                          PAGE_SIZE);

      cout << "page " << pageNumber << endl;
      cout << std::setw(4)
           << JsonSerializer::BPlusTreePageToJson(indexPage,
                                                  tree.sharedData.buffer)
           << endl;
    }

    REQUIRE(tree.Search(DBIndex({StringKey("The Red and the Black")})) ==
            std::make_optional<i64>(10));
    REQUIRE(tree.Search(DBIndex({StringKey("The Million Pound Bank Note")})) ==
            std::make_optional<i64>(20));
    REQUIRE(tree.Search(DBIndex({StringKey("The Catcher in the Rye")})) ==
            std::make_optional<i64>(30));
    REQUIRE(tree.Search(DBIndex({StringKey("Pride and Prejudice")})) ==
            std::make_optional<i64>(40));

    if (auto dataPointer =
            tree.Search(DBIndex({StringKey("The Red and the Black")}))) {
      cout << "searched result: data pointer = " << dataPointer.value() << endl;
    } else {
      REQUIRE(false);
    }
  }
}