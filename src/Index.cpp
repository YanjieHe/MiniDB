#include "Index.hpp"
#include "DBException.hpp"

using std::holds_alternative;
using std::make_shared;
using std::static_pointer_cast;

<<<<<<< Updated upstream
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
    throw DBException("the two indices have different number of key components.");
  }
}
NonLeafPage::NonLeafPage(u16 pageID, const vector<DBColumn> &keyColumns, const PageHeader &header, Buffer &buffer)
    : pageID{pageID}, header{header}, keyColumns{keyColumns}
=======
NonLeafPage::NonLeafPage(u16 pageID, const vector<DBColumn> &keyColumns, const PageHeader &header, Buffer &buffer,
                         size_t order)
    : pageID{pageID}, header{header}, keyColumns{keyColumns}, indexList(order),
      pagePointers(order), order{order}, size{0}
>>>>>>> Stashed changes
{
    size_t pagePointersPos = 0;
    size_t indexListPos = 0;
    for (size_t i = 0; i < header.recordInfoArray.size(); i++)
    {
        const auto &recordInfo = header.recordInfoArray.at(i);
        buffer.pos = recordInfo.location;
        if (i % 2 == 0)
        {
            // page pointer
            u16 bufferID = buffer.ReadU16();
            pagePointers.at(pagePointersPos) = bufferID;
            pagePointersPos++;
        }
        else
        {
            // primary key
            auto record = buffer.ReadRecord(this->keyColumns);
            indexList.at(indexListPos) = RecordToIndex(this->keyColumns, record);
            indexListPos++;
            size++;
        }
    }
}

NonLeafPage::NonLeafPage(u16 pageID, const vector<DBColumn> &keyColumns, vector<Index> indexList,
                         vector<u16> pagePointers, size_t order, size_t size)
    : pageID{pageID}, keyColumns{keyColumns}, indexList{indexList}, pagePointers{pagePointers}, order{order}, size{size}
{
    // TO DO: header
}
LeafPage::LeafPage(u16 pageID, const vector<DBColumn> &keyColumns, const PageHeader &header, Buffer &buffer,
                   size_t order)
    : pageID{pageID}, header{header}, keyColumns{keyColumns}, indexList(order), recordPointers(order),
      pagePointers(order), order{order}, size{0}
{
    size_t pagePointersPos = 0;
    size_t recordPointersPos = 0;
    size_t indexListPos = 0;
    for (size_t i = 0; i < header.recordInfoArray.size(); i++)
    {
        const auto &recordInfo = header.recordInfoArray.at(i);
        buffer.pos = recordInfo.location;
        if (i == 0 || (i + 1) == header.recordInfoArray.size())
        {
            // page pointer
            u16 bufferID = buffer.ReadU16();
            pagePointers.at(pagePointersPos) = bufferID;
            pagePointersPos++;
        }
        else if (i % 2 == 1)
        {
            // record pointer
            u16 bufferID = buffer.ReadU16();
            u16 posIndex = buffer.ReadU16();
            recordPointers.at(recordPointersPos) = RecordPointer(bufferID, posIndex);
            recordPointersPos++;
        }
        else
        {
            // primary key
            auto record = buffer.ReadRecord(this->keyColumns);
            indexList.at(indexListPos) = RecordToIndex(this->keyColumns, record);
            indexListPos++;
            size++;
        }
    }
}

LeafPage::LeafPage(u16 pageID, const vector<DBColumn> &keyColumns, vector<Index> indexList,
                   vector<RecordPointer> recordPointers, vector<u16> pagePointers, size_t order, size_t size)
    : pageID{pageID}, keyColumns{keyColumns}, indexList{indexList}, recordPointers{recordPointers},
      pagePointers{pagePointers}, order{order}, size{size}
{
    // TO DO: header
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

bool Search(BufferManager &bufferManager, Buffer &buffer, shared_ptr<IndexPage> root,
            const Index &index)
{
    if (root)
    {
        shared_ptr<IndexPage> cursor = root;
        while (cursor->IsLeaf() == false)
        {
            for (size_t i = 0; i < cursor->Size(); i++)
            {
                if (CompareIndex(index, cursor->IndexList().at(i)) < 0)
                {
                    u16 pagePointer = cursor->PagePointers().at(i);
                    cursor = LoadIndexPage(bufferManager, buffer, pagePointer, cursor->KeyColumns());
                    break;
                }
                if (i + 1 == cursor->Size())
                {
                    u16 pagePointer = cursor->PagePointers().at(i + 1);
                    cursor = LoadIndexPage(bufferManager, buffer, pagePointer, cursor->KeyColumns());
                    break;
                }
            }
        }

        for (size_t i = 0; i < cursor->Size(); i++)
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
<<<<<<< Updated upstream
            for (size_t i = 0; i < cursor->IndexList().size(); i++)
=======
            for (size_t i = 0; i < cursor->Size(); i++)
>>>>>>> Stashed changes
            {
                // find the position to insert the node
                if (CompareIndex(index, cursor->IndexList().at(i)) < 0)
                {
                    u16 pagePointer = cursor->PagePointers().at(i);
                    cursor = LoadIndexPage(bufferManager, buffer, pagePointer, cursor->KeyColumns());
                    break;
                }

                // if the searching reaches the end
                if (i + 1 == cursor->Size())
                {
                    u16 pagePointer = cursor->PagePointers().at(i + 1);
                    cursor = LoadIndexPage(bufferManager, buffer, pagePointer, cursor->KeyColumns());
                    break;
                }
            }
        }

        if (cursor->Size() < order)
        {
            size_t i = 0;
            // move to the position to insert the node
            while (CompareIndex(index, cursor->IndexList().at(i)) > 0 && i < cursor->Size())
            {
                i++;
            }
            auto &recordPointers = static_pointer_cast<LeafPage>(cursor)->recordPointers;
            for (size_t j = cursor->Size(); j != i; j--)
            {
                cursor->IndexList().at(j) = cursor->IndexList().at(j - 1);
                recordPointers.at(j) = recordPointers.at(j - 1);
            }
            cursor->IndexList().at(i) = index;
            recordPointers.at(i) = recordPointer;
            cursor->Resize(cursor->Size() + 1);
            size_t n = cursor->Size();
            cursor->PagePointers().at(n) = cursor->PagePointers().at(n - 1);
            cursor->PagePointers().at(n - 1) = 0;
        }
        else
        {
            u16 pageID = bufferManager.AllocatePage();
            auto newLeaf = make_shared<LeafPage>(pageID, keyColumns, vector<Index>(order), vector<RecordPointer>(order),
                                                 vector<u16>(order), order, 1);
            newLeaf->IndexList().at(0) = index;
            newLeaf->recordPointers.at(0) = recordPointer;
            vector<Index> virtualNode = cursor->IndexList();
            size_t i = 0;
            while (CompareIndex(index, virtualNode.at(i)) && i < order)
            {
                i++;
            }

            for (size_t j = order; j != i; j--)
            {
                virtualNode.at(j) = virtualNode.at(j - 1);
            }
            virtualNode.at(i) = index;
            cursor->Resize((order + 1) / 2);
            newLeaf->Resize(order + 1 - (order + 1) / 2);
            cursor->PagePointers().at(cursor->Size()) = pageID;
            newLeaf->PagePointers().at(newLeaf->Size()) = cursor->PagePointers().at(order);
            cursor->PagePointers().at(order) = 0;
            for (size_t i = 0; i < cursor->Size(); i++)
            {
                cursor->IndexList().at(i) = virtualNode.at(i);
            }
            for (size_t i = 0, j = cursor->Size(); i < newLeaf->Size(); i++, j++)
            {
                newLeaf->IndexList().at(i) = virtualNode.at(j);
            }

            if (cursor->PageID() == root->PageID())
            {
                u16 pageID = bufferManager.AllocatePage();
                auto newRoot = make_shared<NonLeafPage>(pageID, keyColumns, vector<Index>(order),
                                                        vector<RecordPointer>(order), vector<u16>(order), order, 1);
                newRoot->IndexList().at(0) = newLeaf->IndexList().at(0);
                newRoot->PagePointers().at(0) = cursor->PageID();
                newRoot->PagePointers().at(1) = newLeaf->PageID();
                root = newRoot;
            }
            else
            {
                // TO DO
            }
        }
    }
    else
    {
        u16 pageID = bufferManager.AllocatePage();
        root = make_shared<LeafPage>(pageID, keyColumns, vector<Index>{index}, vector<RecordPointer>{recordPointer},
                                     vector<u16>{});
    }
    return shared_ptr<LeafPage>();
}

void InsertInternal(BufferManager &bufferManager, size_t order, const Index &index, shared_ptr<IndexPage> cursor,
                    shared_ptr<IndexPage> child)
{
    if (cursor->Size() < order)
    {
        size_t i = 0;
        while (CompareIndex(index, cursor->IndexList().at(i)) > 0 && i < cursor->Size())
        {
            i++;
        }
        for (size_t j = cursor->Size(); j > i; j--)
        {
            cursor->IndexList().at(j) = cursor->IndexList().at(j - 1);
        }
        for (size_t j = cursor->Size() + 1; j > i + 1; j--)
        {
            cursor->PagePointers().at(j) = cursor->PagePointers().at(j - 1);
        }
        cursor->IndexList().at(i) = index;
        cursor->Resize(cursor->Size() + 1);
        cursor->PagePointers().at(i + 1) = child->PageID();
    }
    else
    {
        vector<Index> virtualKeys(order + 1);
        vector<u16> virtualPtrs(order + 2);
        for (size_t k = 0; k < order; k++)
        {
            virtualKeys.at(k) = cursor->IndexList().at(k);
        }
        for (size_t k = 0; k < order + 1; k++)
        {
            virtualPtrs.at(k) = cursor->PagePointers().at(k);
        }
        size_t i = 0;
        while (CompareIndex(index, virtualKeys.at(i)) > 0 && i < order)
        {
            i++;
        }
<<<<<<< Updated upstream
        for (size_t k = order; k > i; k--)
=======
        for (size_t k = order; k != i; k--)
>>>>>>> Stashed changes
        {
            virtualKeys.at(k) = virtualKeys.at(k - 1);
        }
        virtualKeys.at(i) = index;
<<<<<<< Updated upstream
        for (size_t k = order + 1; k > i + 1; k--)
=======
        for (size_t k = order + 1; k != i + 1; k--)
>>>>>>> Stashed changes
        {
            virtualPtrs.at(k) = virtualPtrs.at(k - 1);
        }
        virtualPtrs.at(i + 1) = child->PageID();
<<<<<<< Updated upstream
        // NonLeafPage *newPage;
=======
        u16 pageID = bufferManager.AllocatePage();
        auto newInternal = make_shared<NonLeafPage>(pageID, keyColumns, vector<Index>(order),
                                                    vector<RecordPointer>(order), vector<u16>(order), order, 1);
        cursor->Resize((order + 1) / 2);
        newInternal->Resize(order - (order + 1) / 2);
>>>>>>> Stashed changes
        // TO DO
    }
}
} // namespace bplustree
