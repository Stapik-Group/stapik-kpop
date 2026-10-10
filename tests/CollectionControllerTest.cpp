#include "support/TestFixtures.hpp"

#include "kpop/app/CollectionController.hpp"
#include "kpop/document/CollectionSchema.hpp"

#include <gtest/gtest.h>

#include <memory>

namespace
{
    using kpop::app::ArtistRemoval;
    using kpop::app::CollectionController;
    using stapik::document::LoadStatus;
    using namespace kpop::domain;
    using namespace kpop::test;

    class CollectionControllerTest : public testing::Test
    {
    protected:
        [[nodiscard]] std::filesystem::path documentPath() const
        {
            return m_directory.path() / "collection.json";
        }

        [[nodiscard]] std::unique_ptr<CollectionController> openController() const
        {
            return std::make_unique<CollectionController>(documentPath(), m_directory.path() / "images", m_directory.path() / "photos", stapik::sync::CloudSessionHooks{});
        }

    private:
        TemporaryDirectory m_directory;
    };

    TEST_F(CollectionControllerTest, StartsEmptyWhenThereIsNoFile)
    {
        const auto controller = openController();

        EXPECT_EQ(controller->loadStatus(), LoadStatus::Missing);
        EXPECT_TRUE(controller->document().items().empty());
        EXPECT_FALSE(controller->isCloudConnected());
    }

    TEST_F(CollectionControllerTest, AddedItemGetsAnIdAndIsSavedToDisk)
    {
        std::string itemId;
        {
            const auto controller = openController();
            auto item = sampleAlbum();
            item.id.clear();
            itemId = controller->addItem(item);
            EXPECT_FALSE(itemId.empty());
        }

        const auto reopened = openController();
        EXPECT_EQ(reopened->loadStatus(), LoadStatus::Loaded);
        ASSERT_EQ(reopened->document().items().size(), 1U);
        EXPECT_EQ(reopened->document().items().front().id, itemId);
        EXPECT_EQ(reopened->document().items().front().title, "NOEASY");
    }

    TEST_F(CollectionControllerTest, EditsAreUndoableAndRedoable)
    {
        const auto controller = openController();
        const auto itemId = controller->addItem(sampleAlbum());

        auto edited = *controller->document().findItem(itemId);
        edited.title = "Renamed";
        controller->updateItem(edited);
        EXPECT_EQ(controller->document().findItem(itemId)->title, "Renamed");

        EXPECT_TRUE(controller->undoStack().undo());
        EXPECT_EQ(controller->document().findItem(itemId)->title, "NOEASY");

        EXPECT_TRUE(controller->undoStack().redo());
        EXPECT_EQ(controller->document().findItem(itemId)->title, "Renamed");

        controller->removeItem(itemId);
        EXPECT_EQ(controller->document().findItem(itemId), nullptr);

        EXPECT_TRUE(controller->undoStack().undo());
        EXPECT_NE(controller->document().findItem(itemId), nullptr);
    }

    TEST_F(CollectionControllerTest, DuplicateIsACopyWithANewId)
    {
        const auto controller = openController();
        const auto itemId = controller->addItem(sampleAlbum());

        const auto copyId = controller->duplicateItem(itemId);

        ASSERT_FALSE(copyId.empty());
        EXPECT_NE(copyId, itemId);
        ASSERT_EQ(controller->document().items().size(), 2U);

        auto copy = *controller->document().findItem(copyId);
        copy.id = itemId;
        EXPECT_EQ(copy, *controller->document().findItem(itemId));
    }

    TEST_F(CollectionControllerTest, DuplicatingAnUnknownItemChangesNothing)
    {
        const auto controller = openController();
        controller->addItem(sampleAlbum());

        EXPECT_TRUE(controller->duplicateItem("missing").empty());
        EXPECT_EQ(controller->document().items().size(), 1U);
    }

    TEST_F(CollectionControllerTest, DuplicateIsUndoable)
    {
        const auto controller = openController();
        const auto itemId = controller->addItem(sampleAlbum());
        const auto copyId = controller->duplicateItem(itemId);

        EXPECT_TRUE(controller->undoStack().undo());
        EXPECT_EQ(controller->document().findItem(copyId), nullptr);
        EXPECT_NE(controller->document().findItem(itemId), nullptr);
    }

    TEST_F(CollectionControllerTest, DuplicateKeepsTheImagesOfTheOriginal)
    {
        const auto controller = openController();
        const auto itemId = controller->addItem(sampleAlbum());
        const auto copyId = controller->duplicateItem(itemId);

        const auto& original = *controller->document().findItem(itemId);
        const auto& copy = *controller->document().findItem(copyId);
        EXPECT_EQ(copy.image, original.image);
        EXPECT_EQ(copy.photos, original.photos);
    }

    TEST_F(CollectionControllerTest, UndoRestoresTheOriginalPositionOfARemovedItem)
    {
        const auto controller = openController();
        controller->addItem(sampleAlbum("first", "First"));
        controller->addItem(sampleAlbum("second", "Second"));
        controller->addItem(sampleAlbum("third", "Third"));

        controller->removeItem("second");
        EXPECT_TRUE(controller->undoStack().undo());

        ASSERT_EQ(controller->document().items().size(), 3U);
        EXPECT_EQ(controller->document().items()[1].id, "second");
    }

    TEST_F(CollectionControllerTest, UndoneChangesArePersistedToo)
    {
        {
            const auto controller = openController();
            controller->addItem(sampleAlbum());
            EXPECT_TRUE(controller->undoStack().undo());
        }

        EXPECT_TRUE(openController()->document().items().empty());
    }

    TEST_F(CollectionControllerTest, UpdatingWithoutChangesDoesNotCreateAnUndoStep)
    {
        const auto controller = openController();
        const auto itemId = controller->addItem(sampleAlbum());
        const auto undoDepth = controller->undoStack().undoDepth();

        controller->updateItem(*controller->document().findItem(itemId));

        EXPECT_EQ(controller->undoStack().undoDepth(), undoDepth);
    }

    TEST_F(CollectionControllerTest, UpdatingOrRemovingAMissingItemDoesNothing)
    {
        const auto controller = openController();

        controller->updateItem(sampleAlbum("missing"));
        controller->removeItem("missing");

        EXPECT_FALSE(controller->undoStack().canUndo());
    }

    TEST_F(CollectionControllerTest, ArtistInUseCannotBeRemoved)
    {
        const auto controller = openController();
        const auto artistId = controller->addArtist(sampleArtist("", "Stray Kids"));

        auto album = sampleAlbum();
        album.artistId = artistId;
        const auto itemId = controller->addItem(album);

        EXPECT_EQ(controller->removeArtist(artistId), ArtistRemoval::InUse);
        EXPECT_NE(controller->document().findArtist(artistId), nullptr);

        controller->removeItem(itemId);
        EXPECT_EQ(controller->removeArtist(artistId), ArtistRemoval::Removed);
        EXPECT_EQ(controller->document().findArtist(artistId), nullptr);
        EXPECT_EQ(controller->removeArtist(artistId), ArtistRemoval::NotFound);
    }

    TEST_F(CollectionControllerTest, ArtistRemovalCanBeUndone)
    {
        const auto controller = openController();
        const auto artistId = controller->addArtist(sampleArtist("", "ITZY"));

        EXPECT_EQ(controller->removeArtist(artistId), ArtistRemoval::Removed);
        EXPECT_TRUE(controller->undoStack().undo());

        ASSERT_NE(controller->document().findArtist(artistId), nullptr);
        EXPECT_EQ(controller->document().findArtist(artistId)->name, "ITZY");
    }

    TEST_F(CollectionControllerTest, ChangesAreAnnouncedToTheUi)
    {
        const auto controller = openController();
        int notifications = 0;
        controller->signalDocumentChanged().connect([&notifications] { ++notifications; });

        const auto itemId = controller->addItem(sampleAlbum());
        controller->removeItem(itemId);
        EXPECT_TRUE(controller->undoStack().undo());

        EXPECT_EQ(notifications, 3);
    }

    TEST_F(CollectionControllerTest, ChangesAdvanceTheLastUpdateTimestamp)
    {
        const auto controller = openController();
        const auto before = controller->document().lastUpdate();

        controller->addItem(sampleAlbum());

        EXPECT_GT(controller->document().lastUpdate(), before);
    }

    TEST_F(CollectionControllerTest, CorruptedFileIsMovedAsideAndTheCollectionStartsEmpty)
    {
        writeTextFile(documentPath(), "{ this is not json");

        const auto controller = openController();

        EXPECT_EQ(controller->loadStatus(), LoadStatus::Corrupted);
        EXPECT_TRUE(controller->document().items().empty());
        EXPECT_FALSE(std::filesystem::exists(documentPath()));
    }

    TEST_F(CollectionControllerTest, UnusedImagesAreRemovedAndUsedOnesKept)
    {
        const auto controller = openController();
        auto& store = controller->imageLibrary().store();
        const auto used = store.add({ 'u', 's', 'e', 'd' });
        const auto unused = store.add({ 'u', 'n', 'u', 's', 'e', 'd' });

        auto item = sampleAlbum();
        item.image = used;
        controller->addItem(item);

        EXPECT_EQ(controller->removeUnusedImages(), 1U);
        EXPECT_TRUE(store.contains(used));
        EXPECT_FALSE(store.contains(unused));
    }

    TEST_F(CollectionControllerTest, UnusedPhotosAreRemovedAndUsedOnesKept)
    {
        const auto controller = openController();
        auto& store = controller->photoLibrary().store();
        const auto used = store.add({ 'u', 's', 'e', 'd' });
        const auto unused = store.add({ 'u', 'n', 'u', 's', 'e', 'd' });

        auto item = sampleAlbum();
        item.photos = { used };
        controller->addItem(item);

        EXPECT_EQ(controller->removeUnusedImages(), 1U);
        EXPECT_TRUE(store.contains(used));
        EXPECT_FALSE(store.contains(unused));
    }

    TEST_F(CollectionControllerTest, ACoverIsNotKeptJustBecauseItIsUsedAsAPhoto)
    {
        const auto controller = openController();
        auto& covers = controller->imageLibrary().store();
        const auto cover = covers.add({ 'c', 'o', 'v', 'e', 'r' });

        auto item = sampleAlbum();
        item.photos = { cover };
        controller->addItem(item);

        EXPECT_EQ(controller->removeUnusedImages(), 1U);
        EXPECT_FALSE(covers.contains(cover));
    }

    TEST_F(CollectionControllerTest, ImagesAreKeptWhenTheCollectionCouldNotBeRead)
    {
        writeTextFile(documentPath(), "{ this is not json");
        const auto controller = openController();
        auto& store = controller->imageLibrary().store();
        const auto image = store.add({ 'i', 'm', 'g' });

        ASSERT_EQ(controller->loadStatus(), LoadStatus::Corrupted);
        EXPECT_EQ(controller->removeUnusedImages(), 0U);
        EXPECT_TRUE(store.contains(image));
    }

    TEST_F(CollectionControllerTest, FileFromNewerVersionIsReportedAndLeftUntouched)
    {
        const std::string newerFile = R"({ "schemaVersion": 99, "document": {} })";
        writeTextFile(documentPath(), newerFile);

        const auto controller = openController();

        EXPECT_EQ(controller->loadStatus(), LoadStatus::NewerVersion);
        EXPECT_TRUE(std::filesystem::exists(documentPath()));
    }

    TEST_F(CollectionControllerTest, SavedFileUsesTheEnvelopeOfTheCurrentSchema)
    {
        {
            const auto controller = openController();
            controller->addItem(sampleAlbum());
        }

        std::ifstream file(documentPath());
        const auto root = nlohmann::json::parse(file);

        EXPECT_EQ(root.at("schemaVersion"), kpop::document::COLLECTION_SCHEMA_VERSION);
        EXPECT_TRUE(root.at("document").contains("items"));
    }
}
