#ifndef INDEX_HPP
#define INDEX_HPP
#include "Buffer.hpp"
#include "BufferManager.hpp"
#include "DBColumn.hpp"
#include <memory>
#include <optional>

using std::optional;
using std::shared_ptr;
using std::variant;

class RecordPointer {
public:
  u16 bufferID;
  u16 posIndex;

  RecordPointer() = default;

  RecordPointer(u16 bufferID, u16 posIndex)
      : bufferID{bufferID}, posIndex{posIndex} {}
};

class Index {
public:
  typedef variant<i64, string> Key;

  vector<Key> keys;

  Index() = default;
  explicit Index(vector<Key> keys) : keys{keys} {}
};

<<<<<<< Updated upstream
class IndexPage {
public:
  virtual vector<u16> &PagePointers() = 0;
  virtual const vector<u16> &PagePointers() const = 0;
  virtual vector<Index> &IndexList() = 0;
  virtual const vector<Index> &IndexList() const = 0;
  virtual u16 PageID() const = 0;
  virtual bool IsLeaf() const = 0;
  virtual const vector<DBColumn> &KeyColumns() const = 0;
};

class NonLeafPage : public IndexPage {
public:
  u16 pageID;
  PageHeader header;
  vector<DBColumn> keyColumns;
  vector<Index> indexList;
  vector<u16> pagePointers;

  NonLeafPage(u16 pageID, const vector<DBColumn> &keyColumns,
              const PageHeader &header, Buffer &buffer);

  vector<u16> &PagePointers() override { return pagePointers; }

  const vector<u16> &PagePointers() const override { return pagePointers; }

  vector<Index> &IndexList() override { return indexList; }

  const vector<Index> &IndexList() const override { return indexList; }

  bool IsLeaf() const override { return false; }

  u16 PageID() const override { return pageID; }

  const vector<DBColumn> &KeyColumns() const { return keyColumns; }
=======
class IndexPage
{
  public:
    virtual vector<u16> &PagePointers() = 0;
    virtual const vector<u16> &PagePointers() const = 0;
    virtual vector<Index> &IndexList() = 0;
    virtual const vector<Index> &IndexList() const = 0;
    virtual u16 PageID() const = 0;
    virtual bool IsLeaf() const = 0;
    virtual const vector<DBColumn> &KeyColumns() const = 0;
    virtual size_t Size() const = 0;
    virtual void Resize(size_t newSize) = 0;
};

class NonLeafPage : public IndexPage
{
  public:
    u16 pageID;
    PageHeader header;
    vector<DBColumn> keyColumns;
    vector<Index> indexList;
    vector<u16> pagePointers;
    size_t order;
    size_t size;

    NonLeafPage(u16 pageID, const vector<DBColumn> &keyColumns, const PageHeader &header, Buffer &buffer, size_t order);
    NonLeafPage(u16 pageID, const vector<DBColumn> &keyColumns, vector<Index> indexList, vector<u16> pagePointers,
                size_t order, size_t size);

    vector<u16> &PagePointers() override
    {
        return pagePointers;
    }

    const vector<u16> &PagePointers() const override
    {
        return pagePointers;
    }

    vector<Index> &IndexList() override
    {
        return indexList;
    }

    const vector<Index> &IndexList() const override
    {
        return indexList;
    }

    bool IsLeaf() const override
    {
        return false;
    }

    u16 PageID() const override
    {
        return pageID;
    }

    const vector<DBColumn> &KeyColumns() const
    {
        return keyColumns;
    }

    size_t Size() const override
    {
        return size;
    }

    void Resize(size_t newSize) override
    {
        size = newSize;
    }
};

class LeafPage : public IndexPage
{
  public:
    u16 pageID;
    PageHeader header;
    vector<DBColumn> keyColumns;
    vector<Index> indexList;
    vector<RecordPointer> recordPointers;
    vector<u16> pagePointers;
    size_t order;
    size_t size;

    LeafPage(u16 pageID, const vector<DBColumn> &keyColumns, const PageHeader &header, Buffer &buffer, size_t order);

    LeafPage(u16 pageID, const vector<DBColumn> &keyColumns, vector<Index> indexList,
             vector<RecordPointer> recordPointers, vector<u16> pagePointers, size_t order, size_t size);

    vector<u16> &PagePointers() override
    {
        return pagePointers;
    }

    const vector<u16> &PagePointers() const override
    {
        return pagePointers;
    }

    vector<Index> &IndexList() override
    {
        return indexList;
    }

    const vector<Index> &IndexList() const override
    {
        return indexList;
    }

    bool IsLeaf() const override
    {
        return true;
    }

    u16 PageID() const override
    {
        return pageID;
    }

    const vector<DBColumn> &KeyColumns() const
    {
        return keyColumns;
    }

    size_t Size() const override
    {
        return size;
    }

    void Resize(size_t newSize) override
    {
        size = newSize;
    }
>>>>>>> Stashed changes
};

class LeafPage : public IndexPage {
public:
  u16 pageID;
  PageHeader header;
  vector<DBColumn> keyColumns;
  vector<Index> indexList;
  vector<RecordPointer> recordPointers;
  vector<u16> pagePointers;

  LeafPage(u16 pageID, const vector<DBColumn> &keyColumns,
           const PageHeader &header, Buffer &buffer);

  LeafPage(u16 pageID, const vector<DBColumn> &keyColumns,
           vector<Index> indexList, vector<RecordPointer> recordPointers,
           vector<u16> pagePointers);

  vector<u16> &PagePointers() override { return pagePointers; }

  const vector<u16> &PagePointers() const override { return pagePointers; }

  vector<Index> &IndexList() override { return indexList; }

  const vector<Index> &IndexList() const override { return indexList; }

  bool IsLeaf() const override { return true; }

  u16 PageID() const override { return pageID; }

  const vector<DBColumn> &KeyColumns() const { return keyColumns; }
};

Index RecordToIndex(const vector<DBColumn> &keyColumns, const DBRow &record);

namespace bplustree {
bool Search(BufferManager &bufferManager, Buffer &buffer,
            shared_ptr<IndexPage> root, const Index &index);

shared_ptr<IndexPage> LoadIndexPage(BufferManager &bufferManager,
                                    Buffer &buffer, u16 pagePointer,
                                    const vector<DBColumn> &keyColumns);

shared_ptr<IndexPage> Insert(BufferManager &bufferManager, Buffer &buffer,
                             size_t order, shared_ptr<IndexPage> &root,
                             const vector<DBColumn> &keyColumns,
                             const Index &index, RecordPointer recordPointer);

<<<<<<< Updated upstream
void InsertInternal(size_t order, const Index &index, IndexPage *cursor,
                    IndexPage *child);
=======
void InsertInternal(BufferManager& bufferManager, size_t order, const Index &index, shared_ptr<IndexPage> cursor, shared_ptr<IndexPage> child);

>>>>>>> Stashed changes
} // namespace bplustree

template <typename T> int GetComparisonIntResult(const T &x, const T &y) {
  if (x < y) {
    return -1;
  } else if (x > y) {
    return 1;
  } else {
    return 0;
  }
}

int CompareIndexKey(const Index::Key &x, const Index::Key &y);
int CompareIndex(const Index &x, const Index &y);

#endif // INDEX_HPP