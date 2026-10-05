#include "kpop/domain/PartialDate.hpp"

#include <gtest/gtest.h>

namespace
{
    using kpop::domain::PartialDate;

    TEST(PartialDateTest, ParsesYearMonthAndDayPrecision)
    {
        const auto yearOnly = PartialDate::parse("2021");
        ASSERT_TRUE(yearOnly.has_value());
        EXPECT_EQ(yearOnly->year(), 2021);
        EXPECT_FALSE(yearOnly->month().has_value());

        const auto yearMonth = PartialDate::parse("2021-08");
        ASSERT_TRUE(yearMonth.has_value());
        EXPECT_EQ(yearMonth->month(), 8);
        EXPECT_FALSE(yearMonth->day().has_value());

        const auto fullDate = PartialDate::parse("2021-08-23");
        ASSERT_TRUE(fullDate.has_value());
        EXPECT_EQ(fullDate->day(), 23);
    }

    TEST(PartialDateTest, AcceptsSingleDigitMonthAndDay)
    {
        const auto date = PartialDate::parse("2021-8-3");
        ASSERT_TRUE(date.has_value());
        EXPECT_EQ(date->toKey(), "2021-08-03");
    }

    TEST(PartialDateTest, RejectsMalformedText)
    {
        for (const auto* text : { "", "21", "2021-", "2021-13", "2021-00", "2021-02-30", "2021-08-", "2021-08-23-1", "abcd", "2021/08/23", "-2021", "2021-08-x" })
            EXPECT_FALSE(PartialDate::parse(text).has_value()) << text;
    }

    TEST(PartialDateTest, RejectsDayWithoutMonth)
    {
        EXPECT_FALSE(PartialDate::tryCreate(2021, std::nullopt, 5).has_value());
    }

    TEST(PartialDateTest, AcceptsLeapDayOnlyInLeapYears)
    {
        EXPECT_TRUE(PartialDate::tryCreate(2024, 2, 29).has_value());
        EXPECT_FALSE(PartialDate::tryCreate(2023, 2, 29).has_value());
    }

    TEST(PartialDateTest, KeyRoundTripsAtEveryPrecision)
    {
        for (const auto* text : { "2021", "2021-08", "2021-08-23" })
        {
            const auto date = PartialDate::parse(text);
            ASSERT_TRUE(date.has_value()) << text;
            EXPECT_EQ(date->toKey(), text);
        }
    }

    TEST(PartialDateTest, OrdersChronologically)
    {
        const auto earlier = PartialDate::parse("2020-12-31");
        const auto later = PartialDate::parse("2021-01-01");
        ASSERT_TRUE(earlier && later);
        EXPECT_LT(*earlier, *later);
        EXPECT_NE(*earlier, *later);
    }
}
