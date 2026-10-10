#include "PartialDateFormatter.hpp"

#include <algorithm>
#include <format>
#include <vector>

namespace kpop::domain
{
    namespace
    {
        enum class Order
        {
            DayMonthYear,
            MonthDayYear,
            YearMonthDay
        };

        struct Layout
        {
            Order order;
            char separator;
        };

        constexpr std::size_t LANGUAGE_PREFIX_LENGTH = 2;

        Layout layoutOf(const std::string_view languageCode)
        {
            const auto language = languageCode.substr(0, LANGUAGE_PREFIX_LENGTH);

            if (language == "pl" || language == "de")
                return { .order = Order::DayMonthYear, .separator = '.' };

            if (language == "en")
                return { .order = Order::MonthDayYear, .separator = '/' };

            return { .order = Order::YearMonthDay, .separator = '-' };
        }

        std::vector<std::string_view> split(const std::string_view text, const char separator)
        {
            std::vector<std::string_view> parts;

            std::size_t start = 0;
            while (true)
            {
                const auto end = text.find(separator, start);
                if (end == std::string_view::npos)
                {
                    parts.push_back(text.substr(start));
                    return parts;
                }

                parts.push_back(text.substr(start, end - start));
                start = end + 1;
            }
        }

        bool isNumber(const std::string_view text)
        {
            return !text.empty() && std::ranges::all_of(text, [](const char character) { return character >= '0' && character <= '9'; });
        }
    }

    std::string formatPartialDate(const PartialDate& date, const std::string_view languageCode)
    {
        const auto layout = layoutOf(languageCode);
        if (layout.order == Order::YearMonthDay)
            return date.toKey();

        if (!date.month())
            return std::format("{:04}", date.year());

        if (!date.day())
            return std::format("{:02}{}{:04}", *date.month(), layout.separator, date.year());

        const bool dayFirst = layout.order == Order::DayMonthYear;
        const int first = dayFirst ? *date.day() : *date.month();
        const int second = dayFirst ? *date.month() : *date.day();
        return std::format("{:02}{}{:02}{}{:04}", first, layout.separator, second, layout.separator, date.year());
    }

    std::optional<PartialDate> parsePartialDate(const std::string_view text, const std::string_view languageCode)
    {
        if (const auto iso = PartialDate::parse(text))
            return iso;

        const auto layout = layoutOf(languageCode);
        if (layout.order == Order::YearMonthDay)
            return std::nullopt;

        const auto parts = split(text, layout.separator);
        if (!std::ranges::all_of(parts, isNumber))
            return std::nullopt;

        if (parts.size() == 2)
            return PartialDate::parse(std::format("{}-{}", parts[1], parts[0]));

        if (parts.size() == 3)
        {
            const bool dayFirst = layout.order == Order::DayMonthYear;
            const auto day = dayFirst ? parts[0] : parts[1];
            const auto month = dayFirst ? parts[1] : parts[0];
            return PartialDate::parse(std::format("{}-{}-{}", parts[2], month, day));
        }

        return std::nullopt;
    }
}
