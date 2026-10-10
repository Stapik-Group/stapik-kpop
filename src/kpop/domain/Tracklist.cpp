#include "Tracklist.hpp"

#include <algorithm>

namespace kpop::domain
{
    namespace
    {
        std::optional<TrackLength> sumOfLengths(const Tracklist& tracklist, const std::optional<int> disc)
        {
            int totalSeconds = 0;
            bool anyTrack = false;

            for (const auto& track : tracklist)
            {
                if (disc && track.disc != *disc)
                    continue;

                if (!track.length)
                    return std::nullopt;

                totalSeconds += track.length->seconds();
                anyTrack = true;
            }

            return anyTrack ? TrackLength::fromSeconds(totalSeconds) : std::nullopt;
        }
    }

    void normalizeDiscs(Tracklist& tracklist)
    {
        for (auto& track : tracklist)
            track.disc = std::max(track.disc, 1);

        std::ranges::stable_sort(tracklist, {}, &Track::disc);

        int number = 0;
        int previousDisc = 0;
        for (auto& track : tracklist)
        {
            if (track.disc != previousDisc)
            {
                previousDisc = track.disc;
                ++number;
            }

            track.disc = number;
        }
    }

    int discCount(const Tracklist& tracklist)
    {
        int count = 0;
        for (const auto& track : tracklist)
            count = std::max(count, track.disc);

        return count;
    }

    std::optional<TrackLength> totalLength(const Tracklist& tracklist)
    {
        return sumOfLengths(tracklist, std::nullopt);
    }

    std::optional<TrackLength> totalLength(const Tracklist& tracklist, const int disc)
    {
        return sumOfLengths(tracklist, disc);
    }
}
