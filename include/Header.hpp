#ifndef HEADER_HPP
#include <cstdint>
#include <vector>

using i32 = int32_t;
using i64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float_t;
using f64 = double_t;

using std::vector;

class DBRowInfo
{
  public:
    u16 location;
    u16 size;
    DBRowInfo() = default;
    DBRowInfo(u16 location, u16 size) : location{location}, size{size}
    {
    }
};

enum class PageType
{
    B_PLUS_TREE_LEAF = 0,
    B_PLUS_TREE_INTERIOR = 1,
    TABLE = 2
};

class PageHeader
{
  public:
    PageType pageType;
    u16 numOfEntries;
    u16 endOfFreeSpace;
    vector<DBRowInfo> recordInfoArray;
};

class DatabaseHeader
{
  public:
    u16 pageSize;

    size_t Size() const
    {
        return sizeof(pageSize);
    }
};
#endif // HEADER_HPP