#include <catch2/catch.hpp>

#include "BufferManager.hpp"
#include "TestUtils.hpp"
#include <iostream>

TEST_CASE("Test Buffer Manager", "[BufferManager]") {
  string path = "output/test_buffer_manager";
  Buffer buffer(PAGE_SIZE);
  DatabaseHeader dbHeader;

  BufferManager bufferManager(path, dbHeader);

  u16 bufferID0 = bufferManager.AllocatePage();
  REQUIRE(bufferID0 == 0);
  u16 bufferID1 = bufferManager.AllocatePage();
  REQUIRE(bufferID1 == 1);
  buffer.bytes.at(0) = 163;
  bufferManager.SavePage(bufferID0, buffer);

  /* Test Loading a Buffer */
  buffer.bytes.at(0) = 0;
  bufferManager.LoadPage(bufferID0, buffer);
  REQUIRE(buffer.bytes.at(0) == 163);

  /* Test Saving a Buffer */
  buffer.bytes.at(0) = 37;
  buffer.bytes.at(20) = 45;
  bufferManager.SavePage(bufferID0, buffer);
  buffer.bytes.at(20) = 0;
  bufferManager.LoadPage(bufferID0, buffer);
  REQUIRE(buffer.bytes.at(20) == 45);

  /* Test Allocating a new Buffer */
  u16 bufferID2 = bufferManager.AllocatePage();
  REQUIRE(bufferID2 == 2);
  bufferManager.LoadPage(bufferID2, buffer);
  REQUIRE(buffer.bytes.at(0) == 0);
  bufferManager.LoadPage(bufferID0, buffer);
  REQUIRE(buffer.bytes.at(0) == 37);

  /* Test Allocating the Second Buffer */
  buffer.bytes.at(0) = 0;
  buffer.bytes.at(20) = 0;
  buffer.bytes.at(30) = 99;
  u16 bufferID3 = bufferManager.AllocatePage();
  REQUIRE(bufferID3 == 3);
  bufferManager.SavePage(bufferID3, buffer);
  bufferManager.LoadPage(bufferID0, buffer);
  REQUIRE(buffer.bytes.at(20) == 45);
  REQUIRE(buffer.bytes.at(30) == 0);
  bufferManager.LoadPage(bufferID3, buffer);
  REQUIRE(buffer.bytes.at(20) == 0);
  REQUIRE(buffer.bytes.at(30) == 99);

  bufferManager.UpdateDatabaseHeaderToDisk();
}