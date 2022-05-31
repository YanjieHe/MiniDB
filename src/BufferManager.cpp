#include "BufferManager.hpp"

#include <fstream>

#include "DBException.hpp"

using std::fstream;
using std::ifstream;
using std::ofstream;

BufferManager::BufferManager(string path, DatabaseHeader header)
    : path{path}, header{header} {
  ifstream stream(path, std::ios::binary);
  if (stream) {
    Buffer buffer(sizeof(i64));
    header.pageSize = static_cast<size_t>(buffer.ReadI64());
  } else {
    throw DBException("Fail to load the database file: " + path);
  }
}

void BufferManager::LoadBuffer(u16 bufferID, Buffer &buffer) {
  if (bufferID < header.nPages) {
    ifstream stream(path, std::ios::binary);
    stream.seekg(PageStart(bufferID));
    stream.read(reinterpret_cast<char *>(buffer.bytes.data()),
                buffer.bytes.size());
    buffer.pos = 0;
  } else {
    throw DBException("try to access an unallocated page");
  }
}

void BufferManager::SaveBuffer(u16 bufferID, Buffer &buffer) {
  if (bufferID < header.nPages) {
    fstream stream(path, fstream::in | fstream::out | fstream::binary);
    stream.seekp(PageStart(bufferID));
    stream.write(reinterpret_cast<char *>(buffer.bytes.data()),
                 buffer.bytes.size());
  } else {
    throw DBException("the current page hasn't been allocated yet");
  }
}

u16 BufferManager::AllocatePage() {
  ofstream stream(path, std::ios::binary);
  stream.seekp(header.pageSize, std::ios::end);
  stream.write("", 0);
  header.nPages++;
  return header.nPages - 1;
}

size_t BufferManager::PageStart(u16 bufferID) const {
  return header.pageSize * bufferID + header.ByteSize();
}

void CreateEmptyDatabaseFile(string path, const DatabaseHeader &header) {
  ofstream stream(path, std::ios::binary);
  Buffer buffer(header.ByteSize());
  buffer.WriteI64(header.pageSize);
  buffer.WriteU16(header.nPages);
  stream.write(reinterpret_cast<char *>(buffer.bytes.data()),
               buffer.bytes.size());
}