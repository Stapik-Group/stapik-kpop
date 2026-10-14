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
        bool bonus = false;

        bool operator==(const Track&) const = default;
    };

    using Tracklist = std::vector<Track>;
    void normalizeDiscs(Tracklist& tracklist);

    // A position that is kept in the tracklist but not filled in yet. It holds a place, so the numbering of the
    // tracks after it stays right, and it is left out of the total length.
    [[nodiscard]] bool isPlaceholder(const Track& track);

    [[nodiscard]] int discCount(const Tracklist& tracklist);
    [[nodiscard]] std::optional<TrackLength> totalLength(const Tracklist& tracklist);
    [[nodiscard]] std::optional<TrackLength> totalLength(const Tracklist& tracklist, int disc);
}
