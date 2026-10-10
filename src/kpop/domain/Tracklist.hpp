#pragma once

#include "TrackLength.hpp"

#include <optional>
#include <string>
#include <vector>

namespace kpop::domain
{
    struct Track
    {
        std::string title;
        std::string writers;
        std::optional<TrackLength> length;
        bool titleTrack = false;
        int disc = 1;

        bool operator==(const Track&) const = default;
    };

    using Tracklist = std::vector<Track>;
    void normalizeDiscs(Tracklist& tracklist);

    [[nodiscard]] int discCount(const Tracklist& tracklist);
    [[nodiscard]] std::optional<TrackLength> totalLength(const Tracklist& tracklist);
    [[nodiscard]] std::optional<TrackLength> totalLength(const Tracklist& tracklist, int disc);
}
