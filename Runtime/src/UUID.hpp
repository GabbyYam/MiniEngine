#pragma once

#include <stdint.h>
namespace suplex {
    class UUID {
    public:
        UUID();
        UUID(uint64_t uuid);
        UUID(const UUID&) = default;

        operator uint64_t() const { return m_UUID; }

    private:
        uint64_t m_UUID;
    };

}  // namespace suplex

namespace std {
    template <typename T>
    struct hash;

    template <>
    struct hash<suplex::UUID>
    {
        size_t operator()(const suplex::UUID& uuid) const { return (uint64_t)uuid; }
    };
}  // namespace std