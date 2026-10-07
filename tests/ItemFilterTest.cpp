#include "support/TestFixtures.hpp"

#include "kpop/domain/ItemFilter.hpp"

#include <gtest/gtest.h>

namespace
{
    using namespace kpop::domain;
    using namespace kpop::test;

    TEST(ItemFilterTest, EmptyFilterMatchesEverything)
    {
        EXPECT_TRUE(matchesFilter(ItemFilter{}, sampleAlbum(), nullptr));
    }

    TEST(ItemFilterTest, FiltersByKindStatusAndArtist)
    {
        const auto item = sampleAlbum();

        EXPECT_TRUE(matchesFilter(ItemFilter{ .kind = ItemKind::Album }, item, nullptr));
        EXPECT_FALSE(matchesFilter(ItemFilter{ .kind = ItemKind::Photocard }, item, nullptr));
        EXPECT_TRUE(matchesFilter(ItemFilter{ .status = ItemStatus::Owned }, item, nullptr));
        EXPECT_FALSE(matchesFilter(ItemFilter{ .status = ItemStatus::Wishlist }, item, nullptr));
        EXPECT_TRUE(matchesFilter(ItemFilter{ .artistId = "artist-1" }, item, nullptr));
        EXPECT_FALSE(matchesFilter(ItemFilter{ .artistId = "artist-2" }, item, nullptr));
    }

    TEST(ItemFilterTest, TextSearchIsCaseInsensitive)
    {
        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "noeasy" }, sampleAlbum(), nullptr));
        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "NOEASY" }, sampleAlbum(), nullptr));
        EXPECT_FALSE(matchesFilter(ItemFilter{ .text = "oddinary" }, sampleAlbum(), nullptr));
    }

    TEST(ItemFilterTest, EveryTermHasToMatch)
    {
        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "standard noeasy" }, sampleAlbum(), nullptr));
        EXPECT_FALSE(matchesFilter(ItemFilter{ .text = "standard missing" }, sampleAlbum(), nullptr));
    }

    TEST(ItemFilterTest, TextSearchCoversTracklist)
    {
        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "dance night" }, sampleAlbum(), nullptr));
        EXPECT_FALSE(matchesFilter(ItemFilter{ .text = "missing track" }, sampleAlbum(), nullptr));
    }

    TEST(ItemFilterTest, TextSearchCoversTrackWriters)
    {
        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "changbin" }, sampleAlbum(), nullptr));
    }

    TEST(ItemFilterTest, TextSearchCoversArtistNameAndMembers)
    {
        const auto artist = sampleArtist();

        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "stray" }, sampleAlbum(), &artist));
        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "felix" }, sampleAlbum(), &artist));
        EXPECT_FALSE(matchesFilter(ItemFilter{ .text = "stray" }, sampleAlbum(), nullptr));
    }

    TEST(ItemFilterTest, TextSearchCoversLyricsOfAnyVersion)
    {
        auto item = sampleItemOfKind(ItemKind::Lyrics);
        LyricsDetails lyrics;
        lyrics.originalText = "한국어 가사";
        lyrics.romanizedText = "hangugeo gasa";
        lyrics.translatedText = "Tekst po polsku";
        item.details = lyrics;

        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "가사" }, item, nullptr));
        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "gasa" }, item, nullptr));
        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "POLSKU" }, item, nullptr));
    }

    TEST(ItemFilterTest, TextSearchHandlesNonAsciiCase)
    {
        auto item = sampleAlbum();
        item.title = "Łódź";

        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "łódź" }, item, nullptr));
        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "ŁÓDŹ" }, item, nullptr));
    }
}
