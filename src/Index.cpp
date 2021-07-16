#include "Index.hpp"
#include "DBException.hpp"

using std::holds_alternative;
using std::make_shared;
using std::static_pointer_cast;

int CompareIndexKey(const Index::Key &x, const Index::Key &y) {
  if (holds_alternative<i64>(x) && holds_alternative<i64>(y)) {
    return GetComparisonIntResult(std::get<i64>(x), std::get<i64>(y));
  } else if (holds_alternative<string>(x) && holds_alternative<string>(y)) {
    return GetComparisonIntResult(std::get<string>(x), std::get<string>(y));
  } else {
    throw DBException(
        "the two index keys for comparision have two different types");
  }
}

int CompareIndex(const Index &x, const Index &y) {
  if (x.keys.size() == y.keys.size()) {
    for (size_t i = 0; i < x.keys.size(); i++) {
      int result = CompareIndexKey(x.keys[i], y.keys[i]);
      if (result < 0) {
        return -1;
      } else if (result > 0) {
        return 1;
      } else {
      }
    }
    return 0;
  } else {
    throw DBException("the two index has different number of key components.");
  }
}
NonLeafPage::NonLeafPage(u16 pageID, const vector<DBColumn> &keyColumns, const PageHeader &header, Buffer &buffer)
    : pageID{pageID}, header{header}, keyColumns{keyColumns}
{
    for (size_t i = 0; i < header.recordInfoArray.size(); i++)
    {
        const auto &recordInfo = header.recordInfoArray.at(i);
        buffer.pos = recordInfo.location;
        if (i % 2 == 0)
        {
            // page pointer
            u16 bufferID = buffer.ReadU16();
            pagePointers.push_back(bufferID);
        }
        else
        {
            // primary key
            auto record = buffer.ReadRecord(this->keyColumns);
            indexList.push_back(RecordToIndex(this->keyColumns, record));
        }
    }
}

LeafPage::LeafPage(u16 pageID, const vector<DBColumn> &keyColumns, const PageHeader &header, Buffer &buffer)
    : pageID{pageID}, header{header}, keyColumns{keyColumns}
{
    for (size_t i = 0; i < header.recordInfoArray.size(); i++)
    {
        const auto &recordInfo = header.recordInfoArray.at(i);
        buffer.pos = recordInfo.location;
        if (i == 0 || (i + 1) == header.recordInfoArray.size())
        {
            // page pointer
            u16 bufferID = buffer.ReadU16();
            pagePointers.push_back(bufferID);
        }
        else if (i % 2 == 1)
        {
            // record pointer
            u16 bufferID = buffer.ReadU16();
            u16 posIndex = buffer.ReadU16();
            recordPointers.emplace_back(bufferID, posIndex);
        }
        else
        {
            // primary key
            auto record = buffer.ReadRecord(this->keyColumns);
            indexList.push_back(RecordToIndex(this->keyColumns, record));
        }
    }
}
LeafPage::LeafPage(u16 pageID, const vector<DBColumn> &keyColumns, vector<Index> indexList,
                   vector<RecordPointer> recordPointers, vector<u16> pagePointers)
    : pageID{pageID}, keyColumns{keyColumns}, indexList{indexList}, recordPointers{recordPointers}, pagePointers{
                                                                                                        pagePointers}
{
}

int CompareIndexKey(const Index::Key &x, const Index::Key &y)
{
    if (holds_alternative<i64>(x) && holds_alternative<i64>(y))
    {
        if (std::get<i64>(x) < std::get<i64>(y))
        {
            return -1;
        }
        else if (std::get<i64>(x) > std::get<i64>(y))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if (holds_alternative<string>(x) && holds_alternative<string>(y))
    {
        if (std::get<string>(x) < std::get<string>(y))
        {
            return -1;
        }
        else if (std::get<string>(x) > std::get<string>(y))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        throw DBException("the two index keys for comparision have two different types");
    }
}

int CompareIndex(const Index &x, const Index &y)
{
    if (x.keys.size() == y.keys.size())
    {
        for (size_t i = 0; i < x.keys.size(); i++)
        {
            if (int result = CompareIndexKey(x.keys[i], y.keys[i]); result != 0)
            {
                return result;
            }
        }
        return 0;
    }
    else
    {
        throw DBException("two indices have different number of keys");
    }
}

Index RecordToIndex(const vector<DBColumn> &keyColumns, const DBRow &record)
{
    vector<Index::Key> keys;
    for (size_t j = 0; j < keyColumns.size(); j++)
    {
        auto col = keyColumns.at(j);
        if (col.type == TypeTag::INTEGER)
        {
            keys.push_back(std::get<i64>(record.values.at(j)));
        }
        else if (col.type == TypeTag::TEXT)
        {
            keys.push_back(std::get<string>(record.values.at(j)));
        }
        else
        {
            throw DBException("the index column type is not supported");
        }
    }
    return Index(keys);
}

namespace bplustree
{

bool Search(BufferManager &bufferManager, size_t pageSize, Buffer &buffer, shared_ptr<IndexPage> root,
            const Index &index)
{
    if (root)
    {
        shared_ptr<IndexPage> cursor = root;
        while (cursor->IsLeaf() == false)
        {
            for (size_t i = 0; i < cursor->IndexList().size(); i++)
            {
                if (CompareIndex(index, cursor->IndexList().at(i)) < 0)
                {
                    u16 pagePointer = cursor->PagePointers().at(i);
                    cursor = LoadIndexPage(bufferManager, buffer, pagePointer, cursor->KeyColumns());
                    break;
                }
                if (i + 1 == cursor->IndexList().size())
                {
                    u16 pagePointer = cursor->PagePointers().at(i + 1);
                    cursor = LoadIndexPage(bufferManager, buffer, pagePointer, cursor->KeyColumns());
                    break;
                }
            }
        }

        for (size_t i = 0; i < cursor->IndexList().size(); i++)
        {
            if (CompareIndex(cursor->IndexList().at(i), index))
            {
                return true;
            }
        }
        return false;
    }
    else
    {
        return false;
    }
}

shared_ptr<IndexPage> LoadIndexPage(BufferManager &bufferManager, Buffer &buffer, u16 pagePointer,
                                    const vector<DBColumn> &keyColumns)
{
    bufferManager.LoadBuffer(pagePointer, buffer);
    PageHeader header;
    LoadHeader(buffer, header);
    if (header.pageType == PageType::B_PLUS_TREE_LEAF)
    {
        return make_shared<LeafPage>(pagePointer, keyColumns, header, buffer);
    }
    else if (header.pageType == PageType::B_PLUS_TREE_INTERIOR)
    {
        return make_shared<NonLeafPage>(pagePointer, keyColumns, header, buffer);
    }
    else
    {
        throw DBException("The page is not for B+ tree");
    }
}

shared_ptr<IndexPage> Insert(BufferManager &bufferManager, Buffer &buffer, size_t order, shared_ptr<IndexPage> &root,
                             const vector<DBColumn> &keyColumns, const Index &index, RecordPointer recordPointer)
{
    if (root)
    {
        shared_ptr<IndexPage> cursor = root;
        shared_ptr<IndexPage> parent;

        while (cursor->IsLeaf() == false)
        {
            parent = cursor;
            for (int i = 0; i < cursor->IndexList().size(); i++)
            {
                if (CompareIndex(index, cursor->IndexList().at(i)) < 0)
                {
                    u16 pagePointer = cursor->PagePointers().at(i);
                    cursor = LoadIndexPage(bufferManager, buffer, pagePointer, cursor->KeyColumns());
                    break;
                }

                if (i + 1 == cursor->IndexList().size())
                {
                    u16 pagePointer = cursor->PagePointers().at(i + 1);
                    cursor = LoadIndexPage(bufferManager, buffer, pagePointer, cursor->KeyColumns());
                    break;
                }
            }
        }

        if (cursor->IndexList().size() < order)
        {
            size_t i = 0;
            while (CompareIndex(index, cursor->IndexList().at(i)) > 0 && i < cursor->IndexList().size())
            {
                i++;
            }
            cursor->IndexList().insert(cursor->IndexList().begin() + i, index);
            vector<RecordPointer> &recordPointers = static_pointer_cast<LeafPage>(cursor)->recordPointers;
            recordPointers.insert(recordPointers.begin() + i, recordPointer);
            // TO DO
        }
        else
        {
            // TO DO
        }
    }
    else
    {
        u16 pageID = bufferManager.AllocatePage();
        root = make_shared<LeafPage>(pageID, keyColumns, vector<Index>{index}, vector<RecordPointer>{recordPointer},
                                     vector<u16>{});
    }
}
void InsertInternal(size_t order, const Index &index, IndexPage *cursor, IndexPage *child)
{
    if (cursor->IndexList().size() < order)
    {
        size_t i = 0;
        while (CompareIndex(index, cursor->IndexList().at(i)) > 0 && i < cursor->IndexList().size())
        {
            i++;
        }
        for (size_t j = cursor->IndexList().size(); j > i; j--)
        {
            cursor->IndexList().at(j) = cursor->IndexList().at(j - 1);
        }
        for (size_t j = cursor->IndexList().size() + 1; j > i + 1; j--)
        {
            cursor->PagePointers().at(j) = cursor->PagePointers().at(j - 1);
        }
        cursor->IndexList().at(i) = index;
        cursor->PagePointers().at(i + 1) = child->PageID();
    }
    else
    {
        vector<Index> virtualKeys(order + 1);
        vector<u16> virtualPtrs(order + 2);
        for (int k = 0; k < order; k++)
        {
            virtualKeys.at(k) = cursor->IndexList().at(k);
        }
        for (int k = 0; k < order + 1; k++)
        {
            virtualPtrs.at(k) = cursor->PagePointers().at(k);
        }
        int i = 0;
        while (CompareIndex(index, virtualKeys.at(i)) > 0 && i < order)
        {
            i++;
        }
        for (int k = order; k > i; k--)
        {
            virtualKeys.at(k) = virtualKeys.at(k - 1);
        }
        virtualKeys.at(i) = index;
        for (int k = order + 1; k > i + 1; k--)
        {
            virtualPtrs.at(k) = virtualPtrs.at(k - 1);
        }
        virtualPtrs.at(i + 1) = child->PageID();
        NonLeafPage *newPage;
        // TO DO
    }
}
} // namespace bplustree
