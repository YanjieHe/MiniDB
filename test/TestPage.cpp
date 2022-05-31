#include <catch2/catch.hpp>

#include "BufferManager.hpp"
#include "JsonSerializer.hpp"
#include "Page.hpp"
#include "TestUtils.hpp"

vector<json> LoadPage(BufferManager& bufferManager, u16 bufferID);

vector<json> LoadPage(BufferManager& bufferManager, u16 bufferID) {
  vector<DBColumn> columns = BookDataColumns();
  Buffer buffer(PAGE_SIZE);
  bufferManager.LoadBuffer(bufferID, buffer);
  Page page(columns, buffer, PAGE_SIZE);
  REQUIRE(page.header.pageType == PageType::TABLE);

  vector<json> result;
  for (size_t i = 0; i < page.NumOfRows(); i++) {
    const auto& record = page.GetRow(buffer, i);
    result.push_back(JsonSerializer::DBRowToJson(record));
  }
  return result;
}

TEST_CASE("Test Page", "[Page]") {
  Buffer buffer = CreateBookPage();
  auto columns = BookDataColumns();
  DatabaseHeader dbHeader;
  dbHeader.nPages = 0;
  dbHeader.pageSize = PAGE_SIZE;

  SECTION("Example Page") {
    string path = "output/example_page.bin";

    CreateEmptyDatabaseFile(path, dbHeader);
    BufferManager bufferManager(path, dbHeader);
    u16 pageID = bufferManager.AllocatePage();
    bufferManager.SaveBuffer(pageID, buffer);

    auto rows = LoadPage(bufferManager, pageID);
    REQUIRE(rows == vector<json>{{"The Red and the Black", 1830},
                                 {"The Million Pound Bank Note", 1893}});
  }

  SECTION("Add a Row") {
    string path = "output/add_a_row.bin";
    Page page(columns, buffer, PAGE_SIZE);
    REQUIRE(page.NumOfRows() == 2);
    DBRow newRow = DBRow({DBString("The Three-Body Problem"), DBInt64(2008)});
    REQUIRE(page.AddRow(buffer, newRow));
    REQUIRE(page.NumOfRows() == 3);
    page.UpdateHeader(buffer);

    CreateEmptyDatabaseFile(path, dbHeader);
    BufferManager bufferManager(path, dbHeader);
    u16 pageID = bufferManager.AllocatePage();
    bufferManager.SaveBuffer(pageID, buffer);

    auto rows = LoadPage(bufferManager, pageID);
    REQUIRE(rows == vector<json>{{"The Red and the Black", 1830},
                                 {"The Million Pound Bank Note", 1893},
                                 {"The Three-Body Problem", 2008}});
  }

  SECTION("Insert a Row") {
    string path = "output/insert_a_row.bin";
    Page page(columns, buffer, PAGE_SIZE);
    REQUIRE(page.NumOfRows() == 2);
    DBRow newRow = DBRow({DBString("The Three-Body Problem"), DBInt64(2008)});
    REQUIRE(page.InsertRow(buffer, newRow, 1));
    REQUIRE(page.NumOfRows() == 3);
    page.UpdateHeader(buffer);

    CreateEmptyDatabaseFile(path, dbHeader);
    BufferManager bufferManager(path, dbHeader);
    u16 pageID = bufferManager.AllocatePage();
    bufferManager.SaveBuffer(pageID, buffer);

    auto rows = LoadPage(bufferManager, pageID);
    REQUIRE(rows == vector<json>{{"The Red and the Black", 1830},
                                 {"The Three-Body Problem", 2008},
                                 {"The Million Pound Bank Note", 1893}});
  }
  SECTION("Delete a Row") {
    string path = "output/delete_a_row.bin";
    Page page(columns, buffer, PAGE_SIZE);
    REQUIRE(page.NumOfRows() == 2);
    REQUIRE(page.DeleteRow(buffer, 1));
    REQUIRE(page.NumOfRows() == 1);
    page.UpdateHeader(buffer);

    CreateEmptyDatabaseFile(path, dbHeader);
    BufferManager bufferManager(path, dbHeader);
    u16 pageID = bufferManager.AllocatePage();
    bufferManager.SaveBuffer(pageID, buffer);

    auto rows = LoadPage(bufferManager, pageID);
    REQUIRE(rows == vector<json>{{"The Red and the Black", 1830}});
  }
}