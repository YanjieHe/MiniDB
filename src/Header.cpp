#include "Header.hpp"

PageHeader EmptyTablePageHeader(size_t pageSize) {
  return PageHeader(PageType::TABLE, 0, static_cast<u16>(pageSize - 1), {});
}

PageHeader EmptyIndexPageHeader(size_t pageSize, PageType pageType) {
  return PageHeader(pageType, 0, static_cast<u16>(pageSize - 1), {});
}