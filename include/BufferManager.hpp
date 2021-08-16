#ifndef BUFFER_MANAGER_HPP
#define BUFFER_MANAGER_HPP
#include "Buffer.hpp"

class BufferManager {
public:
  string path;
  DatabaseHeader header;

  explicit BufferManager(string path);

  void LoadBuffer(u16 bufferID, Buffer &buffer);
  void SaveBuffer(u16 bufferID, Buffer &buffer);
  u16 AllocatePage();
  size_t PageStart(u16 bufferID) const;
};

void CreateEmptyDatabaseFile(string path, const DatabaseHeader& header);

#endif // BUFFER_MANAGER_HPP