#include <catch2/catch.hpp>

#include "Buffer.hpp"


TEST_CASE("Test Buffer", "[Buffer]") {
  Buffer buf(8);
  SECTION("Test Moving a Block") {
    buf.bytes = vector<u8>{9, 8, 7, 6, 5, 4, 3, 2};
    buf.ResetPosition();
    buf.MoveBlock(2, 3, 4);
    REQUIRE(buf.bytes == vector<u8>{9, 8, 7, 6, 7, 6, 5, 2});
  }
}