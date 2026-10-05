#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

namespace kpop::domain
{
    template<typename EnumType>
    struct EnumEntry
    {
        EnumType value;
        std::string_view id;
    };

    template<typename EnumType>
    [[nodiscard]] constexpr EnumEntry<EnumType> entry(const EnumType value, const std::string_view id)
    {
        return EnumEntry<EnumType>{ .value = value, .id = id };
    }

    template<typename EnumType, std::size_t EntryCount>
    class EnumCatalog
    {
    public:
        constexpr EnumCatalog(const std::string_view keyPrefix, const std::array<EnumEntry<EnumType>, EntryCount> entries) :
            m_keyPrefix(keyPrefix),
            m_entries(entries)
        {}

        [[nodiscard]] constexpr std::span<const EnumEntry<EnumType>> entries() const
        {
            return m_entries;
        }

        [[nodiscard]] constexpr std::string_view idOf(const EnumType value) const
        {
            for (const auto& candidate : m_entries)
            {
                if (candidate.value == value)
                    return candidate.id;
            }

            return {};
        }

        [[nodiscard]] constexpr std::optional<EnumType> fromId(const std::string_view id) const
        {
            for (const auto& candidate : m_entries)
            {
                if (candidate.id == id)
                    return candidate.value;
            }

            return std::nullopt;
        }

        [[nodiscard]] std::string nameKey(const EnumType value) const
        {
            return std::string(m_keyPrefix) + "." + std::string(idOf(value));
        }

    private:
        std::string_view m_keyPrefix;
        std::array<EnumEntry<EnumType>, EntryCount> m_entries;
    };

    template<typename EnumType, typename... RemainingEntries>
    [[nodiscard]] constexpr auto makeEnumCatalog(
        const std::string_view keyPrefix,
        const EnumEntry<EnumType> firstEntry,
        const RemainingEntries... remainingEntries)
    {
        static_assert((std::is_same_v<RemainingEntries, EnumEntry<EnumType>> && ...), "All entries must belong to the same enumeration");

        constexpr std::size_t entryCount = 1 + sizeof...(RemainingEntries);
        return EnumCatalog<EnumType, entryCount>(
            keyPrefix,
            std::array<EnumEntry<EnumType>, entryCount>{ firstEntry, remainingEntries... });
    }
}
