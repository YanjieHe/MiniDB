#ifndef BUFFER_MANAGER_HPP
#define BUFFER_MANAGER_HPP
#include "Buffer.hpp"

class BufferManager {
public:
  string path;
  explicit BufferManager(string path) : path{path} {}

  void LoadBuffer(u16 bufferID, size_t pageSize, Buffer &buffer);
  void SaveBuffer(u16 bufferID, size_t pageSize, Buffer &buffer);
};

#endif // BUFFER_MANAGER_HPP