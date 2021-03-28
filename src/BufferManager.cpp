#include "BufferManager.hpp"
#include <fstream>

using std::ifstream;
using std::ofstream;

void BufferManager::LoadBuffer(u16 bufferID, size_t pageSize, Buffer &buffer) {
  ifstream stream(path, std::ios::binary);
  stream.seekg(pageSize * bufferID);
  stream.read(reinterpret_cast<char *>(buffer.bytes.data()),
              buffer.bytes.size());
}

void BufferManager::SaveBuffer(u16 bufferID, size_t pageSize, Buffer &buffer) {
  ofstream stream(path, std::ios::binary);
  stream.seekp(pageSize * bufferID);
  stream.write(reinterpret_cast<char *>(buffer.bytes.data()),
               buffer.bytes.size());
}