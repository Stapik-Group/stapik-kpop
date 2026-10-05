#include "PartialDate.hpp"

#include <charconv>
#include <chrono>
#include <format>

namespace kpop::domain
{
    namespace
    {
        constexpr int MIN_YEAR = 1;
        constexpr int MAX_YEAR = 9999;
        constexpr std::size_t YEAR_DIGITS = 4;
        constexpr std::size_t MAX_MONTH_OR_DAY_DIGITS = 2;

        std::optional<int> parseNumber(const std::string_view text, const std::size_t minDigits, const std::size_t maxDigits)
        {
            if (text.size() < minDigits || text.size() > maxDigits)
                return std::nullopt;

            int value = 0;
            const auto* const end = text.data() + text.size();
            if (const auto [parsedEnd, errorCode] = std::from_chars(text.data(), end, value); errorCode != std::errc{} || parsedEnd != end)
                return std::nullopt;

            return value;
        }
    }

    PartialDate::PartialDate(const int year, const std::optional<int> month, const std::optional<int> day) :
        m_year(year),
        m_month(month),
        m_day(day)
    {}

    std::optional<PartialDate> PartialDate::tryCreate(const int year, const std::optional<int> month, const std::optional<int> day)
    {
        if (year < MIN_YEAR || year > MAX_YEAR)
            return std::nullopt;

        if (day && !month)
            return std::nullopt;

        if (month && (*month < 1 || *month > 12))
            return std::nullopt;

        if (day)
        {
            if (*day < 1)
                return std::nullopt;

            const std::chrono::year_month_day date{
                std::chrono::year{ year },
                std::chrono::month{ static_cast<unsigned>(*month) },
                std::chrono::day{ static_cast<unsigned>(*day) } };

            if (!date.ok())
                return std::nullopt;
        }

        return PartialDate(year, month, day);
    }

    std::optional<PartialDate> PartialDate::parse(const std::string_view text)
    {
        const auto firstDash = text.find('-');
        const auto year = parseNumber(text.substr(0, firstDash), YEAR_DIGITS, YEAR_DIGITS);
        if (!year)
            return std::nullopt;

        if (firstDash == std::string_view::npos)
            return tryCreate(*year);

        const auto afterYear = text.substr(firstDash + 1);
        const auto secondDash = afterYear.find('-');
        const auto month = parseNumber(afterYear.substr(0, secondDash), 1, MAX_MONTH_OR_DAY_DIGITS);
        if (!month)
            return std::nullopt;

        if (secondDash == std::string_view::npos)
            return tryCreate(*year, month);

        const auto day = parseNumber(afterYear.substr(secondDash + 1), 1, MAX_MONTH_OR_DAY_DIGITS);
        if (!day)
            return std::nullopt;

        return tryCreate(*year, month, day);
    }

    int PartialDate::year() const
    {
        return m_year;
    }

    std::optional<int> PartialDate::month() const
    {
        return m_month;
    }

    std::optional<int> PartialDate::day() const
    {
        return m_day;
    }

    std::string PartialDate::toKey() const
    {
        if (m_month && m_day)
            return std::format("{:04}-{:02}-{:02}", m_year, *m_month, *m_day);

        if (m_month)
            return std::format("{:04}-{:02}", m_year, *m_month);

        return std::format("{:04}", m_year);
    }
}
