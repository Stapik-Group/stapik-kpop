#include "kpop/image/ImageEncoder.hpp"

#include "support/ImageFixtures.hpp"
#include "support/TestFixtures.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

namespace
{
    using namespace kpop::image;
    using namespace kpop::test;

    constexpr guint32 OPAQUE_BLUE = 0x336699FF;
    constexpr guint32 FULLY_TRANSPARENT = 0x00000000;

    class ImageEncoderTest : public testing::Test
    {
    protected:
        [[nodiscard]] std::filesystem::path path(const std::string& name) const
        {
            return m_directory.path() / name;
        }

        // The prepared image as gdk-pixbuf sees it.
        [[nodiscard]] std::optional<ImageInfo> prepareAndInspect(const std::filesystem::path& source)
        {
            const auto prepared = prepareImage(source);
            writeBytes(path("prepared.jpg"), prepared);
            return inspectImage(path("prepared.jpg"));
        }

    private:
        TemporaryDirectory m_directory;
    };

    TEST_F(ImageEncoderTest, LargeImageIsShrunkToFitAndKeepsItsProportions)
    {
        ASSERT_TRUE(writePng(path("large.png"), 2000, 1000, false, OPAQUE_BLUE));

        const auto info = prepareAndInspect(path("large.png"));

        ASSERT_TRUE(info.has_value());
        EXPECT_EQ(info->format, "jpeg");
        EXPECT_EQ(info->width, MAX_IMAGE_DIMENSION);
        EXPECT_EQ(info->height, MAX_IMAGE_DIMENSION / 2);
    }

    TEST_F(ImageEncoderTest, PhotosKeepMoreDetailThanCovers)
    {
        ASSERT_TRUE(writePng(path("large.png"), 3000, 1500, false, OPAQUE_BLUE));

        writeBytes(path("photo.jpg"), prepareImage(path("large.png"), MAX_PHOTO_DIMENSION));
        const auto info = inspectImage(path("photo.jpg"));

        ASSERT_TRUE(info.has_value());
        EXPECT_EQ(info->width, MAX_PHOTO_DIMENSION);
        EXPECT_EQ(info->height, MAX_PHOTO_DIMENSION / 2);
    }

    TEST_F(ImageEncoderTest, SmallImageIsNotEnlarged)
    {
        ASSERT_TRUE(writePng(path("small.png"), 100, 50, false, OPAQUE_BLUE));

        const auto info = prepareAndInspect(path("small.png"));

        ASSERT_TRUE(info.has_value());
        EXPECT_EQ(info->format, "jpeg");
        EXPECT_EQ(info->width, 100);
        EXPECT_EQ(info->height, 50);
    }

    TEST_F(ImageEncoderTest, TransparentPixelsBecomeWhite)
    {
        ASSERT_TRUE(writePng(path("transparent.png"), 64, 64, true, FULLY_TRANSPARENT));

        writeBytes(path("prepared.jpg"), prepareImage(path("transparent.png")));

        GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(path("prepared.jpg").string().c_str(), nullptr);
        ASSERT_NE(pixbuf, nullptr);

        const guchar* pixel = gdk_pixbuf_get_pixels(pixbuf);
        EXPECT_GT(pixel[0], 240);
        EXPECT_GT(pixel[1], 240);
        EXPECT_GT(pixel[2], 240);
        g_object_unref(pixbuf);
    }

    TEST_F(ImageEncoderTest, SameImageIsPreparedToSameBytes)
    {
        ASSERT_TRUE(writePng(path("cover.png"), 300, 300, false, OPAQUE_BLUE));

        EXPECT_EQ(prepareImage(path("cover.png")), prepareImage(path("cover.png")));
    }

    TEST_F(ImageEncoderTest, FileThatIsNotAnImageIsRejected)
    {
        writeTextFile(path("notes.png"), "this is not a picture");

        EXPECT_THROW(static_cast<void>(prepareImage(path("notes.png"))), ImageImportError);
    }

    TEST_F(ImageEncoderTest, MissingFileIsRejected)
    {
        EXPECT_THROW(static_cast<void>(prepareImage(path("missing.png"))), ImageImportError);
    }
}
