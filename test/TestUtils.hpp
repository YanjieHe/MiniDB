#include <nlohmann/json.hpp>
#include "DBRow.hpp"

using nlohmann::json;

void PrintFormattedJson(json jsonObj);

DBRow::Value DBString(string s);

DBRow::Value DBInt64(i64 i);