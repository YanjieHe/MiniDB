#include <catch2/catch.hpp>

#include "BPlusTree.hpp"
#include "DBIndex.hpp"
#include "TestUtils.hpp"

const size_t order = 3;

TEST_CASE("Test B+ Tree", "[B+ Tree]") {
  // auto columns = BookDataColumns();
  // auto buffer = CreateBookPage();
  // DatabaseHeader dbHeader;
  // dbHeader.pageSize = PAGE_SIZE;

  SECTION("single index") {
    // string path = "output/b_plus_tree.bin";
    // CreateEmptyDatabaseFile(path, dbHeader);
    // BufferManager bufferManager(path);
    // bufferManager.SaveBuffer(bufferManager.AllocatePage(), buffer);
    // {
    //   BPlusTree tree(order, bufferManager, PAGE_SIZE, columns);
    //   tree.Insert(DBIndex({StringKey("The Red and the Black")}));
    //   tree.Insert(DBIndex({StringKey("The Million Pound Bank Note")}));
    //   tree.Insert(DBIndex({StringKey("The Catcher in the Rye")}));
    //   tree.Insert(DBIndex({StringKey("Pride and Prejudice")}));
    // }
    // if (auto dataPointer =
    //         tree.Search(DBIndex({StringKey("The Red and the Black")}))) {
    //   REQUIRE(dataPointer->bufferID == 0);
    //   REQUIRE(dataPointer->posIndex == 0);
    // } else {
    //   REQUIRE(false);
    // }
  }
}