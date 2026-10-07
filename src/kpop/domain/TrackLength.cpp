#include "TrackLength.hpp"

#include <array>
#include <charconv>
#include <format>

namespace kpop::domain
{
    namespace
    {
        constexpr int SECONDS_PER_MINUTE = 60;
        constexpr int SECONDS_PER_HOUR = 3600;
        constexpr int MAX_SECONDS = 99 * SECONDS_PER_HOUR + 59 * SECONDS_PER_MINUTE + 59;
        constexpr std::size_t MAX_PARTS = 3;
        constexpr std::size_t SECONDS_DIGITS = 2;

        std::optional<int> parseNumber(const std::string_view text, const std::size_t minDigits, const std::size_t maxDigits)
        {
            if (text.size() < minDigits || text.size() > maxDigits)
                return std::nullopt;

            for (const char character : text)
            {
                if (character < '0' || character > '9')
                    return std::nullopt;
            }

            int value = 0;
            const auto* const end = text.data() + text.size();
            if (const auto [parsedEnd, errorCode] = std::from_chars(text.data(), end, value); errorCode != std::errc{} || parsedEnd != end)
                return std::nullopt;

            return value;
        }
    }

    TrackLength::TrackLength(const int seconds) :
        m_seconds(seconds)
    {}

    std::optional<TrackLength> TrackLength::fromSeconds(const int seconds)
    {
        if (seconds < 1 || seconds > MAX_SECONDS)
            return std::nullopt;

        return TrackLength(seconds);
    }

    std::optional<TrackLength> TrackLength::parse(const std::string_view text)
    {
        std::array<std::string_view, MAX_PARTS> parts{};
        std::size_t count = 0;

        for (std::size_t start = 0;;)
        {
            if (count == MAX_PARTS)
                return std::nullopt;

            const auto colon = text.find(':', start);
            parts[count++] = text.substr(start, colon == std::string_view::npos ? std::string_view::npos : colon - start);

            if (colon == std::string_view::npos)
                break;

            start = colon + 1;
        }

        if (count < 2)
            return std::nullopt;

        const bool hasHours = count == MAX_PARTS;
        const auto hours = hasHours ? parseNumber(parts[0], 1, 2) : std::optional(0);
        const auto minutes = parseNumber(parts[count - 2], hasHours ? 2 : 1, 2);
        const auto seconds = parseNumber(parts[count - 1], SECONDS_DIGITS, SECONDS_DIGITS);

        if (!hours || !minutes || !seconds || *seconds >= SECONDS_PER_MINUTE || (hasHours && *minutes >= SECONDS_PER_MINUTE))
            return std::nullopt;

        return fromSeconds(*hours * SECONDS_PER_HOUR + *minutes * SECONDS_PER_MINUTE + *seconds);
    }

    int TrackLength::seconds() const
    {
        return m_seconds;
    }

    std::string TrackLength::toText() const
    {
        const int hours = m_seconds / SECONDS_PER_HOUR;
        const int minutes = m_seconds % SECONDS_PER_HOUR / SECONDS_PER_MINUTE;
        const int seconds = m_seconds % SECONDS_PER_MINUTE;

        if (hours > 0)
            return std::format("{}:{:02}:{:02}", hours, minutes, seconds);

        return std::format("{}:{:02}", minutes, seconds);
    }
}
