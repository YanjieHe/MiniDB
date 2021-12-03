#include "BPlusTree.hpp"
#include "Page.hpp"
#include "DBException.hpp"

using std::holds_alternative;

BPlusTree::BPlusTree(BufferManager &bufferManager, u16 rootPageID,
                     size_t pageSize, size_t maxNumOfKeysInOnePage,
                     const vector<DBColumn> &columns)
    : bufferManager{bufferManager}, rootPageID{rootPageID}, pageSize{pageSize},
      maxNumOfKeysInOnePage{maxNumOfKeysInOnePage}, buffer(pageSize),
      columns{columns} {}

void BPlusTree::Insert(DBIndex indexToInsert, DBRow data) {
  bufferManager.LoadBuffer(rootPageID, buffer);
  Page page(columns, buffer, pageSize);

  for (size_t i = 0; i < page.NumOfRows(); i++) {
    DBRow row = page.GetRow(buffer, i);
    i64 dataPointerVal = std::get<i64>(row.values[0]);
    DBIndex index = GetIndexFromRow(row);
    if (CompareIndex(indexToInsert, index) < 0) {
      // found the position for the new index
      if (page.NumOfRows() >= maxNumOfKeysInOnePage) {

      } else {
        // insert the new key-value pair in the current page
      }
    }
  }
}

DBIndex BPlusTree::GetIndexFromRow(const DBRow &row) {
  if (row.values.size() <= 1) {
    throw DBException("wrong number of items in a row for index");
  }
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

static DBRow ConvertIndexToRow(const DBIndex &index, i64 dataPointerVal) {
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