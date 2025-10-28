#pragma once

#include <xhash>

namespace Titan
{

    class UUID
    {
    public:
        UUID();
        UUID(uint64_t uuid);
        UUID(const UUID&) = default;

        operator uint64_t() const { return m_UUID; }

    private:
        uint64_t m_UUID;
    };

} // namespace Titan

namespace std
{

    template <>
    struct hash<Titan::UUID>
    {
        std::size_t operator()(const Titan::UUID& uuid) const { return hash<uint64_t>()((uint64_t)uuid); }
    };

} // namespace std