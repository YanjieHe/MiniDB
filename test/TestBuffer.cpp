#include <catch2/catch.hpp>
#include "Buffer.hpp"

TEST_CASE("Test Buffer", "[Buffer]") {
  Buffer buf(8);
  SECTION("Test \"MoveBlock\"") {
    buf.bytes = vector<u8>{9, 8, 7, 6, 5, 4, 3, 2};
    buf.Reset();
    buf.MoveBlock(2, 3, 4);
    REQUIRE(buf.bytes == vector<u8>{9, 8, 7, 6, 7, 6, 5, 2});
  }
}