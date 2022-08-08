#include "BPlusTree.hpp"

#include "DBException.hpp"
#include "Page.hpp"

using std::holds_alternative;
using std::make_optional;
using std::make_shared;

#include <iostream>

using std::cout;
using std::endl;

IndexPage::IndexPage(const vector<DBColumn> &columns, Buffer &buffer,
                     size_t pageSize)
    : header{EmptyIndexPageHeader(pageSize, PageType::B_PLUS_TREE_LEAF)},
      columns{columns} {
  buffer.PreserveBufferPos([&]() { buffer.LoadHeader(header); });
}

IndexPage::IndexPage(PageHeader pageHeader, const vector<DBColumn> &columns)
    : header{pageHeader}, columns{columns} {}

/* An example of a B+ tree node:
P1 | K1 | P2 | K2 | P3 | K3 | P4
0  | 1  | 2  | 3  | 4  | 5  | 6
*/
size_t IndexPage::LoadAllIndices(Buffer &buffer,
                                 vector<DBIndex> &indices) const {
  size_t offset = 0;
  for (size_t i = 0; i < header.recordInfoArray.size(); i++) {
    if (i % 2 == 1) {
      buffer.PreserveBufferPos([this, &buffer, &indices, i, offset]() {
        buffer.pos = header.recordInfoArray.at(i).location;
        DBIndex index = ReadDBIndex(columns, buffer);
        indices.at(offset) = index;
      });
      offset++;
    }
  }
  return offset;
}

size_t IndexPage::LoadAllPointers(Buffer &buffer, vector<i64> &pointers) const {
  size_t offset = 0;
  for (size_t i = 0; i < header.recordInfoArray.size(); i++) {
    if (i % 2 == 0) {
      buffer.PreserveBufferPos([this, &buffer, &pointers, i, offset]() {
        buffer.pos = header.recordInfoArray.at(i).location;
        pointers.at(offset) = buffer.ReadI64();
      });
      offset++;
    }
  }
  return offset;
}

void IndexPage::Store(const BPlusTreeNode *node) {
  header.numOfEntries = node->size + (node->size + 1);
  if (node->isLeaf) {
    header.pageType = PageType::B_PLUS_TREE_LEAF;
  } else {
    header.pageType = PageType::B_PLUS_TREE_NON_LEAF;
  }
  header.recordInfoArray.clear();
  size_t curPos = node->sharedData.pageSize;
  size_t dataPointerSize = sizeof(i64);
  for (size_t i = 0; i < header.numOfEntries; i++) {
    if (i % 2 == 0) {
      curPos = curPos - dataPointerSize;
      header.recordInfoArray.emplace_back(curPos, dataPointerSize);
    } else {
      curPos = curPos - node->indices.at(i / 2).ComputeSize();
      header.recordInfoArray.emplace_back(
          curPos, node->indices.at(i / 2).ComputeSize());
    }
  }
  header.endOfFreeSpace = curPos;
  node->sharedData.buffer.Clear();
  node->sharedData.buffer.SaveHeader(header);
  node->sharedData.bufferManager.SaveBuffer(node->pageID,
                                            node->sharedData.buffer);
  for (size_t i = 0; i < header.recordInfoArray.size(); i++) {
    if (i % 2 == 0) {
      /* pointer */
      node->sharedData.buffer.pos = header.recordInfoArray.at(i).location;
      node->sharedData.buffer.WriteI64(node->pointers.at(i / 2));
    } else {
      /* index */
      node->sharedData.buffer.pos = header.recordInfoArray.at(i).location;
      const DBIndex &index = node->indices.at(i / 2);
      for (const auto &key : index.keys) {
        if (holds_alternative<i64>(key)) {
          node->sharedData.buffer.WriteI64(std::get<i64>(key));
        } else if (holds_alternative<string>(key)) {
          const string &text = std::get<string>(key);
          node->sharedData.buffer.WriteU16(text.size());
          node->sharedData.buffer.WriteText(text);
        } else {
          throw DBException("index key type error");
        }
      }
    }
  }

  node->sharedData.bufferManager.SaveBuffer(node->pageID,
                                            node->sharedData.buffer);
}

const PageHeader &IndexPage::Header() const { return header; }

const vector<DBColumn> &IndexPage::Columns() const { return columns; }

BPlusTreeSharedData::BPlusTreeSharedData(int order,
                                         BufferManager &bufferManager,
                                         size_t pageSize,
                                         const vector<DBColumn> &columns)
    : order{order},
      bufferManager{bufferManager},
      pageSize{pageSize},
      buffer(pageSize),
      columns{columns} {}

/* create an empty B+ tree node page */
BPlusTreeNode::BPlusTreeNode(BPlusTreeSharedData &sharedData)
    : sharedData{sharedData},
      pageID{sharedData.bufferManager.AllocatePage()},
      isLeaf{false},
      size{0},
      indices(sharedData.order),
      pointers(sharedData.order + 1) {
  for (size_t i = 0; i < pointers.size(); i++) {
    pointers.at(i) = -1;
  }
  sharedData.buffer.Clear();
  sharedData.buffer.SaveHeader(EmptyIndexPageHeader(
      sharedData.pageSize, PageType::B_PLUS_TREE_NON_LEAF));
  sharedData.bufferManager.SaveBuffer(pageID, sharedData.buffer);
}

/* An example of a B+ tree node:
P1 | K1 | P2 | K2 | P3 | K3 | P4
*/
BPlusTreeNode::BPlusTreeNode(BPlusTreeSharedData &sharedData, u16 pageID)
    : sharedData{sharedData},
      pageID{pageID},
      isLeaf{false},
      size{0},
      indices(sharedData.order),
      pointers(sharedData.order + 1) {
  for (size_t i = 0; i < pointers.size(); i++) {
    pointers.at(i) = -1;
  }
  sharedData.bufferManager.LoadBuffer(pageID, sharedData.buffer);
  IndexPage indexPage(sharedData.columns, sharedData.buffer,
                      sharedData.pageSize);
  this->isLeaf = indexPage.header.pageType == PageType::B_PLUS_TREE_LEAF;
  this->size = indexPage.header.numOfEntries / 2;
  indexPage.LoadAllIndices(sharedData.buffer, indices);
  indexPage.LoadAllPointers(sharedData.buffer, pointers);
}

void BPlusTreeNode::Save() const {
  IndexPage indexPage(sharedData.columns, sharedData.buffer,
                      sharedData.pageSize);
  indexPage.Store(this);
}

BPlusTree::BPlusTree(int order, BufferManager &bufferManager, size_t pageSize,
                     const vector<DBColumn> &columns)
    : root(), sharedData(order, bufferManager, pageSize, columns) {}

void BPlusTree::Insert(const DBIndex &indexToInsert, i64 dataPointer) {
  int order = sharedData.order;

  /* if there is no root node, create a new root node */
  if (bool(root) == false) {
    root = make_shared<BPlusTreeNode>(sharedData);
    root->indices.at(0) = indexToInsert;
    root->pointers.at(0) = dataPointer;
    root->isLeaf = true;
    root->size = 1;
    root->Save();
  } else {
    /* traverse the B+ tree */

    BPlusTreeNode::Ptr cursor = root;
    BPlusTreeNode::Ptr parent;

    /* keep traversing till the cursor reaches a leaf node */
    while (cursor->isLeaf == false) {
      parent = cursor;
      for (int i = 0; i < cursor->size; i++) {
        if (indexToInsert < cursor->indices.at(i)) {
          /* found the position to insert node */
          cursor =
              make_shared<BPlusTreeNode>(sharedData, cursor->pointers.at(i));
          break;
        } else if (i == cursor->size - 1) {
          cursor =
              make_shared<BPlusTreeNode>(sharedData, cursor->pointers.at(i));
          break;
        } else {
          /* pass */
        }
      }
    }
    if (cursor->size < order) {
      int i = 0;
      while (i < cursor->size && indexToInsert > cursor->indices.at(i)) {
        i++;
      }
      for (int j = cursor->size; j > i; j--) {
        cursor->indices.at(j) = cursor->indices.at(j - 1);
      }
      cursor->indices.at(i) = indexToInsert;
      cursor->size++;
      cursor->pointers.at(cursor->size) = cursor->pointers.at(cursor->size - 1);
      cursor->pointers.at(cursor->size - 1) = -1;

      /*************************/
      for (int j = cursor->size; j > i; j--) {
        cursor->pointers.at(j) = cursor->pointers.at(j - 1);
      }
      cursor->pointers.at(i) = dataPointer;
      /*************************/

      cursor->Save();
    } else {
      /* create a new leaf node */
      BPlusTreeNode::Ptr newLeaf = make_shared<BPlusTreeNode>(sharedData);
      vector<DBIndex> virtualIndices(order + 1);

      /*****************/
      vector<i64> virtualPointers(order + 2);
      /*****************/

      for (int k = 0; k < order; k++) {
        virtualIndices.at(k) = cursor->indices.at(k);
      }

      /*****************/
      for (int k = 0; k < order + 1; k++) {
        virtualPointers.at(k) = cursor->pointers.at(k);
      }
      /*****************/

      int location = 0;

      /* find where to insert the new node */
      while (location < order && indexToInsert > virtualIndices.at(location)) {
        location++;
      }

      /* move indices */
      for (int k = order; k > location; k--) {
        virtualIndices.at(k) = virtualIndices.at(k - 1);
      }

      /*********** BEGIN ************/
      for (int k = order + 1; k > location; k--) {
        virtualPointers.at(k) = virtualPointers.at(k - 1);
      }
      /************ END *************/

      virtualIndices.at(location) = indexToInsert;
      newLeaf->isLeaf = true;
      cursor->size = (order + 1) / 2;
      newLeaf->size = order + 1 - (order + 1) / 2;
      cursor->pointers.at(cursor->size) = newLeaf->pageID;
      newLeaf->pointers.at(newLeaf->size) = cursor->pointers.at(order);
      cursor->pointers.at(order) = -1;

      /* copy the first half of indices */
      for (int i = 0; i < cursor->size; i++) {
        cursor->indices.at(i) = virtualIndices.at(i);
      }

      /* update the new leaf indices to the virtual indices */
      for (int i = 0, j = cursor->size; i < newLeaf->size; i++, j++) {
        newLeaf->indices.at(i) = virtualIndices.at(j);
      }

      /*********** BEGIN ************/
      virtualPointers.at(location) = dataPointer;
      for (int i = 0; i < cursor->size; i++) {
        cursor->pointers.at(i) = virtualPointers.at(i);
      }

      for (int i = 0, j = cursor->size; i < newLeaf->size + 1; i++, j++) {
        newLeaf->pointers.at(i) = virtualPointers.at(j);
      }
      /************ END *************/

      cursor->Save();
      newLeaf->Save();

      if (cursor->pageID == root->pageID) {
        BPlusTreeNode::Ptr newRoot = make_shared<BPlusTreeNode>(sharedData);
        newRoot->indices.at(0) = newLeaf->indices.at(0);
        newRoot->pointers.at(0) = cursor->pageID;
        newRoot->pointers.at(1) = newLeaf->pageID;
        newRoot->isLeaf = false;
        newRoot->size = 1;
        root = newRoot;
        root->Save();
      } else {
        InsertInternal(newLeaf->indices.at(0), parent, newLeaf);
      }
    }
  }
}

void BPlusTree::InsertInternal(const DBIndex &indexToInsert,
                               BPlusTreeNode::Ptr cursor,
                               BPlusTreeNode::Ptr child) {
  int order = sharedData.order;
  if (cursor->size < order) {
    /* if there isn't an overflow */
    int i = 0;
    /* search the position to insert the current index */
    while (i < cursor->size && indexToInsert > cursor->indices.at(i)) {
      i++;
    }
    /* first part | position to insert | second part */
    /* move the second part forward one slot to make room for the new index */
    for (int j = cursor->size; j > i; j--) {
      cursor->indices.at(j) = cursor->indices.at(j - 1);
    }
    /* move the pointers too */
    for (int j = cursor->size + 1; j > i + 1; j--) {
      cursor->pointers.at(j) = cursor->pointers.at(j - 1);
    }
    /* insert the index */
    cursor->indices.at(i) = indexToInsert;
    cursor->pointers.at(i) = -2;
    /* insert the pointer */
    cursor->pointers.at(i + 1) = child->pageID;
    /* increase the count of nodes */
    cursor->size++;
    cursor->Save();
  } else {
    /* if there is an overflow, split the node */
    /* create a new internal node */
    BPlusTreeNode::Ptr newInternal = make_shared<BPlusTreeNode>(sharedData);
    /* create virtual indices */
    vector<DBIndex> virtualIndices(order + 1);
    /* create virtual pointers */
    vector<i64> virtualPointers(order + 2);
    /* copy all the indices from the cursor to the virtual indices */
    for (int k = 0; k < order; k++) {
      virtualIndices.at(k) = cursor->indices.at(k);
    }
    /* copy all the pointers too */
    for (int k = 0; k < order + 1; k++) {
      virtualPointers.at(k) = cursor->pointers.at(k);
    }

    int i = 0;
    /* search the position to insert the current index */
    while (i < order && indexToInsert > virtualIndices.at(i)) {
      i++;
    }
    /* first part | position to insert | second part */
    /* move the second part forward one slot to make room for the new index */
    for (int k = order; k > i; k--) {
      virtualIndices.at(k) = virtualIndices.at(k - 1);
    }
    /* move the pointers too */
    for (int k = order + 1; k > i + 1; k--) {
      virtualPointers.at(k) = virtualPointers.at(k - 1);
    }
    /* insert the index */
    virtualIndices.at(i) = indexToInsert;
    virtualPointers.at(i) = -2;
    /* insert the pointer */
    virtualPointers.at(i + 1) = child->pageID;

    newInternal->isLeaf = false;
    cursor->size = (order + 1) / 2;
    newInternal->size = order - (order + 1) / 2;
    /* insert a new node as an internal node */
    for (int i = 0, j = cursor->size + 1; i < newInternal->size; i++, j++) {
      newInternal->indices.at(i) = virtualIndices.at(j);
    }
    for (int i = 0, j = cursor->size + 1; i < newInternal->size + 1; i++, j++) {
      newInternal->pointers.at(i) = virtualPointers.at(j);
    }
    cursor->Save();
    newInternal->Save();
    /* if the cursor is the root node */
    if (cursor->pageID == root->pageID) {
      BPlusTreeNode::Ptr newRoot = make_shared<BPlusTreeNode>(sharedData);
      newRoot->indices.at(0) = cursor->indices.at(cursor->size);
      newRoot->pointers.at(0) = cursor->pageID;
      newRoot->pointers.at(1) = newInternal->pageID;
      newRoot->isLeaf = false;
      newRoot->size = 1;
      root = newRoot;
      root->Save();
    } else {
      InsertInternal(cursor->indices.at(cursor->size), FindParent(root, cursor),
                     newInternal);
    }
  }
}

BPlusTreeNode::Ptr BPlusTree::FindParent(BPlusTreeNode::Ptr cursor,
                                         BPlusTreeNode::Ptr child) {
  BPlusTreeNode::Ptr parent;
  /* if the cursor reaches the end of the tree */
  if (cursor->isLeaf ||
      make_shared<BPlusTreeNode>(sharedData, cursor->pointers.at(0))->isLeaf) {
    return {};
  } else {
    for (int i = 0; i < cursor->size + 1; i++) {
      auto ithPointer =
          make_shared<BPlusTreeNode>(sharedData, cursor->pointers.at(i));
      if (ithPointer->pageID == child->pageID) {
        /* if found child node */
        parent = cursor;
        return parent;
      } else {
        parent = FindParent(ithPointer, child);
        if (parent) {
          return parent;
        }
      }
    }
    return parent;
  }
}

optional<i64> BPlusTree::Search(DBIndex index) {
  if (root) {
    BPlusTreeNode::Ptr cursor = root;

    while (cursor->isLeaf == false) {
      for (int i = 0; i < cursor->size; i++) {
        if (index < cursor->indices.at(i)) {
          cursor =
              make_shared<BPlusTreeNode>(sharedData, cursor->pointers.at(i));
          break;
        }

        if (i == cursor->size - 1) {
          cursor = make_shared<BPlusTreeNode>(sharedData,
                                              cursor->pointers.at(i + 1));
          break;
        }
      }
    }

    for (int i = 0; i < cursor->size; i++) {
      if (cursor->indices.at(i) == index) {
        return make_optional<i64>(cursor->pointers.at(i));
      }
    }

    return {};
  } else {
    return {};
  }
}