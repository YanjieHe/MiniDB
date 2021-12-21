#include "BPlusTree.hpp"
#include "DBException.hpp"
#include "Page.hpp"

using std::holds_alternative;
using std::make_optional;

BPlusTreeNode::BPlusTreeNode(u16 pageID) : pageID{pageID} {}
BPlusTreeNode::BPlusTreeNode(const BPlusTreeNode &other)
    : pageID{other.pageID} {}

bool BPlusTreeNode::IsLeaf(const Page &page) const {
  return page.header.pageType == PageType::B_PLUS_TREE_LEAF;
}

size_t BPlusTreeNode::Size(const Page &page) const {
  size_t nRows = page.NumOfRows();
  if (nRows == 0) {
    return 0;
  } else {
    return nRows - 1;
  }
}

BPlusTreeNode BPlusTreeNode::operator=(const BPlusTreeNode &other) {
  return other;
}

BPlusTree::BPlusTree(BufferManager &bufferManager, optional<u16> rootPageID,
                     size_t pageSize, size_t maxNumOfKeysInOnePage,
                     const vector<DBColumn> &columns)
    : bufferManager{bufferManager}, rootPageID{rootPageID}, pageSize{pageSize},
      maxNumOfKeysInOnePage{maxNumOfKeysInOnePage},
      buffer(pageSize), columns{columns} {}

optional<i64> BPlusTree::Search(DBIndex indexToSearch) {
  if (rootPageID.has_value()) {
    // traverse to find the pointer value
    BPlusTreeNode cursor(rootPageID.value());
    Page page = LoadPage(cursor.pageID);
    // till we reach leaf node
    while (cursor.IsLeaf(page) == false) {
      for (size_t i = 0; i < cursor.Size(page); i++) {
        // if the element to be found is not the current one
        // make sure the buffer is for the current page
        DBRow keyValuePair = page.GetRow(buffer, i);
        int comparisonResult =
            CompareIndex(indexToSearch, GetIndexFromRow(keyValuePair));
        if (comparisonResult < 0) {
          i64 pointer = GetPointerValueFromRow(keyValuePair);
          cursor = BPlusTreeNode(static_cast<u16>(pointer));
          page = LoadPage(cursor.pageID);
          break;
        }
        // if the program reaches the end of the cursor node
        if (i + 1 == cursor.Size(page)) {
          DBRow valueRow = page.GetRow(buffer, i + 1);
          i64 pointer = GetPointerValueFromRow(valueRow);
          cursor = BPlusTreeNode(static_cast<u16>(pointer));
          page = LoadPage(cursor.pageID);
          break;
        }
      }
    }

    // traverse the cursor and find the node with value x
    for (size_t i = 0; i < cursor.Size(page); i++) {
      // if found then return
      DBRow keyValuePair = page.GetRow(buffer, i);
      int comparisonResult =
          CompareIndex(indexToSearch, GetIndexFromRow(keyValuePair));
      if (comparisonResult == 0) {
        // found
        return make_optional(GetPointerValueFromRow(keyValuePair));
      }
    }
    // otherwise, the element is not in the tree
    // not found
    return {};
  } else {
    // the tree is empty
    return {};
  }
}

optional<BPlusTreeNode> BPlusTree::FindParent(BPlusTreeNode cursor,
                                              BPlusTreeNode child) {
  Page page = LoadPage(cursor.pageID);
  // if the cursor reaches the end of the tree
  if (cursor.IsLeaf(page)) {
    return {};
  } else {
    DBRow keyValuePair = page.GetRow(buffer, 0);
    i64 pointer = GetPointerValueFromRow(keyValuePair);
    BPlusTreeNode cursorChild = BPlusTreeNode(static_cast<u16>(pointer));
    page = LoadPage(cursorChild.pageID);
    if (cursorChild.IsLeaf(page)) {
      return {};
    } else {
      page = LoadPage(cursor.pageID);
    }
  }
  // traverse the current node wtih every child of it
  for (size_t i = 0; i < cursor.Size(page) + 1; i++) {
    // update the parent for the child node
    DBRow keyValuePair = page.GetRow(buffer, i);
    i64 pointer = GetPointerValueFromRow(keyValuePair);
    if (static_cast<u16>(pointer) == child.pageID) {
      return make_optional(cursor);
    } else {
      // otherwise, recursively traverse to find the child node
      cursor = BPlusTreeNode(static_cast<u16>(pointer));
      optional<BPlusTreeNode> parent = FindParent(cursor, child);
      // if the parent is found, then return the parent node
      if (parent) {
        return parent;
      }
    }
  }
  // the parent is not found
  return {};
}

u16 BPlusTree::CreateNewNode() {
  u16 pageID = bufferManager.AllocatePage();
  bufferManager.LoadBuffer(pageID, buffer);
  auto newNodeHeader = PageHeader(PageType::B_PLUS_TREE_LEAF, 0,
                                  static_cast<u16>(pageSize - 1), {});
  PreserveBufferPos(buffer, [&]() { SaveHeader(buffer, newNodeHeader); });
  return pageID;
}

Page BPlusTree::LoadPage(size_t pageID) {
  bufferManager.LoadBuffer(pageID, buffer);
  return Page(columns, buffer, pageSize);
}

DBIndex BPlusTree::GetIndexFromRow(const DBRow &row) {
  if (row.values.size() <= 1) {
    throw DBException("wrong number of items in a row for index");
  } else {
    vector<DBIndex::Key> keys(row.values.size() - 1);
    for (size_t i = 1; i < row.values.size(); i++) {
      if (holds_alternative<i64>(row.values[i])) {
        keys[i - 1] = std::get<i64>(row.values[i]);
      } else if (holds_alternative<string>(row.values[i])) {
        keys[i - 1] = std::get<string>(row.values[i]);
      } else {
        throw DBException("wrong type for index");
      }
    }
    return DBIndex(keys);
  }
}

DBRow BPlusTree::ConvertIndexToRow(const DBIndex &index, i64 dataPointerVal) {
  vector<DBRow::Value> values(index.keys.size() + 1);
  values[0] = dataPointerVal;
  for (size_t i = 0; i < index.keys.size(); i++) {
    if (holds_alternative<i64>(index.keys[i])) {
      values[i + 1] = std::get<i64>(index.keys[i]);
    } else {
      values[i + 1] = std::get<string>(index.keys[i]);
    }
  }
  return DBRow(values);
}

i64 BPlusTree::GetPointerValueFromRow(const DBRow &row) {
  if (row.values.size() <= 1) {
    throw DBException("wrong number of items in a row for index");
  } else {
    if (holds_alternative<i64>(row.values.back())) {
      return std::get<i64>(row.values.back());
    } else {
      throw DBException("wrong pointer value format");
    }
  }
}

size_t BPlusTree::Size(const Page &page) {
  if (page.header.numOfEntries == 0) {
    return 0;
  } else {
    return page.header.numOfEntries - 1;
  }
}
