#include "kpop/domain/ImageId.hpp"

#include <gtest/gtest.h>

#include <string>

namespace
{
    using kpop::domain::ImageId;

    const std::string VALID_HASH = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";

    TEST(ImageIdTest, AcceptsSixtyFourHexDigits)
    {
        const auto id = ImageId::parse(VALID_HASH);

        ASSERT_TRUE(id.has_value());
        EXPECT_EQ(id->hash(), VALID_HASH);
    }

    TEST(ImageIdTest, UpperCaseDigitsAreKeptInLowerCase)
    {
        std::string upperCase = VALID_HASH;
        for (auto& character : upperCase)
        {
            if (character >= 'a' && character <= 'f')
                character = static_cast<char>(character - 'a' + 'A');
        }

        const auto id = ImageId::parse(upperCase);

        ASSERT_TRUE(id.has_value());
        EXPECT_EQ(id->hash(), VALID_HASH);
        EXPECT_EQ(id, ImageId::parse(VALID_HASH));
    }

    TEST(ImageIdTest, RejectsWrongLengthAndNonHexCharacters)
    {
        EXPECT_FALSE(ImageId::parse("").has_value());
        EXPECT_FALSE(ImageId::parse(VALID_HASH.substr(1)).has_value());
        EXPECT_FALSE(ImageId::parse(VALID_HASH + "0").has_value());
        EXPECT_FALSE(ImageId::parse("g" + VALID_HASH.substr(1)).has_value());
        EXPECT_FALSE(ImageId::parse("../" + VALID_HASH.substr(3)).has_value());
    }
}
