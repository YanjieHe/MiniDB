#ifndef BUFFER_MANAGER_HPP
#define BUFFER_MANAGER_HPP
#include "Buffer.hpp"

#define DATABASE_HEADER_FILENAME "DATABASE_HEADER.json"

const size_t EXTENT_SIZE = 65536;
const size_t N_PAGES_IN_AN_EXTENT = 8;

class BufferManager {
public:
  string directoryPath;
  DatabaseHeader header;

  BufferManager(string directoryPath, DatabaseHeader header);

  void LoadPage(u16 pageId, Buffer &buffer);
  void SavePage(u16 pageId, Buffer &buffer);
  u16 AllocatePage();
  void UpdateDatabaseHeaderToDisk();
  void LoadDatabaseHeaderFromDisk();

private:
  string ExtentFilePath(u16 pageId) const;
};

#endif // BUFFER_MANAGER_HPP