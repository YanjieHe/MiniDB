#ifndef JSON_SERIALIZER_HPP
#define JSON_SERIALIZER_HPP

#include <nlohmann/json.hpp>

#include "DBColumn.hpp"
#include "Page.hpp"
#include "BPlusTree.hpp"

using Json = nlohmann::json;

class JsonSerializer {
 public:
  static Json PageHeaderToJson(const PageHeader &header);
  static Json DBRowToJson(const DBRow &record);
  static Json DBColumnToJson(const DBColumn &column);
  static Json PageInfoToJson(const IPage& page);
  static Json PageToJson(const Page& page, Buffer& buffer);
  static Json BPlusTreePageToJson(const IndexPage& page, Buffer& buffer);
  static Json BPlusTreeIndexToJson(const DBIndex& index);
  static Json DatabaseHeaderToJson(const DatabaseHeader& header);
};

#endif /* JSON_SERIALIZER_HPP */