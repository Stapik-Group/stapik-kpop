#include "kpop/domain/ItemSort.hpp"

#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace
{
    using namespace kpop::domain;

    struct Entry
    {
        std::string title;
        std::optional<std::string> artistName;
        std::optional<PartialDate> releaseDate;
    };

    // The entries are given newest first, like the list of the application receives them.
    std::vector<std::string> sortedTitles(const std::vector<Entry>& entries, const SortOrder order)
    {
        std::vector<CollectionItem> items;
        std::vector<Artist> artists;
        items.reserve(entries.size());
        artists.reserve(entries.size());

        for (const auto& entry : entries)
        {
            CollectionItem item;
            item.id = entry.title;
            item.title = entry.title;

            AlbumDetails album;
            album.releaseDate = entry.releaseDate;
            item.details = album;
            items.push_back(std::move(item));

            Artist artist;
            artist.name = entry.artistName.value_or("");
            artists.push_back(std::move(artist));
        }

        std::vector<SortableItem> sortable;
        for (std::size_t index = 0; index < entries.size(); ++index)
            sortable.push_back({ &items[index], entries[index].artistName ? &artists[index] : nullptr });

        sortItems(sortable, order);

        std::vector<std::string> titles;
        for (const auto& entry : sortable)
            titles.push_back(entry.item->title);

        return titles;
    }

    using Titles = std::vector<std::string>;

    TEST(ItemSortTest, AddedOrdersFollowTheIncomingOrder)
    {
        const std::vector<Entry> entries = { { "C", {}, {} }, { "A", {}, {} }, { "B", {}, {} } };

        EXPECT_EQ(sortedTitles(entries, SortOrder::AddedNewest), (Titles{ "C", "A", "B" }));
        EXPECT_EQ(sortedTitles(entries, SortOrder::AddedOldest), (Titles{ "B", "A", "C" }));
    }

    TEST(ItemSortTest, TitlesAreSortedAlphabeticallyIgnoringCase)
    {
        const std::vector<Entry> entries = { { "banana", {}, {} }, { "Apple", {}, {} }, { "cherry", {}, {} } };

        EXPECT_EQ(sortedTitles(entries, SortOrder::TitleAscending), (Titles{ "Apple", "banana", "cherry" }));
        EXPECT_EQ(sortedTitles(entries, SortOrder::TitleDescending), (Titles{ "cherry", "banana", "Apple" }));
    }

    TEST(ItemSortTest, ArtistsAreSortedAlphabeticallyAndItemsWithoutArtistComeLast)
    {
        const std::vector<Entry> entries = {
            { "no artist", std::nullopt, {} },
            { "by Twice", "Twice", {} },
            { "by aespa", "aespa", {} },
            { "by Stray Kids", "Stray Kids", {} } };

        EXPECT_EQ(sortedTitles(entries, SortOrder::ArtistAscending), (Titles{ "by aespa", "by Stray Kids", "by Twice", "no artist" }));
        EXPECT_EQ(sortedTitles(entries, SortOrder::ArtistDescending), (Titles{ "by Twice", "by Stray Kids", "by aespa", "no artist" }));
    }

    TEST(ItemSortTest, ReleaseDatesAreSortedAndItemsWithoutDateComeLast)
    {
        const std::vector<Entry> entries = {
            { "undated", {}, std::nullopt },
            { "2021-08", {}, PartialDate::tryCreate(2021, 8) },
            { "2019", {}, PartialDate::tryCreate(2019) },
            { "2021", {}, PartialDate::tryCreate(2021) } };

        EXPECT_EQ(sortedTitles(entries, SortOrder::ReleaseDateOldest), (Titles{ "2019", "2021", "2021-08", "undated" }));
        EXPECT_EQ(sortedTitles(entries, SortOrder::ReleaseDateNewest), (Titles{ "2021-08", "2021", "2019", "undated" }));
    }

    TEST(ItemSortTest, EqualItemsKeepTheIncomingOrder)
    {
        const std::vector<Entry> entries = {
            { "newer", "Twice", PartialDate::tryCreate(2021) },
            { "older", "Twice", PartialDate::tryCreate(2021) } };

        EXPECT_EQ(sortedTitles(entries, SortOrder::ArtistAscending), (Titles{ "newer", "older" }));
        EXPECT_EQ(sortedTitles(entries, SortOrder::ArtistDescending), (Titles{ "newer", "older" }));
        EXPECT_EQ(sortedTitles(entries, SortOrder::ReleaseDateNewest), (Titles{ "newer", "older" }));
    }

    TEST(ItemSortTest, OnlyAlbumsAndClipsHaveAReleaseDate)
    {
        std::vector<CollectionItem> items(3);
        items[0].title = "photocard";
        items[0].details = PhotocardDetails{};
        items[1].title = "clip";

        ClipDetails clip;
        clip.releaseDate = PartialDate::tryCreate(2020);
        items[1].details = clip;

        items[2].title = "album";

        AlbumDetails album;
        album.releaseDate = PartialDate::tryCreate(2022);
        items[2].details = album;

        std::vector<SortableItem> sortable = { { &items[0], nullptr }, { &items[1], nullptr }, { &items[2], nullptr } };
        sortItems(sortable, SortOrder::ReleaseDateOldest);

        ASSERT_EQ(sortable.size(), 3U);
        EXPECT_EQ(sortable[0].item->title, "clip");
        EXPECT_EQ(sortable[1].item->title, "album");
        EXPECT_EQ(sortable[2].item->title, "photocard");
    }
}
