#include <catch2/catch.hpp>

#include "BufferManager.hpp"
#include "TestUtils.hpp"

TEST_CASE("Test Buffer Manager", "[BufferManager]") {
  string path = "output/test_buffer_manager";
  Buffer buffer(PAGE_SIZE);
  DatabaseHeader dbHeader;
  dbHeader.nPages = 0;
  dbHeader.pageSize = PAGE_SIZE;

  BufferManager bufferManager(path, dbHeader);

  u16 bufferID0 = bufferManager.AllocatePage();
  REQUIRE(bufferID0 == 0);
  u16 bufferID1 = bufferManager.AllocatePage();
  REQUIRE(bufferID1 == 1);
  buffer.bytes.at(0) = 163;
  bufferManager.SaveBuffer(bufferID0, buffer);

  SECTION("Test Loading a Buffer") {
    buffer.bytes.at(0) = 0;
    bufferManager.LoadBuffer(bufferID0, buffer);
    REQUIRE(buffer.bytes.at(0) == 163);
  }

  SECTION("Test Saving a Buffer") {
    buffer.bytes.at(0) = 0;
    buffer.bytes.at(20) = 45;
    bufferManager.SaveBuffer(bufferID0, buffer);
    buffer.bytes.at(20) = 0;
    bufferManager.LoadBuffer(bufferID0, buffer);
    REQUIRE(buffer.bytes.at(20) == 45);
  }

  SECTION("Test Allocating a new Buffer") {
    u16 bufferID2 = bufferManager.AllocatePage();
    REQUIRE(bufferID2 == 2);
    bufferManager.LoadBuffer(bufferID2, buffer);
    REQUIRE(buffer.bytes.at(0) == 0);
    bufferManager.LoadBuffer(bufferID0, buffer);
    REQUIRE(buffer.bytes.at(0) == 163);
  }

  // SECTION("Test Allocating the Second Buffer") {
  //   buffer.bytes.at(0) = 0;
  //   buffer.bytes.at(20) = 0;
  //   buffer.bytes.at(30) = 99;
  //   u16 bufferID2 = bufferManager.AllocatePage();
  //   REQUIRE(bufferID2 == 1);
  //   bufferManager.SaveBuffer(bufferID2, buffer);
  //   bufferManager.LoadBuffer(bufferID, buffer);
  //   REQUIRE(buffer.bytes.at(20) == 45);
  //   REQUIRE(buffer.bytes.at(30) == 0);
  //   bufferManager.LoadBuffer(bufferID2, buffer);
  //   REQUIRE(buffer.bytes.at(20) == 0);
  //   REQUIRE(buffer.bytes.at(30) == 99);
  // }
}