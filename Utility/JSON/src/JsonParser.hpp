#pragma once

#include <unordered_map>
#include <variant>
#include <vector>
namespace suplex {
    struct JsonEntity
    {
        std::variant<std::monostate,
                     bool,
                     int,
                     float,
                     double,
                     std::string,
                     std::vector<JsonEntity>,
                     std::unordered_map<std::string, JsonEntity>>
            inner;
    };
}  // namespace suplex