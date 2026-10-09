#include "kpop/image/ImageSynchronizer.hpp"

#include "support/FakeAssetStorage.hpp"
#include "support/TestFixtures.hpp"

#include "stapik/cloud/CloudAssetProtocol.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace
{
    using namespace kpop::image;
    using kpop::domain::ImageId;
    using kpop::test::FakeAssetStorage;
    using kpop::test::TemporaryDirectory;
    using kpop::test::writeTextFile;

    using Ids = std::vector<ImageId>;

    ImageBytes bytesOf(const std::string& text)
    {
        return ImageBytes(text.begin(), text.end());
    }

    ImageId idOf(const ImageBytes& content)
    {
        return *ImageId::parse(stapik::cloud::sha256Hex(content));
    }

    // Puts a file in the fake cloud under the name an image of the store would have in it.
    void putInCloud(FakeAssetStorage& storage, const ImageBytes& content)
    {
        storage.files[ImageStore::fileName(idOf(content))] = FakeAssetStorage::File{ std::string(ImageStore::MIME_TYPE), content, std::nullopt };
    }

    TEST(ImageSyncPlanTest, UploadsWhatOnlyThisComputerHasAndDownloadsWhatOnlyTheCloudHas)
    {
        const auto a = idOf(bytesOf("a"));
        const auto b = idOf(bytesOf("b"));
        const auto c = idOf(bytesOf("c"));
        const auto unrelated = idOf(bytesOf("unrelated"));

        const auto plan = planImageSync({ a, b, c }, { a, b }, { b, c, unrelated });

        EXPECT_EQ(plan.toUpload, Ids{ a });
        EXPECT_EQ(plan.toDownload, Ids{ c });
    }

    TEST(ImageSyncPlanTest, ImagesThatAreMissingEverywhereAreLeftAlone)
    {
        const auto a = idOf(bytesOf("a"));

        const auto plan = planImageSync({ a }, {}, {});

        EXPECT_TRUE(plan.toUpload.empty());
        EXPECT_TRUE(plan.toDownload.empty());
    }

    TEST(ImageSyncPlanTest, RepeatedIdsAreCollapsedAndTheResultIsSorted)
    {
        const auto a = idOf(bytesOf("a"));
        const auto b = idOf(bytesOf("b"));
        const auto [first, second] = a < b ? std::pair{ a, b } : std::pair{ b, a };

        const auto plan = planImageSync({ second, first, second }, { second, first, first }, {});

        EXPECT_EQ(plan.toUpload, (Ids{ first, second }));
    }

    class ImageSynchronizerTest : public testing::Test
    {
    protected:
        [[nodiscard]] ImageId addLocally(const std::string& text)
        {
            return m_store.add(bytesOf(text));
        }

        TemporaryDirectory m_directory;
        ImageStore m_store{ m_directory.path() };
        FakeAssetStorage m_cloud;
    };

    TEST_F(ImageSynchronizerTest, UploadsAnImageTheCloudLacks)
    {
        const auto id = addLocally("cover");

        const auto report = synchronizeImages(m_cloud, m_store, { id });

        EXPECT_FALSE(report.failed);
        EXPECT_EQ(report.uploaded, 1);
        ASSERT_TRUE(m_cloud.files.contains(ImageStore::fileName(id)));
        EXPECT_EQ(m_cloud.files.at(ImageStore::fileName(id)).mimeType, "image/jpeg");
        EXPECT_EQ(m_cloud.files.at(ImageStore::fileName(id)).content, bytesOf("cover"));
    }

    TEST_F(ImageSynchronizerTest, DownloadsAnImageThisComputerLacks)
    {
        const auto content = bytesOf("cover");
        putInCloud(m_cloud, content);
        const auto id = idOf(content);

        const auto report = synchronizeImages(m_cloud, m_store, { id });

        EXPECT_FALSE(report.failed);
        EXPECT_EQ(report.downloaded, 1);
        EXPECT_EQ(m_store.read(id), content);
    }

    TEST_F(ImageSynchronizerTest, DoesNothingWhenBothSidesAgree)
    {
        const auto content = bytesOf("cover");
        const auto id = m_store.add(content);
        putInCloud(m_cloud, content);

        const auto report = synchronizeImages(m_cloud, m_store, { id });

        EXPECT_FALSE(report.failed);
        EXPECT_EQ(report.uploaded, 0);
        EXPECT_EQ(report.downloaded, 0);
        EXPECT_EQ(report.skipped, 0);
        EXPECT_EQ(m_cloud.uploadCalls, 0);
        EXPECT_EQ(m_cloud.downloadCalls, 0);
    }

    TEST_F(ImageSynchronizerTest, ImagesNoEntryUsesAreNeitherUploadedNorDownloaded)
    {
        const auto unusedLocal = addLocally("local only");
        const auto unusedRemote = bytesOf("cloud only");
        putInCloud(m_cloud, unusedRemote);

        const auto report = synchronizeImages(m_cloud, m_store, {});

        EXPECT_EQ(report.uploaded, 0);
        EXPECT_EQ(report.downloaded, 0);
        EXPECT_FALSE(m_cloud.files.contains(ImageStore::fileName(unusedLocal)));
        EXPECT_FALSE(m_store.contains(idOf(unusedRemote)));
    }

    TEST_F(ImageSynchronizerTest, CloudFileWhoseContentDoesNotMatchItsNameIsReplaced)
    {
        const auto content = bytesOf("cover");
        const auto id = m_store.add(content);
        m_cloud.files[ImageStore::fileName(id)] = FakeAssetStorage::File{ std::string(ImageStore::MIME_TYPE), bytesOf("broken"), std::nullopt };

        const auto report = synchronizeImages(m_cloud, m_store, { id });

        EXPECT_EQ(report.uploaded, 1);
        EXPECT_EQ(m_cloud.files.at(ImageStore::fileName(id)).content, content);
    }

    TEST_F(ImageSynchronizerTest, ImageTooLargeForTheSlotIsSkippedAndTheOthersAreStillUploaded)
    {
        const auto small = addLocally("ok");
        const auto large = addLocally(std::string(100, 'x'));
        m_cloud.maxFileSize = 10;

        const auto report = synchronizeImages(m_cloud, m_store, { small, large });

        EXPECT_FALSE(report.failed);
        EXPECT_EQ(report.uploaded, 1);
        EXPECT_EQ(report.skipped, 1);
        EXPECT_TRUE(m_cloud.files.contains(ImageStore::fileName(small)));
        EXPECT_FALSE(m_cloud.files.contains(ImageStore::fileName(large)));
    }

    TEST_F(ImageSynchronizerTest, DamagedLocalFileIsNotUploaded)
    {
        const auto id = addLocally("cover");
        writeTextFile(m_store.pathOf(id), "damaged");

        const auto report = synchronizeImages(m_cloud, m_store, { id });

        EXPECT_FALSE(report.failed);
        EXPECT_EQ(report.skipped, 1);
        EXPECT_EQ(m_cloud.uploadCalls, 0);
    }

    TEST_F(ImageSynchronizerTest, DownloadWhoseContentDoesNotMatchItsIdIsRejected)
    {
        const auto wanted = idOf(bytesOf("cover"));
        m_cloud.files[ImageStore::fileName(wanted)] = FakeAssetStorage::File{ std::string(ImageStore::MIME_TYPE), bytesOf("something else"), wanted.hash() };

        const auto report = synchronizeImages(m_cloud, m_store, { wanted });

        EXPECT_FALSE(report.failed);
        EXPECT_EQ(report.downloaded, 0);
        EXPECT_EQ(report.skipped, 1);
        EXPECT_FALSE(m_store.contains(wanted));
    }

    TEST_F(ImageSynchronizerTest, FileThatDisappearedFromTheCloudIsSkipped)
    {
        const auto content = bytesOf("cover");
        putInCloud(m_cloud, content);
        const auto id = idOf(content);
        m_cloud.vanishingFiles.insert(ImageStore::fileName(id));

        const auto report = synchronizeImages(m_cloud, m_store, { id });

        EXPECT_FALSE(report.failed);
        EXPECT_EQ(report.skipped, 1);
        EXPECT_FALSE(m_store.contains(id));
    }

    TEST_F(ImageSynchronizerTest, FailingListingFailsTheRunAndChangesNothing)
    {
        const auto id = addLocally("cover");
        m_cloud.failListing = true;

        const auto report = synchronizeImages(m_cloud, m_store, { id });

        EXPECT_TRUE(report.failed);
        EXPECT_FALSE(report.failure.empty());
        EXPECT_EQ(m_cloud.uploadCalls, 0);
        EXPECT_TRUE(m_store.contains(id));
    }

    TEST_F(ImageSynchronizerTest, RefusedUploadFailsTheRunAndStopsAtTheFirstImage)
    {
        const auto first = addLocally("one");
        const auto second = addLocally("two");
        m_cloud.failUploads = true;

        const auto report = synchronizeImages(m_cloud, m_store, { first, second });

        EXPECT_TRUE(report.failed);
        EXPECT_EQ(m_cloud.uploadCalls, 1);
    }
}
