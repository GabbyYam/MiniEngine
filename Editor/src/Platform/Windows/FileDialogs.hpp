#pragma once
#include <string>

namespace suplex {
    class FileDialogs {
    public:
        [[nodiscard("")]] static std::string OpenFile(const char* filter);
        [[nodiscard("")]] static std::string SaveFile(const char* filter);
    };
}  // namespace suplex