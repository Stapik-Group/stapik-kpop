#include "kpop/domain/ArtistColor.hpp"

#include <gtest/gtest.h>

#include <set>

namespace
{
    using kpop::domain::artistColor;
    using kpop::domain::artistColorCount;
    using stapik::domain::CategoryColor;

    TEST(ArtistColorTest, FirstArtistsGetDifferentColors)
    {
        std::set<CategoryColor> colors;
        for (std::size_t index = 0; index < artistColorCount(); ++index)
            colors.insert(artistColor(index));

        EXPECT_EQ(colors.size(), artistColorCount());
    }

    TEST(ArtistColorTest, ColorsRepeatAfterThePalette)
    {
        for (std::size_t index = 0; index < artistColorCount(); ++index)
            EXPECT_EQ(artistColor(index), artistColor(index + artistColorCount()));
    }

    TEST(ArtistColorTest, GrayIsNeverUsed)
    {
        for (std::size_t index = 0; index < artistColorCount() * 2; ++index)
            EXPECT_NE(artistColor(index), CategoryColor::Gray);
    }
}
