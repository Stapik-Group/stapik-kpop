#include "kpop/image/ImageArchiveStorage.hpp"

#include "support/TestFixtures.hpp"

#include "stapik/cloud/CloudAssetProtocol.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

namespace
{
    using namespace kpop::image;
    using kpop::test::TemporaryDirectory;

    ImageBytes bytesOf(const std::string& text)
    {
        return ImageBytes(text.begin(), text.end());
    }

    class ImageArchiveStorageTest : public ::testing::Test
    {
    protected:
        TemporaryDirectory directory;
        ImageStore covers{ directory.path() / "covers" };
        ImageStore photos{ directory.path() / "photos" };
        ImageArchiveStorage storage{ covers, photos };
    };

    TEST_F(ImageArchiveStorageTest, ListsCoversAndPhotosUnderTheirOwnPrefixes)
    {
        const auto cover = covers.add(bytesOf("cover"));
        const auto photo = photos.add(bytesOf("photo"));

        const auto assets = storage.listAssets();

        ASSERT_EQ(assets.size(), 2U);
        const auto find = [&assets](const std::string& name)
        {
            return std::ranges::find(assets, name, &stapik::cloud::AssetInfo::filename);
        };
        const auto coverAsset = find("covers-" + ImageStore::fileName(cover));
        const auto photoAsset = find("photos-" + ImageStore::fileName(photo));
        ASSERT_NE(coverAsset, assets.end());
        ASSERT_NE(photoAsset, assets.end());
        EXPECT_EQ(coverAsset->checksumSha256, cover.hash());
        EXPECT_EQ(coverAsset->sizeBytes, 5);
        EXPECT_EQ(coverAsset->mimeType, "image/jpeg");
    }

    TEST_F(ImageArchiveStorageTest, TheSameImageInBothLibrariesIsListedTwice)
    {
        static_cast<void>(covers.add(bytesOf("same")));
        static_cast<void>(photos.add(bytesOf("same")));

        EXPECT_EQ(storage.listAssets().size(), 2U);
    }

    TEST_F(ImageArchiveStorageTest, DownloadsTheContentOfTheRightLibrary)
    {
        const auto cover = covers.add(bytesOf("cover"));

        EXPECT_EQ(storage.downloadAsset("covers-" + ImageStore::fileName(cover)), bytesOf("cover"));
        EXPECT_FALSE(storage.downloadAsset("photos-" + ImageStore::fileName(cover)));
    }

    TEST_F(ImageArchiveStorageTest, UploadStoresTheImageInTheLibraryOfItsPrefix)
    {
        const auto content = bytesOf("photo");
        const auto id = *kpop::domain::ImageId::parse(stapik::cloud::sha256Hex(content));

        const auto info = storage.uploadAsset("photos-" + ImageStore::fileName(id), "image/jpeg", content);

        EXPECT_EQ(info.checksumSha256, id.hash());
        EXPECT_TRUE(photos.contains(id));
        EXPECT_FALSE(covers.contains(id));
    }

    TEST_F(ImageArchiveStorageTest, UploadRefusesContentThatDoesNotMatchTheName)
    {
        const auto id = *kpop::domain::ImageId::parse(stapik::cloud::sha256Hex(bytesOf("photo")));

        EXPECT_THROW(
            static_cast<void>(storage.uploadAsset("photos-" + ImageStore::fileName(id), "image/jpeg", bytesOf("another"))),
            ::CloudStorageException);
        EXPECT_FALSE(photos.contains(id));
    }

    TEST_F(ImageArchiveStorageTest, RefusesNamesThatAreNotImages)
    {
        for (const auto* name : { "cover.jpg", "covers-x.jpg", "other-0000.jpg", "" })
            EXPECT_THROW(static_cast<void>(storage.downloadAsset(name)), stapik::cloud::InvalidAssetFilenameException) << name;
    }

    TEST_F(ImageArchiveStorageTest, DeleteRemovesOnlyFromItsLibrary)
    {
        const auto cover = covers.add(bytesOf("same"));
        static_cast<void>(photos.add(bytesOf("same")));

        EXPECT_TRUE(storage.deleteAsset("covers-" + ImageStore::fileName(cover)));

        EXPECT_FALSE(covers.contains(cover));
        EXPECT_TRUE(photos.contains(cover));
        EXPECT_FALSE(storage.deleteAsset("covers-" + ImageStore::fileName(cover)));
    }
}
