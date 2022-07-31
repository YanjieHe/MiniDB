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
    tree.Insert(DBIndex({StringKey("The Red and the Black")}));
    tree.Insert(DBIndex({StringKey("The Million Pound Bank Note")}));
    tree.Insert(DBIndex({StringKey("The Catcher in the Rye")}));
    tree.Insert(DBIndex({StringKey("Pride and Prejudice")}));

    cout << "columns size = " << tree.sharedData.columns.size() << endl;

    {
      bufferManager.LoadBuffer(0, tree.sharedData.buffer);
      IndexPage indexPage(tree.sharedData.columns, tree.sharedData.buffer,
                          PAGE_SIZE);

      cout << "page 0" << endl;
      cout << std::setw(4)
           << JsonSerializer::BPlusTreePageToJson(indexPage,
                                                  tree.sharedData.buffer)
           << endl;
    }

    {
      bufferManager.LoadBuffer(1, tree.sharedData.buffer);
      IndexPage indexPage(tree.sharedData.columns, tree.sharedData.buffer,
                          PAGE_SIZE);
      cout << "page 1" << endl;
      cout << std::setw(4)
           << JsonSerializer::BPlusTreePageToJson(indexPage,
                                                  tree.sharedData.buffer)
           << endl;
    }

    // if (auto dataPointer =
    //         tree.Search(DBIndex({StringKey("The Red and the Black")}))) {
    // } else {
    //   REQUIRE(false);
    // }
  }
}