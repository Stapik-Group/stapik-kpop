#pragma once

#include <compare>
#include <optional>
#include <string>
#include <string_view>

namespace kpop::domain
{
    class TrackLength
    {
    public:
        // Accepts 1 second up to 99:59:59.
        [[nodiscard]] static std::optional<TrackLength> fromSeconds(int seconds);

        // Accepts "m:ss", "mm:ss" and "h:mm:ss".
        [[nodiscard]] static std::optional<TrackLength> parse(std::string_view text);

        [[nodiscard]] int seconds() const;

        // "m:ss", or "h:mm:ss" when the length reaches an hour.
        [[nodiscard]] std::string toText() const;

        [[nodiscard]] std::strong_ordering operator<=>(const TrackLength& other) const = default;
        [[nodiscard]] bool operator==(const TrackLength& other) const = default;

    private:
        explicit TrackLength(int seconds);

        int m_seconds;
    };
}
