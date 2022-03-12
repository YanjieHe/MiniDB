#include "BufferManager.hpp"
#include "Page.hpp"
#include "TestUtils.hpp"
#include <catch2/catch.hpp>

const size_t PAGE_SIZE = 65536;

vector<json> LoadPage(string path, u16 bufferID);
vector<DBColumn> ExampleColumns();
vector<DBRow> ExampleRows();
Buffer CreateExamplePage();

vector<DBColumn> ExampleColumns() {
  return vector<DBColumn>{DBColumn(false, TypeTag::TEXT, "book"),
                          DBColumn(false, TypeTag::INTEGER, "published_year")};
}

vector<DBRow> ExampleRows() {
  return vector<DBRow>{
      DBRow({DBString("The Red and the Black"), DBInt64(1830)}),
      DBRow({DBString("The Million Pound Bank Note"), DBInt64(1893)})};
}

Buffer CreateExamplePage() {
  auto columns = ExampleColumns();
  vector<DBRow> records = ExampleRows();
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

vector<json> LoadPage(string path, u16 bufferID) {
  vector<DBColumn> columns = ExampleColumns();
  Buffer buffer(PAGE_SIZE);
  BufferManager bufferManager(path);
  bufferManager.LoadBuffer(bufferID, buffer);
  Page page(columns, buffer, PAGE_SIZE);
  REQUIRE(page.header.pageType == PageType::TABLE);

  vector<json> result;
  for (size_t i = 0; i < page.NumOfRows(); i++) {
    const auto &record = page.GetRow(buffer, i);
    result.push_back(DBRowToJson(record));
  }
  return result;
}

TEST_CASE("Test Page", "[Page]") {
  Buffer buffer = CreateExamplePage();
  auto columns = ExampleColumns();
  DatabaseHeader dbHeader;
  dbHeader.pageSize = PAGE_SIZE;

  SECTION("Example Page") {
    string path = "output/example_page.bin";

    CreateEmptyDatabaseFile(path, dbHeader);
    BufferManager bufferManager(path);
    bufferManager.SaveBuffer(0, buffer);

    auto rows = LoadPage(path, 0);
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
    BufferManager bufferManager(path);
    bufferManager.SaveBuffer(0, buffer);

    auto rows = LoadPage(path, 0);
    REQUIRE(rows == vector<json>{{"The Red and the Black", 1830},
                                 {"The Million Pound Bank Note", 1893},
                                 {"The Three-Body Problem", 2008}});
  }

  SECTION("Insert a Row") {
    string path = "output/insert_a_row.bin";
    Page page(columns, buffer, PAGE_SIZE);
    REQUIRE(page.NumOfRows() == 2);
    DBRow newRow = DBRow({DBString("The Three-Body Problem"), DBInt64(2008)});
    PrintFormattedJson(PageHeaderToJson(page.header));
    REQUIRE(page.InsertRow(buffer, newRow, 1));
    PrintFormattedJson(PageHeaderToJson(page.header));
    REQUIRE(page.NumOfRows() == 3);
    page.UpdateHeader(buffer);

    CreateEmptyDatabaseFile(path, dbHeader);
    BufferManager bufferManager(path);
    bufferManager.SaveBuffer(0, buffer);

    auto rows = LoadPage(path, 0);
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
    BufferManager bufferManager(path);
    bufferManager.SaveBuffer(0, buffer);

    auto rows = LoadPage(path, 0);
    REQUIRE(rows == vector<json>{{"The Red and the Black", 1830}});
  }
}