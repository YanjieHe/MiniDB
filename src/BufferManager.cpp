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

void BufferManager::LoadPage(u16 pageId, Buffer &buffer) {
  if (pageId < header.nPages) {
    /* the page exists */
    ifstream stream(ExtentFilePath(pageId), std::ios::binary);
    stream.seekg((pageId % N_PAGES_IN_AN_EXTENT) * PAGE_SIZE, std::ios::beg);
    stream.read(reinterpret_cast<char *>(buffer.bytes.data()),
                buffer.bytes.size());
    buffer.pos = 0;
  } else {
    throw DBException("try to access an unallocated page. Page ID: " +
                      std::to_string(pageId));
  }
}

void BufferManager::SavePage(u16 pageId, Buffer &buffer) {
  if (pageId < header.nPages) {
    size_t offset = (pageId % N_PAGES_IN_AN_EXTENT) * PAGE_SIZE;

    ifstream reader(ExtentFilePath(pageId), std::ios::binary);
    vector<u8> entireFile(EXTENT_SIZE);
    reader.read(reinterpret_cast<char *>(entireFile.data()), offset);
    reader.seekg(offset + PAGE_SIZE);
    reader.read(reinterpret_cast<char*>(entireFile.data()+offset+PAGE_SIZE), EXTENT_SIZE - (offset + PAGE_SIZE));
    reader.close();

    for (size_t i = 0; i < buffer.bytes.size(); i++) {
      entireFile.at(i + offset) = buffer.bytes.at(i);
    }

    ofstream writer(ExtentFilePath(pageId), std::ios::binary);
    writer.write(reinterpret_cast<char *>(entireFile.data()),
                 entireFile.size());
    writer.close();
  } else {
    throw DBException("the current page " + std::to_string(pageId) +
                      " hasn't been allocated yet");
  }
}

u16 BufferManager::AllocatePage() {
  size_t pageId = header.nPages;
  if (pageId % N_PAGES_IN_AN_EXTENT == 0) {
    /* create a new file */
    ofstream stream(ExtentFilePath(pageId), std::ios_base::binary);
    vector<u8> bytes(EXTENT_SIZE);
    std::fill(bytes.begin(), bytes.end(), 0);
    stream.write(reinterpret_cast<char *>(bytes.data()), bytes.size());
  } else {
    /* use existing extent */
  }
  header.nPages++;
  return pageId;
}

void BufferManager::UpdateDatabaseHeaderToDisk() {
  ofstream stream(directoryPath + "/" + DATABASE_HEADER_FILENAME);
  stream << std::setw(4) << JsonSerializer::DatabaseHeaderToJson(header);
}

void BufferManager::LoadDatabaseHeaderFromDisk() {
  ifstream stream(directoryPath + "/" + DATABASE_HEADER_FILENAME);
  Json headerJson = Json::parse(stream);
  JsonSerializer::JsonToDatabaseHeader(headerJson, header);
}

string BufferManager::ExtentFilePath(u16 pageId) const {
  size_t extentId = pageId / N_PAGES_IN_AN_EXTENT;
  return directoryPath + "/" + std::to_string(extentId) + ".bin";
}