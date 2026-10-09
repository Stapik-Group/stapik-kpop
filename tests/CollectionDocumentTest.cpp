#include "support/TestFixtures.hpp"

#include "stapik/sync/SyncCoordinator.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{
    using kpop::document::CollectionDocument;
    using namespace kpop::domain;
    using namespace kpop::test;

    static_assert(stapik::sync::SyncableDocument<CollectionDocument>);

    CollectionDocument roundTrip(const CollectionDocument& document)
    {
        return CollectionDocument::fromJson(document.toJson());
    }

    TEST(CollectionDocumentTest, EmptyDocumentRoundTrips)
    {
        const auto restored = roundTrip(CollectionDocument{});
        EXPECT_TRUE(restored.items().empty());
        EXPECT_TRUE(restored.artists().empty());
    }

    TEST(CollectionDocumentTest, PreservesEveryFieldOfAnAlbum)
    {
        const auto restored = roundTrip(sampleDocument());

        ASSERT_EQ(restored.items().size(), 1U);
        EXPECT_EQ(restored.items().front(), sampleAlbum());
        ASSERT_EQ(restored.artists().size(), 1U);
        EXPECT_EQ(restored.artists().front(), sampleArtist());
    }

    TEST(CollectionDocumentTest, PreservesEveryItemKind)
    {
        CollectionDocument document;

        auto photocard = sampleItemOfKind(ItemKind::Photocard);
        photocard.details = PhotocardDetails{ .member = "Felix", .origin = PhotocardOrigin::LuckyDraw, .source = "Soundwave", .forTrade = true };

        auto merchandise = sampleItemOfKind(ItemKind::Merchandise);
        merchandise.details = MerchandiseDetails{ .type = MerchandiseType::Lightstick, .version = "Ver. 2", .official = false, .size = "" };

        auto clip = sampleItemOfKind(ItemKind::Clip);
        clip.details = ClipDetails{
            .type = ClipType::Performance,
            .platform = "YouTube",
            .url = "https://example.com/watch",
            .releaseDate = PartialDate::tryCreate(2022),
            .albumTitle = "ODDINARY",
            .watched = true };

        auto lyrics = sampleItemOfKind(ItemKind::Lyrics);
        lyrics.details = LyricsDetails{
            .albumTitle = "ODDINARY",
            .writers = "3RACHA",
            .originalText = "한국어\nsecond line",
            .romanizedText = "hangugeo",
            .translatedText = "Koreański",
            .translationLanguage = "pl" };

        auto event = sampleItemOfKind(ItemKind::Event);
        event.details = EventDetails{
            .type = EventType::Fansign,
            .date = PartialDate::tryCreate(2023, 5, 6),
            .venue = "Arena",
            .city = "Seoul",
            .seat = "A12" };

        for (auto* item : { &photocard, &merchandise, &clip, &lyrics, &event })
            document.insertItem(document.items().size(), *item);

        const auto restored = roundTrip(document);

        ASSERT_EQ(restored.items().size(), 5U);
        EXPECT_EQ(restored.items()[0], photocard);
        EXPECT_EQ(restored.items()[1], merchandise);
        EXPECT_EQ(restored.items()[2], clip);
        EXPECT_EQ(restored.items()[3], lyrics);
        EXPECT_EQ(restored.items()[4], event);
    }

    TEST(CollectionDocumentTest, ItemWithoutOptionalFieldsRoundTrips)
    {
        CollectionDocument document;
        CollectionItem item;
        item.id = "bare";
        item.title = "Bare";
        document.insertItem(0, item);

        const auto restored = roundTrip(document);
        EXPECT_EQ(restored.items().front(), item);
        EXPECT_FALSE(restored.items().front().price.has_value());
    }

    TEST(CollectionDocumentTest, KeepsAlbumWithoutTracklistEmpty)
    {
        CollectionDocument document;
        auto item = sampleAlbum();
        std::get<AlbumDetails>(item.details).tracklist.clear();
        document.insertItem(0, item);

        const auto restored = roundTrip(document);
        EXPECT_TRUE(std::get<AlbumDetails>(restored.items().front().details).tracklist.empty());
    }

    TEST(CollectionDocumentTest, ReadsTracklistSavedAsPlainTitles)
    {
        auto json = sampleDocument().toJson();
        json["items"][0]["details"]["tracklist"] = { "Intro", "Outro" };

        const auto restored = CollectionDocument::fromJson(json);
        const auto& tracklist = std::get<AlbumDetails>(restored.items().front().details).tracklist;
        ASSERT_EQ(tracklist.size(), 2U);
        EXPECT_EQ(tracklist.front().title, "Intro");
        EXPECT_FALSE(tracklist.front().length.has_value());
    }

    TEST(CollectionDocumentTest, RejectsInvalidTrackLength)
    {
        auto json = sampleDocument().toJson();
        json["items"][0]["details"]["tracklist"][0]["length"] = 0;

        EXPECT_THROW(static_cast<void>(CollectionDocument::fromJson(json)), std::invalid_argument);
    }

    TEST(CollectionDocumentTest, ItemWithoutImageHasNoImageKey)
    {
        CollectionDocument document;
        auto item = sampleAlbum();
        item.image.reset();
        document.insertItem(0, item);

        EXPECT_EQ(document.toJson().dump().find("\"image\""), std::string::npos);
        EXPECT_FALSE(roundTrip(document).items().front().image.has_value());
    }

    TEST(CollectionDocumentTest, RejectsInvalidImageId)
    {
        auto json = sampleDocument().toJson();
        json["items"][0]["image"] = "not-a-hash";

        EXPECT_THROW(static_cast<void>(CollectionDocument::fromJson(json)), std::invalid_argument);
    }

    TEST(CollectionDocumentTest, ReadsNullImageAsNoImage)
    {
        auto json = sampleDocument().toJson();
        json["items"][0]["image"] = nullptr;

        const auto restored = CollectionDocument::fromJson(json);
        EXPECT_FALSE(restored.items().front().image.has_value());
    }

    TEST(CollectionDocumentTest, ListsEveryReferencedImageOnceInOrder)
    {
        const auto first = ImageId::parse(std::string(64, 'a'));
        const auto second = ImageId::parse(std::string(64, 'b'));
        ASSERT_TRUE(first.has_value() && second.has_value());

        CollectionDocument document;
        for (const auto& image : { second, first, second, std::optional<ImageId>() })
        {
            auto item = sampleAlbum();
            item.image = image;
            document.insertItem(0, item);
        }

        EXPECT_EQ(document.referencedImages(), (std::vector<ImageId>{ *first, *second }));
        EXPECT_TRUE(CollectionDocument().referencedImages().empty());
    }

    TEST(CollectionDocumentTest, KeepsSizeOfApparel)
    {
        CollectionDocument document;
        auto item = sampleItemOfKind(ItemKind::Merchandise);
        item.details = MerchandiseDetails{ .type = MerchandiseType::Apparel, .version = "", .official = true, .size = "XL" };
        document.insertItem(0, item);

        const auto restored = roundTrip(document);
        EXPECT_EQ(std::get<MerchandiseDetails>(restored.items().front().details).size, "XL");
    }

    TEST(CollectionDocumentTest, ReadsMerchandiseWithoutSizeAsEmpty)
    {
        CollectionDocument document;
        document.insertItem(0, sampleItemOfKind(ItemKind::Merchandise));

        const auto restored = roundTrip(document);
        EXPECT_TRUE(std::get<MerchandiseDetails>(restored.items().front().details).size.empty());
    }

    TEST(CollectionDocumentTest, KeepsAmountsOfCurrenciesWithoutDecimals)
    {
        CollectionDocument document;
        auto item = sampleAlbum();
        item.price = stapik::domain::Money(15000, won());
        document.insertItem(0, item);

        const auto restored = roundTrip(document);
        ASSERT_TRUE(restored.items().front().price.has_value());
        EXPECT_EQ(restored.items().front().price->minorUnits(), 15000);
        EXPECT_EQ(restored.items().front().price->currency().decimalPlaces, 0);
        EXPECT_EQ(restored.items().front().price->currency().code, "KRW");
    }

    TEST(CollectionDocumentTest, KeepsTimestamps)
    {
        const auto baseline = sampleDocument().lastUpdate() + std::chrono::microseconds{ 123456 };
        const auto restored = roundTrip(sampleDocument().withLastKnownCloudUpdate(baseline));

        EXPECT_EQ(restored.lastUpdate(), sampleDocument().lastUpdate());
        ASSERT_TRUE(restored.lastKnownCloudUpdate().has_value());
        EXPECT_EQ(*restored.lastKnownCloudUpdate(), baseline);
    }

    TEST(CollectionDocumentTest, RejectsUnknownEnumValue)
    {
        auto json = sampleDocument().toJson();
        json["items"][0]["status"] = "lost-in-space";

        EXPECT_THROW(static_cast<void>(CollectionDocument::fromJson(json)), std::invalid_argument);
    }

    TEST(CollectionDocumentTest, RejectsInvalidDate)
    {
        auto json = sampleDocument().toJson();
        json["items"][0]["acquiredOn"] = "2024-02-31";

        EXPECT_THROW(static_cast<void>(CollectionDocument::fromJson(json)), std::invalid_argument);
    }

    TEST(CollectionDocumentTest, RejectsDocumentFromNewerSchema)
    {
        auto json = sampleDocument().toJson();
        json["schemaVersion"] = 999;

        EXPECT_THROW(static_cast<void>(CollectionDocument::fromJson(json)), std::invalid_argument);
    }

    TEST(CollectionDocumentTest, RejectsMissingRequiredField)
    {
        auto json = sampleDocument().toJson();
        json["items"][0].erase("title");

        EXPECT_THROW(static_cast<void>(CollectionDocument::fromJson(json)), std::exception);
    }

    TEST(CollectionDocumentTest, InsertReplaceAndRemoveWorkById)
    {
        auto document = sampleDocument();

        auto second = sampleAlbum("item-2", "ODDINARY");
        document.insertItem(0, second);
        ASSERT_EQ(document.items().size(), 2U);
        EXPECT_EQ(document.items().front().id, "item-2");
        EXPECT_EQ(document.indexOfItem("item-1"), 1U);

        second.title = "Changed";
        EXPECT_TRUE(document.replaceItem(second));
        EXPECT_EQ(document.findItem("item-2")->title, "Changed");

        EXPECT_FALSE(document.replaceItem(sampleAlbum("missing")));
        EXPECT_TRUE(document.removeItem("item-2"));
        EXPECT_FALSE(document.removeItem("item-2"));
        EXPECT_EQ(document.findItem("item-2"), nullptr);
    }

    TEST(CollectionDocumentTest, InsertBeyondTheEndAppends)
    {
        auto document = sampleDocument();
        document.insertItem(100, sampleAlbum("item-2"));

        EXPECT_EQ(document.items().back().id, "item-2");
    }

    TEST(CollectionDocumentTest, CountsItemsOfAnArtist)
    {
        auto document = sampleDocument();
        document.insertItem(0, sampleAlbum("item-2"));

        EXPECT_EQ(document.countItemsOfArtist("artist-1"), 2U);
        EXPECT_EQ(document.countItemsOfArtist("someone-else"), 0U);
    }

    TEST(CollectionDocumentTest, WithLastKnownCloudUpdateLeavesTheOriginalUntouched)
    {
        const auto original = sampleDocument();
        const auto baseline = original.lastUpdate() + std::chrono::hours{ 1 };
        const auto updated = original.withLastKnownCloudUpdate(baseline);

        EXPECT_FALSE(original.lastKnownCloudUpdate().has_value());
        EXPECT_EQ(updated.lastKnownCloudUpdate(), baseline);
        EXPECT_EQ(updated.items(), original.items());
    }
}
