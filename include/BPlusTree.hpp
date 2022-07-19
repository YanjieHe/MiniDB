#ifndef B_PLUS_TREE_HPP
#define B_PLUS_TREE_HPP
#include <memory>
#include <optional>

#include "BufferManager.hpp"
#include "DBIndex.hpp"
#include "DBRow.hpp"
#include "Page.hpp"

using std::optional;

class BPlusTreeSharedData {
 public:
  /* max number of keys in a node */
  int order;
  BufferManager &bufferManager;
  size_t pageSize;
  Buffer buffer;
  vector<DBColumn> columns;
  BPlusTreeSharedData(int order, BufferManager &bufferManager, size_t pageSize,
                      const vector<DBColumn> &columns);
};

class BPlusTreeNode {
 public:
  typedef std::shared_ptr<BPlusTreeNode> Ptr;

  BPlusTreeSharedData &sharedData;
  u16 pageID;
  bool isLeaf;
  int size;
  vector<DBIndex> indices;
  vector<u16> pointers;

  BPlusTreeNode(BPlusTreeSharedData &sharedData);
  BPlusTreeNode(BPlusTreeSharedData &sharedData, u16 pageID);
  void Save() const;
};

// class BPlusTreePage {
//  public:
//   u16 pageID;
//   BPlusTreeSharedData &sharedData;

//   BPlusTreeNode LoadNode();
//   void SaveNode(const BPlusTreeNode &node);
// };

class IndexPage {
 public:
  PageHeader header;
  vector<DBColumn> columns;

  IndexPage(const vector<DBColumn> &columns, Buffer &buffer, size_t pageSize);
  IndexPage(PageHeader pageHeader, const vector<DBColumn> &columns);
  void LoadAllIndices(Buffer &buffer, vector<DBIndex> &indices);
  void LoadAllPointers(Buffer &buffer, vector<u16> &pointers);
  DBIndex ReadIndex(Buffer &buffer);
  void Store(const BPlusTreeNode *node);
};

class BPlusTree {
 public:
  BPlusTreeNode::Ptr root;
  BPlusTreeSharedData sharedData;

  BPlusTree(int order, BufferManager &bufferManager, size_t pageSize,
            const vector<DBColumn> &columns);
  void Insert(const DBIndex &indexToInsert);
  void InsertInternal(const DBIndex &indexToInsert, BPlusTreeNode::Ptr cursor,
                      BPlusTreeNode::Ptr child);
  BPlusTreeNode::Ptr FindParent(BPlusTreeNode::Ptr cursor,
                                BPlusTreeNode::Ptr child);
  optional<DataPointer> Search(DBIndex index);
};

#endif  // B_PLUS_TREE_HPP