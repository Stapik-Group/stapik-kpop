#include "Tracklist.hpp"

namespace kpop::domain
{
    std::optional<TrackLength> totalLength(const Tracklist& tracklist)
    {
        if (tracklist.empty())
            return std::nullopt;

        int totalSeconds = 0;
        for (const auto& track : tracklist)
        {
            if (!track.length)
                return std::nullopt;

            totalSeconds += track.length->seconds();
        }

        return TrackLength::fromSeconds(totalSeconds);
    }
}
