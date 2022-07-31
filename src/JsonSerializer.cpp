#include "JsonSerializer.hpp"

#include <magic_enum/magic_enum.hpp>

using std::holds_alternative;

Json JsonSerializer::PageHeaderToJson(const PageHeader& header) {
  vector<Json> dbRowInfoArray;
  for (auto info : header.recordInfoArray) {
    dbRowInfoArray.push_back(
        {{"location", info.location}, {"size", info.size}});
  }
  return {{"pageType", magic_enum::enum_name(header.pageType)},
          {"numOfEntries", header.numOfEntries},
          {"endOfFreeSpace", header.endOfFreeSpace},
          {"recordInfoArray", dbRowInfoArray}};
}

Json JsonSerializer::DBRowToJson(const DBRow& record) {
  Json obj;
  for (auto value : record.values) {
    if (holds_alternative<monostate>(value)) {
      obj.push_back(nullptr);
    } else if (holds_alternative<i64>(value)) {
      obj.push_back(std::get<i64>(value));
    } else if (holds_alternative<f64>(value)) {
      obj.push_back(std::get<f64>(value));
    } else {
      obj.push_back(std::get<string>(value));
    }
  }
  return obj;
}

Json JsonSerializer::DBColumnToJson(const DBColumn& column) {
  return {{"name", column.name},
          {"type", magic_enum::enum_name(column.type)},
          {"nullable", column.nullable}};
}

Json JsonSerializer::PageInfoToJson(const IPage& page) {
  Json headerJson = PageHeaderToJson(page.Header());
  vector<Json> columnsJson(page.Columns().size());
  for (size_t i = 0; i < page.Columns().size(); i++) {
    columnsJson.at(i) = DBColumnToJson(page.Columns().at(i));
  }
  return {{"header", headerJson}, {"columns", columnsJson}};
}

Json JsonSerializer::PageToJson(const Page& page, Buffer& buffer) {
  Json pageJson = PageInfoToJson(page);
  vector<Json> rowsJson(page.header.numOfEntries);
  for (size_t i = 0; i < page.header.numOfEntries; i++) {
    DBRow row = page.GetRow(buffer, i);
    rowsJson.at(i) = DBRowToJson(row);
  }
  pageJson["rows"] = rowsJson;
  return pageJson;
}

Json JsonSerializer::BPlusTreePageToJson(const IndexPage& page,
                                         Buffer& buffer) {
  Json pageJson = PageInfoToJson(page);
  vector<Json> rowsJson;
  vector<DBIndex> indices(page.header.numOfEntries / 2);
  vector<u16> pointers((page.header.numOfEntries / 2) + 1);
  page.LoadAllIndices(buffer, indices);
  page.LoadAllPointers(buffer, pointers);
  for (int i = 0; i < page.header.numOfEntries; i++) {
    if (i % 2 == 0) {
      rowsJson.push_back(pointers.at(i / 2));
    } else {
      rowsJson.push_back(BPlusTreeIndexToJson(indices.at(i / 2)));
    }
  }
  pageJson["rows"] = rowsJson;
  return pageJson;
}

Json JsonSerializer::BPlusTreeIndexToJson(const DBIndex& index) {
  Json indexJson;
  vector<Json> keysJson;
  for (const DBIndex::Key& key : index.keys) {
    if (std::holds_alternative<i64>(key)) {
      keysJson.push_back(std::get<i64>(key));
    } else if (std::holds_alternative<string>(key)) {
      keysJson.push_back(std::get<string>(key));
    } else {
      throw DBException(string(__FUNCTION__) +
                        ": the index key is not a supported type");
    }
  }
  indexJson["keys"] = keysJson;
  return indexJson;
}