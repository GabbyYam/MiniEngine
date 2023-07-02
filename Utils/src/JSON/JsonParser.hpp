#pragma once

#include <unordered_map>
#include <variant>
#include <vector>
namespace suplex {
    struct JsonObject
    {
        std::variant<std::monostate,
                     bool,
                     int,
                     float,
                     double,
                     std::string,
                     std::vector<JsonObject>,
                     std::unordered_map<std::string, JsonObject>>
            inner;
    };
}  // namespace suplex