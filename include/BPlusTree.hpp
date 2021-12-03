#ifndef B_PLUS_TREE_HPP
#define B_PLUS_TREE_HPP
#include "BufferManager.hpp"
#include "DBIndex.hpp"
#include "DBRow.hpp"

class BPlusTree {
public:
  BufferManager &bufferManager;
  u16 rootPageID;
  size_t pageSize;
  size_t maxNumOfKeysInOnePage;
  Buffer buffer;
  vector<DBColumn> columns;

  BPlusTree(BufferManager &bufferManager, u16 rootPageID, size_t pageSize, size_t maxNumOfKeysInOnePage,
            const vector<DBColumn> &columns);

  void Insert(DBIndex indexToInsert, DBRow data);

  static DBIndex GetIndexFromRow(const DBRow &row);
  static DBRow ConvertIndexToRow(const DBIndex& index, i64 dataPointerVal);
};

#endif // B_PLUS_TREE_HPP