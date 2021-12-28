#ifndef B_PLUS_TREE_HPP
#define B_PLUS_TREE_HPP
#include "BufferManager.hpp"
#include "DBIndex.hpp"
#include "DBRow.hpp"
#include "Page.hpp"
#include <optional>

using std::optional;

class BPlusTreeNode {
public:
  u16 pageID;

  explicit BPlusTreeNode(u16 pageID);
  BPlusTreeNode(const BPlusTreeNode& other);
  bool IsLeaf(const Page &page) const;
  size_t Size(const Page &page) const;

  BPlusTreeNode operator=(const BPlusTreeNode &other);
};

class BPlusTree {
public:
  size_t order;
  BufferManager &bufferManager;
  optional<u16> rootPageID;
  size_t pageSize;
  size_t maxNumOfKeysInOnePage;
  Buffer buffer;
  vector<DBColumn> columns;

  BPlusTree(size_t order, BufferManager &bufferManager, optional<u16> rootPageID,
            size_t pageSize, size_t maxNumOfKeysInOnePage,
            const vector<DBColumn> &columns);

  void Insert(DBIndex indexToInsert, i64 dataPointerVal);
  void InsertInternal(DBIndex indexToInsert, BPlusTreeNode cursor, BPlusTreeNode child);
  optional<i64> Search(DBIndex indexToSearch);
  optional<BPlusTreeNode> FindParent(BPlusTreeNode cursor, BPlusTreeNode child);
  u16 CreateNewNode();
  Page LoadPage(size_t pageID);

  static DBIndex GetIndexFromRow(const DBRow &row);
  static DBRow ConvertIndexToRow(const DBIndex &index, i64 dataPointerVal);
  static i64 GetPointerValueFromRow(const DBRow &row);
  static size_t Size(const Page &page);
};

#endif // B_PLUS_TREE_HPP