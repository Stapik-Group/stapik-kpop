#pragma once

#include <compare>
#include <optional>
#include <string>
#include <string_view>

namespace kpop::domain
{
    class PartialDate
    {
    public:
        [[nodiscard]] static std::optional<PartialDate> tryCreate(int year, std::optional<int> month = std::nullopt, std::optional<int> day = std::nullopt);

        // Accepts "YYYY", "YYYY-MM" and "YYYY-MM-DD".
        [[nodiscard]] static std::optional<PartialDate> parse(std::string_view text);

        [[nodiscard]] int year() const;
        [[nodiscard]] std::optional<int> month() const;
        [[nodiscard]] std::optional<int> day() const;

        [[nodiscard]] std::string toKey() const;

        [[nodiscard]] std::strong_ordering operator<=>(const PartialDate& other) const = default;
        [[nodiscard]] bool operator==(const PartialDate& other) const = default;

    private:
        PartialDate(int year, std::optional<int> month, std::optional<int> day);

        int m_year;
        std::optional<int> m_month;
        std::optional<int> m_day;
    };
}
