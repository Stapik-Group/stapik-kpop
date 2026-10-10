#include "ArtistColor.hpp"

#include <array>

namespace kpop::domain
{
    namespace
    {
        using stapik::domain::CategoryColor;

        // Neighbours differ a lot, so artists added one after another are easy to tell apart.
        constexpr std::array PALETTE = {
            CategoryColor::Red,
            CategoryColor::Blue,
            CategoryColor::Green,
            CategoryColor::Orange,
            CategoryColor::Purple,
            CategoryColor::Teal,
            CategoryColor::Pink,
            CategoryColor::Yellow,
            CategoryColor::Brown };
    }

    stapik::domain::CategoryColor artistColor(const std::size_t artistIndex)
    {
        return PALETTE[artistIndex % PALETTE.size()];
    }

    std::size_t artistColorCount()
    {
        return PALETTE.size();
    }
}
