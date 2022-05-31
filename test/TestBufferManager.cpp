#include <catch2/catch.hpp>
#include <iostream>

#include "BufferManager.hpp"
#include "TestUtils.hpp"

using std::cout;
using std::endl;

TEST_CASE("Test Buffer Manager", "[BufferManager]") {
  string path = "output/test_buffer_manager.bin";
  Buffer buffer(255);
  DatabaseHeader dbHeader;
  dbHeader.nPages = 0;
  dbHeader.pageSize = PAGE_SIZE;

  CreateEmptyDatabaseFile(path, dbHeader);
  BufferManager bufferManager(path, dbHeader);

  u16 bufferID = bufferManager.AllocatePage();
  REQUIRE(bufferID == 0);
  buffer.bytes.at(0) = 163;
  bufferManager.SaveBuffer(bufferID, buffer);

  SECTION("Test Loading a Buffer") {
    buffer.bytes.at(0) = 0;
    bufferManager.LoadBuffer(bufferID, buffer);
    REQUIRE(buffer.bytes.at(0) == 163);
  }

  SECTION("Test Saving a Buffer") {
    buffer.bytes.at(0) = 0;
    buffer.bytes.at(20) = 45;
    bufferManager.SaveBuffer(bufferID, buffer);
    buffer.bytes.at(20) = 0;
    bufferManager.LoadBuffer(bufferID, buffer);
    REQUIRE(buffer.bytes.at(20) == 45);
    cout << "20: " << (int)buffer.bytes.at(20) << endl;
  }

  SECTION("Test Allocating the Second Buffer") {
    // buffer.bytes.at(0) = 0;
    // buffer.bytes.at(20) = 0;
    // buffer.bytes.at(30) = 99;
    // u16 bufferID2 = bufferManager.AllocatePage();
    // REQUIRE(bufferID2 == 1);
    // bufferManager.SaveBuffer(bufferID2, buffer);
    // bufferManager.LoadBuffer(bufferID, buffer);
    // REQUIRE(buffer.bytes.at(20) == 45);
    // REQUIRE(buffer.bytes.at(30) == 0);
    // bufferManager.LoadBuffer(bufferID2, buffer);
    // REQUIRE(buffer.bytes.at(20) == 0);
    // REQUIRE(buffer.bytes.at(30) == 99);
  }
}