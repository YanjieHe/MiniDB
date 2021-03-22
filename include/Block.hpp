#ifndef BLOCK_HPP
#define BLOCK_HPP
#include "DBColumn.hpp"
#include "DBRow.hpp"
#include <fstream>


using std::ifstream;
using std::ofstream;

class Block {
public:
  vector<u8> bytes;
  size_t pos;

  explicit Block(size_t pageSize) : bytes(pageSize), pos{0} {}
  u8 ReadU8();
  u16 ReadU16();
  i64 ReadI64();
  f64 ReadF64();
  void WriteU8(u8 u);
  void WriteU16(u16 u);
  void WriteI64(i64 i);
  void WriteF64(f64 f);
  void WriteText(const std::string s);
  DBRow ReadRecord(vector<DBColumn> &columns);
  void SaveToFile(ofstream &stream);
  void LoadFromFile(ifstream &stream);
};

void WriteBlock(ofstream &stream, Block &block);
void LoadBlockAtCurrentPos(ifstream &stream, Block &block);
void LoadHeader(Block &block, Header &header);
void LoadBlock(ifstream& stream, u16 blockIndex, size_t pageSize, Block& block);
#endif // BLOCK_HPP