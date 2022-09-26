#include "BufferManager.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>

#include "DBException.hpp"
#include "JsonSerializer.hpp"

using std::fstream;
using std::ifstream;
using std::ofstream;
namespace fs = std::filesystem;

BufferManager::BufferManager(string directoryPath, DatabaseHeader header)
    : directoryPath{directoryPath}, header{header} {
  if (!fs::exists(directoryPath)) {
    fs::create_directory(directoryPath);
  }
  UpdateDatabaseHeaderToDisk();
}

void BufferManager::LoadBuffer(u16 bufferID, Buffer &buffer) {
  if (bufferID < header.nPages) {
    ifstream stream(BufferFilePath(bufferID), std::ios::binary);
    // stream.seekg(PageStart(bufferID));
    // cout << "[load buffer] page start: " << PageStart(bufferID) << endl;
    stream.read(reinterpret_cast<char *>(buffer.bytes.data()),
                buffer.bytes.size());
    buffer.pos = 0;
  } else {
    throw DBException("try to access an unallocated page " +
                      std::to_string(bufferID));
  }
}

void BufferManager::SaveBuffer(u16 bufferID, Buffer &buffer) {
  if (bufferID < header.nPages) {
    ofstream stream(BufferFilePath(bufferID), std::ios::binary);
    // stream.seekp(PageStart(bufferID), std::ios::beg);
    // cout << "[save buffer] page start: " << PageStart(bufferID) << endl;
    stream.write(reinterpret_cast<char *>(buffer.bytes.data()),
                 buffer.bytes.size());
  } else {
    throw DBException("the current page " + std::to_string(bufferID) +
                      " hasn't been allocated yet");
  }
}

u16 BufferManager::AllocatePage() {
  ofstream stream(BufferFilePath(header.nPages), std::ios_base::binary);
  // cout << "[allocate page] file (" << directoryPath << ") "
  //      << "header.pageSize = " << header.pageSize << endl;
  // stream.seekp(header.pageSize, std::ios::beg);
  // stream.write("", 0);
  vector<u8> bytes(header.pageSize);
  std::fill(bytes.begin(), bytes.end(), 0);
  stream.write(reinterpret_cast<char *>(bytes.data()), bytes.size());
  header.nPages++;
  return header.nPages - 1;
}

size_t BufferManager::PageStart(u16 bufferID) const {
  return header.pageSize * bufferID + header.ByteSize();
}

void BufferManager::UpdateDatabaseHeaderToDisk() {
  ofstream stream(directoryPath + "/" + DATABASE_HEADER_FILENAME);
  stream << std::setw(4) << JsonSerializer::DatabaseHeaderToJson(header);
}

string BufferManager::BufferFilePath(u16 bufferID) const {
  return directoryPath + "/" + std::to_string(bufferID) + ".bin";
}