#include "support/TestFixtures.hpp"

#include "kpop/domain/ItemFilter.hpp"

#include <gtest/gtest.h>

namespace
{
    using namespace kpop::domain;
    using namespace kpop::test;

    TEST(ItemFilterTest, RegionIsRecognisedFromOldFreeTextEntries)
    {
        EXPECT_EQ(regionOf(""), Region::Unspecified);
        EXPECT_EQ(regionOf("   "), Region::Unspecified);
        EXPECT_EQ(regionOf("japan"), Region::Japan);
        EXPECT_EQ(regionOf("Japan"), Region::Japan);
        EXPECT_EQ(regionOf(" JP "), Region::Japan);
        EXPECT_EQ(regionOf("Japonia"), Region::Japan);
        EXPECT_EQ(regionOf("South Korea"), Region::Korea);
        EXPECT_EQ(regionOf("Tajwan"), Region::Taiwan);
        EXPECT_EQ(regionOf("Narnia"), Region::Other);
    }

    TEST(ItemFilterTest, RegionTextRoundTrips)
    {
        for (const auto region : { Region::Korea, Region::Japan, Region::China, Region::Taiwan, Region::Usa, Region::Europe })
            EXPECT_EQ(regionOf(regionText(region, "ignored")), region);

        EXPECT_EQ(regionText(Region::Unspecified, "ignored"), "");
        EXPECT_EQ(regionText(Region::Other, "Narnia"), "Narnia");
    }

    TEST(ItemFilterTest, RegionFilterKeepsOnlyAlbumsOfThatRegion)
    {
        auto japanese = sampleAlbum();
        std::get<AlbumDetails>(japanese.details).region = "Japan";

        auto korean = sampleAlbum();
        std::get<AlbumDetails>(korean.details).region = "korea";

        EXPECT_TRUE(matchesFilter(ItemFilter{ .region = Region::Japan }, japanese, nullptr));
        EXPECT_FALSE(matchesFilter(ItemFilter{ .region = Region::Japan }, korean, nullptr));
        EXPECT_FALSE(matchesFilter(ItemFilter{ .region = Region::Japan }, sampleAlbum(), nullptr));
        EXPECT_TRUE(matchesFilter(ItemFilter{}, korean, nullptr));
    }

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

    TEST(ItemFilterTest, TextSearchCoversEditionAndVariant)
    {
        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "standard" }, sampleAlbum(), nullptr));
        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "felix" }, sampleAlbum(), nullptr));
        EXPECT_FALSE(matchesFilter(ItemFilter{ .text = "hyunjin" }, sampleAlbum(), nullptr));
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

    TEST(ItemFilterTest, TextSearchCoversApparelSize)
    {
        auto item = sampleItemOfKind(ItemKind::Merchandise);
        item.details = MerchandiseDetails{ .type = MerchandiseType::Apparel, .version = "", .official = true, .size = "XL" };

        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "xl" }, item, nullptr));
        EXPECT_FALSE(matchesFilter(ItemFilter{ .text = "xxl" }, item, nullptr));
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

    TEST(ItemFilterTest, TextSearchIgnoresDiacritics)
    {
        auto item = sampleAlbum();
        item.title = "Łódź";

        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "lodz" }, item, nullptr));
        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "LODZ" }, item, nullptr));
        EXPECT_FALSE(matchesFilter(ItemFilter{ .text = "lodzx" }, item, nullptr));

        item.title = "Lodz";
        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "łódź" }, item, nullptr));
    }

    TEST(ItemFilterTest, TextSearchIgnoresDiacriticsInArtistNames)
    {
        auto item = sampleAlbum();
        auto artist = sampleArtist();
        artist.name = "Beyoncé";

        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "beyonce" }, item, &artist));
    }

    TEST(ItemFilterTest, TextSearchFindsHangulByItsBeginning)
    {
        auto item = sampleAlbum();
        item.title = "방탄소년단";

        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "방탄" }, item, nullptr));
        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "소년단" }, item, nullptr));
        EXPECT_FALSE(matchesFilter(ItemFilter{ .text = "블랙핑크" }, item, nullptr));
    }

    TEST(ItemFilterTest, TextSearchFindsJapaneseTitles)
    {
        auto item = sampleAlbum();
        item.title = "ありがとう";

        EXPECT_TRUE(matchesFilter(ItemFilter{ .text = "ありが" }, item, nullptr));
        EXPECT_FALSE(matchesFilter(ItemFilter{ .text = "さようなら" }, item, nullptr));
    }
}
