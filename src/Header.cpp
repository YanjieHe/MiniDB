#include "Header.hpp"
#include <magic_enum/magic_enum.hpp>

PageHeader EmptyTablePageHeader(size_t pageSize) {
  return PageHeader(PageType::TABLE, 0, static_cast<u16>(pageSize - 1), {});
}

json PageHeaderToJson(const PageHeader &header) {
  vector<json> dbRowInfoArray;
  for (auto info : header.recordInfoArray) {
    dbRowInfoArray.push_back(
        {{"location", info.location}, {"size", info.size}});
  }
  return {{"pageType", magic_enum::enum_name(header.pageType)},
          {"numOfEntries", header.numOfEntries},
          {"endOfFreeSpace", header.endOfFreeSpace},
          {"recordInfoArray", dbRowInfoArray}};
}