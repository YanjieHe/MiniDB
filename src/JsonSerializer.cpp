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

Json JsonSerializer::PageInfoToJson(const Page& page) {
  Json headerJson = PageHeaderToJson(page.header);
  vector<Json> columnsJson(page.columns.size());
  for (size_t i = 0; i < page.columns.size(); i++) {
    columnsJson.at(i) = DBColumnToJson(page.columns.at(i));
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