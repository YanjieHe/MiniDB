#ifndef BUFFER_MANAGER_HPP
#define BUFFER_MANAGER_HPP
#include "Buffer.hpp"

#define DATABASE_HEADER_FILENAME "DATABASE_HEADER.json"

class BufferManager {
 public:
  string directoryPath;
  DatabaseHeader header;

  BufferManager(string directoryPath, DatabaseHeader header);

  void LoadBuffer(u16 bufferID, Buffer &buffer);
  void SaveBuffer(u16 bufferID, Buffer &buffer);
  u16 AllocatePage();
  size_t PageStart(u16 bufferID) const;
  void UpdateDatabaseHeaderToDisk();

 private:
  string BufferFilePath(u16 bufferID) const;
};

#endif  // BUFFER_MANAGER_HPP