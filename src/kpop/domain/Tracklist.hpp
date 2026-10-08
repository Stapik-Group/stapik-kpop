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

        bool operator==(const Track&) const = default;
    };

    // The position of a track is its index; numbers are never stored.
    using Tracklist = std::vector<Track>;

    // Only known when the tracklist is not empty and every track has a length.
    [[nodiscard]] std::optional<TrackLength> totalLength(const Tracklist& tracklist);
}
