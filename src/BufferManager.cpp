#include "BufferManager.hpp"
#include <fstream>

using std::ifstream;
using std::ofstream;

BufferManager::BufferManager(string path) : path{path}
{
    ifstream stream(path, std::ios::binary);
    Buffer buffer(header.Size());
    stream.read(reinterpret_cast<char *>(buffer.bytes.data()), buffer.bytes.size());
    header.pageSize = buffer.ReadU16();
}

void BufferManager::LoadBuffer(u16 bufferID, Buffer &buffer)
{
    ifstream stream(path, std::ios::binary);
    stream.seekg(PageStart(bufferID));
    stream.read(reinterpret_cast<char *>(buffer.bytes.data()), buffer.bytes.size());
    buffer.pos = 0;
}

void BufferManager::SaveBuffer(u16 bufferID, Buffer &buffer)
{
    ofstream stream(path, std::ios::binary);
    stream.seekp(PageStart(bufferID));
    stream.write(reinterpret_cast<char *>(buffer.bytes.data()), buffer.bytes.size());
}

u16 BufferManager::AllocatePage()
{
    ofstream stream(path, std::ios::binary);
    stream.seekp(header.pageSize, std::ios::end);
    stream.write("", 0);
    return header.pageSize;
}

size_t BufferManager::PageStart(u16 bufferID) const
{
    return header.pageSize * bufferID + header.Size();
}