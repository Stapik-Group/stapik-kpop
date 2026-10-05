#include "support/TestFixtures.hpp"

#include "kpop/domain/CollectionStatistics.hpp"

#include <gtest/gtest.h>

namespace
{
    using namespace kpop::domain;
    using namespace kpop::test;

    TEST(CollectionStatisticsTest, EmptyCollectionHasNothing)
    {
        const auto statistics = computeStatistics({});

        EXPECT_EQ(statistics.totalEntries(), 0U);
        EXPECT_EQ(statistics.ownedCopies, 0U);
        EXPECT_TRUE(statistics.spent.empty());
    }

    TEST(CollectionStatisticsTest, CountsEntriesPerKindAndOwnedCopies)
    {
        std::vector<CollectionItem> items{ sampleAlbum("album-1"), sampleAlbum("album-2"), sampleItemOfKind(ItemKind::Photocard) };
        items[1].status = ItemStatus::Wishlist;

        const auto statistics = computeStatistics(items);

        EXPECT_EQ(statistics.entriesOfKind(ItemKind::Album), 2U);
        EXPECT_EQ(statistics.entriesOfKind(ItemKind::Photocard), 1U);
        EXPECT_EQ(statistics.entriesOfKind(ItemKind::Event), 0U);
        EXPECT_EQ(statistics.totalEntries(), 3U);
        EXPECT_EQ(statistics.ownedCopies, 4U);
    }

    TEST(CollectionStatisticsTest, SumsSpentMoneyPerCurrencyMultipliedByQuantity)
    {
        auto polishAlbum = sampleAlbum("polish");
        polishAlbum.quantity = 3;
        polishAlbum.price = stapik::domain::Money(1000, zloty());

        auto koreanAlbum = sampleAlbum("korean");
        koreanAlbum.quantity = 1;
        koreanAlbum.price = stapik::domain::Money(20000, won());

        auto secondPolishAlbum = sampleAlbum("polish-2");
        secondPolishAlbum.status = ItemStatus::Ordered;
        secondPolishAlbum.quantity = 1;
        secondPolishAlbum.price = stapik::domain::Money(500, zloty());

        const auto statistics = computeStatistics(std::vector<CollectionItem>{ polishAlbum, koreanAlbum, secondPolishAlbum });

        ASSERT_EQ(statistics.spent.size(), 2U);
        EXPECT_EQ(statistics.spent[0].currency().code, "PLN");
        EXPECT_EQ(statistics.spent[0].minorUnits(), 3500);
        EXPECT_EQ(statistics.spent[1].currency().code, "KRW");
        EXPECT_EQ(statistics.spent[1].minorUnits(), 20000);
    }

    TEST(CollectionStatisticsTest, WishlistAndSoldItemsAreNotSpentMoney)
    {
        auto wished = sampleAlbum("wished");
        wished.status = ItemStatus::Wishlist;

        auto sold = sampleAlbum("sold");
        sold.status = ItemStatus::Sold;

        const auto statistics = computeStatistics(std::vector<CollectionItem>{ wished, sold });

        EXPECT_TRUE(statistics.spent.empty());
        EXPECT_EQ(statistics.ownedCopies, 0U);
    }
}
