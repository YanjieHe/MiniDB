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
  BPlusTreeNode(const BPlusTreeNode &other);
  bool IsLeaf(const Page &page) const;
  size_t Size(const Page &page) const;

  BPlusTreeNode operator=(const BPlusTreeNode &other);
};

class BPlusTree {
public:
  /* max number of keys in a node */
  size_t order;
  BufferManager &bufferManager;
  optional<u16> rootPageID;
  size_t pageSize;
  Buffer buffer;
  vector<DBColumn> columns;

  BPlusTree(size_t order, BufferManager &bufferManager,
            optional<u16> rootPageID, size_t pageSize,
            const vector<DBColumn> &columns);

  void Insert(DBIndex indexToInsert, i64 dataPointerVal);
  void InsertInternal(DBIndex indexToInsert, BPlusTreeNode cursor,
                      BPlusTreeNode child);
  optional<i64> Search(DBIndex indexToSearch);
  optional<BPlusTreeNode> FindParent(BPlusTreeNode cursor, BPlusTreeNode child);
  u16 CreateNewNode(PageType pageType);
  Page LoadPage(size_t pageID);
  void SavePage(Page& page, size_t pageID);
  void LoadKeyValuePairs(Page& page, vector<DBIndex>& keys, vector<i64>& pointers);
  void SaveKeyValuePairs(Page& page, const vector<DBIndex>& keys, const vector<i64>& pointers);

  /********** Utility Static Methods **********/
  static DBIndex GetIndexFromRow(const DBRow &row);
  static DBRow ConvertIndexToRow(const DBIndex &index, i64 dataPointerVal);
  static i64 GetPointerValueFromRow(const DBRow &row);
};

#endif // B_PLUS_TREE_HPP