#include "kpop/image/ImageEncoder.hpp"
#include "kpop/image/ImageLibrary.hpp"

#include "support/ImageFixtures.hpp"
#include "support/TestFixtures.hpp"

#include "stapik/cloud/CloudAssetProtocol.hpp"

#include <gtest/gtest.h>

#include <filesystem>

namespace
{
    using namespace kpop::image;
    using namespace kpop::test;

    constexpr guint32 OPAQUE_BLUE = 0x336699FF;

    TEST(ImageLibraryTest, ImportedImageIsStoredAndFoundByItsId)
    {
        const TemporaryDirectory directory;
        ASSERT_TRUE(writePng(directory.path() / "cover.png", 300, 200, false, OPAQUE_BLUE));
        ImageLibrary library(directory.path() / "images");

        const auto id = library.importFile(directory.path() / "cover.png");

        const auto path = library.pathOf(id);
        ASSERT_TRUE(path.has_value());
        EXPECT_TRUE(std::filesystem::is_regular_file(*path));
        EXPECT_TRUE(library.store().read(id).has_value());

        const auto info = inspectImage(*path);
        ASSERT_TRUE(info.has_value());
        EXPECT_EQ(info->format, "jpeg");
    }

    TEST(ImageLibraryTest, ImportingTheSameImageTwiceGivesTheSameId)
    {
        const TemporaryDirectory directory;
        ASSERT_TRUE(writePng(directory.path() / "cover.png", 300, 200, false, OPAQUE_BLUE));
        ImageLibrary library(directory.path() / "images");

        EXPECT_EQ(library.importFile(directory.path() / "cover.png"), library.importFile(directory.path() / "cover.png"));
        EXPECT_EQ(library.store().ids().size(), 1U);
    }

    TEST(ImageLibraryTest, ImageThatIsNotStoredHasNoPath)
    {
        const TemporaryDirectory directory;
        const ImageLibrary library(directory.path());
        const auto id = kpop::domain::ImageId::parse(stapik::cloud::sha256Hex({ 'x' }));
        ASSERT_TRUE(id.has_value());

        EXPECT_FALSE(library.pathOf(*id).has_value());
    }

    TEST(ImageLibraryTest, FileThatIsNotAnImageIsNotImported)
    {
        const TemporaryDirectory directory;
        writeTextFile(directory.path() / "notes.png", "not a picture");
        ImageLibrary library(directory.path() / "images");

        EXPECT_THROW(static_cast<void>(library.importFile(directory.path() / "notes.png")), ImageImportError);
        EXPECT_TRUE(library.store().ids().empty());
    }
}
