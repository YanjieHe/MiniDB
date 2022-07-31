#include <nlohmann/json.hpp>

#include "Buffer.hpp"
#include "DBColumn.hpp"
#include "DBIndex.hpp"
#include "DBRow.hpp"

using nlohmann::json;

const size_t PAGE_SIZE = 65536;

void PrintFormattedJson(json jsonObj);

DBRow::Value DBString(string s);

DBRow::Value DBInt64(i64 i);

DBIndex::Key StringKey(string s);

DBIndex::Key Int64Key(i64 i);

vector<DBColumn> BookDataColumns();

vector<DBColumn> BookIndexColumn();

vector<DBRow> BookRows();

Buffer CreateBookPage();