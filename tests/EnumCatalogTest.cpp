#include "kpop/domain/Enumerations.hpp"

#include <gtest/gtest.h>

#include <set>
#include <string>

namespace
{
    using namespace kpop::domain;

    template<typename Catalog>
    void expectWellFormed(const Catalog& catalog)
    {
        std::set<std::string> seenIds;

        for (const auto& entry : catalog.entries())
        {
            EXPECT_FALSE(entry.id.empty());
            EXPECT_TRUE(seenIds.insert(std::string(entry.id)).second) << "duplicate id " << entry.id;
            EXPECT_EQ(catalog.idOf(entry.value), entry.id);
            EXPECT_EQ(catalog.fromId(entry.id), entry.value);
        }
    }

    TEST(EnumCatalogTest, EveryCatalogHasUniqueIdsThatRoundTrip)
    {
        expectWellFormed(ITEM_KINDS);
        expectWellFormed(ITEM_STATUSES);
        expectWellFormed(ITEM_CONDITIONS);
        expectWellFormed(ARTIST_TYPES);
        expectWellFormed(ALBUM_TYPES);
        expectWellFormed(ALBUM_FORMATS);
        expectWellFormed(MERCHANDISE_TYPES);
        expectWellFormed(PHOTOCARD_ORIGINS);
        expectWellFormed(CLIP_TYPES);
        expectWellFormed(EVENT_TYPES);
    }

    TEST(EnumCatalogTest, UnknownIdIsNotFound)
    {
        EXPECT_FALSE(ITEM_STATUSES.fromId("missing").has_value());
        EXPECT_FALSE(ITEM_STATUSES.fromId("").has_value());
    }

    TEST(EnumCatalogTest, NameKeyCombinesPrefixAndId)
    {
        EXPECT_EQ(ITEM_STATUSES.nameKey(ItemStatus::Wishlist), "kpop.status.wishlist");
        EXPECT_EQ(MERCHANDISE_TYPES.nameKey(MerchandiseType::SeasonsGreetings), "kpop.merchandiseType.seasonsGreetings");
    }

    TEST(EnumCatalogTest, EveryKindHasItsOwnColor)
    {
        std::set<int> colors;
        for (const auto& entry : ITEM_KINDS.entries())
            colors.insert(static_cast<int>(colorOf(entry.value)));

        EXPECT_EQ(colors.size(), ITEM_KIND_COUNT);
    }
}
