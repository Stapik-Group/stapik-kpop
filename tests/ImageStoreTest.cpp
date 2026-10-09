#include "kpop/image/ImageStore.hpp"

#include "support/TestFixtures.hpp"

#include "stapik/cloud/CloudAssetProtocol.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

namespace
{
    using namespace kpop::image;
    using kpop::domain::ImageId;
    using kpop::test::TemporaryDirectory;
    using kpop::test::writeTextFile;

    ImageBytes bytesOf(const std::string& text)
    {
        return ImageBytes(text.begin(), text.end());
    }

    TEST(ImageStoreTest, AddedImageIsNamedAfterItsContent)
    {
        const TemporaryDirectory directory;
        ImageStore store(directory.path() / "images");
        const auto content = bytesOf("cover");

        const auto id = store.add(content);

        EXPECT_EQ(id.hash(), stapik::cloud::sha256Hex(content));
        EXPECT_TRUE(store.contains(id));
        EXPECT_TRUE(std::filesystem::is_regular_file(directory.path() / "images" / (id.hash() + ".jpg")));
        EXPECT_EQ(store.read(id), content);
    }

    TEST(ImageStoreTest, AddingTheSameContentTwiceKeepsOneFile)
    {
        const TemporaryDirectory directory;
        ImageStore store(directory.path());

        const auto first = store.add(bytesOf("cover"));
        const auto second = store.add(bytesOf("cover"));

        EXPECT_EQ(first, second);
        EXPECT_EQ(store.ids().size(), 1U);
    }

    TEST(ImageStoreTest, EmptyContentIsRejected)
    {
        const TemporaryDirectory directory;
        ImageStore store(directory.path());

        EXPECT_THROW(static_cast<void>(store.add({})), std::invalid_argument);
    }

    TEST(ImageStoreTest, MissingImageIsNotFound)
    {
        const TemporaryDirectory directory;
        const ImageStore store(directory.path());
        const auto id = ImageId::parse(stapik::cloud::sha256Hex(bytesOf("nothing")));
        ASSERT_TRUE(id.has_value());

        EXPECT_FALSE(store.contains(*id));
        EXPECT_FALSE(store.read(*id).has_value());
    }

    TEST(ImageStoreTest, DamagedFileIsNotReadAndIsReplacedWhenAddedAgain)
    {
        const TemporaryDirectory directory;
        ImageStore store(directory.path());
        const auto content = bytesOf("cover");
        const auto id = store.add(content);

        writeTextFile(store.pathOf(id), "damaged");

        EXPECT_TRUE(store.contains(id));
        EXPECT_FALSE(store.read(id).has_value());

        EXPECT_EQ(store.add(content), id);
        EXPECT_EQ(store.read(id), content);
    }

    TEST(ImageStoreTest, ListsOnlyImagesOfTheStoreSortedById)
    {
        const TemporaryDirectory directory;
        ImageStore store(directory.path());
        const auto first = store.add(bytesOf("one"));
        const auto second = store.add(bytesOf("two"));
        writeTextFile(directory.path() / "notes.txt", "not an image");
        writeTextFile(directory.path() / "abc.jpg", "wrong name");

        const auto ids = store.ids();

        ASSERT_EQ(ids.size(), 2U);
        EXPECT_LT(ids[0], ids[1]);
        EXPECT_TRUE((ids[0] == first && ids[1] == second) || (ids[0] == second && ids[1] == first));
    }

    TEST(ImageStoreTest, ListOfMissingDirectoryIsEmpty)
    {
        const TemporaryDirectory directory;
        const ImageStore store(directory.path() / "does-not-exist");

        EXPECT_TRUE(store.ids().empty());
    }

    TEST(ImageStoreTest, RemoveReportsWhetherThereWasAnythingToRemove)
    {
        const TemporaryDirectory directory;
        ImageStore store(directory.path());
        const auto id = store.add(bytesOf("cover"));

        EXPECT_TRUE(store.remove(id));
        EXPECT_FALSE(store.contains(id));
        EXPECT_FALSE(store.remove(id));
    }

    TEST(ImageStoreTest, FileNameRoundTripsAndRejectsForeignNames)
    {
        const auto id = ImageId::parse(stapik::cloud::sha256Hex(bytesOf("cover")));
        ASSERT_TRUE(id.has_value());

        EXPECT_EQ(ImageStore::fileName(*id), id->hash() + ".jpg");
        EXPECT_EQ(ImageStore::idFromFileName(ImageStore::fileName(*id)), id);

        EXPECT_FALSE(ImageStore::idFromFileName(id->hash()).has_value());
        EXPECT_FALSE(ImageStore::idFromFileName(id->hash() + ".png").has_value());
        EXPECT_FALSE(ImageStore::idFromFileName("cover.jpg").has_value());
    }
}
