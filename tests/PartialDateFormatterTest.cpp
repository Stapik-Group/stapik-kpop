#include "kpop/domain/PartialDateFormatter.hpp"

#include <gtest/gtest.h>

namespace
{
    using namespace kpop::domain;

    const auto FULL_DATE = *PartialDate::tryCreate(2021, 8, 3);
    const auto MONTH = *PartialDate::tryCreate(2021, 8);
    const auto YEAR = *PartialDate::tryCreate(2021);

    TEST(PartialDateFormatterTest, PolishAndGermanWriteDayMonthYearWithDots)
    {
        for (const auto* language : { "pl", "de" })
        {
            EXPECT_EQ(formatPartialDate(FULL_DATE, language), "03.08.2021") << language;
            EXPECT_EQ(formatPartialDate(MONTH, language), "08.2021") << language;
            EXPECT_EQ(formatPartialDate(YEAR, language), "2021") << language;
        }
    }

    TEST(PartialDateFormatterTest, EnglishWritesMonthDayYearWithSlashes)
    {
        EXPECT_EQ(formatPartialDate(FULL_DATE, "en"), "08/03/2021");
        EXPECT_EQ(formatPartialDate(MONTH, "en"), "08/2021");
        EXPECT_EQ(formatPartialDate(YEAR, "en"), "2021");
    }

    TEST(PartialDateFormatterTest, UnknownLanguageGetsTheIsoForm)
    {
        EXPECT_EQ(formatPartialDate(FULL_DATE, "ja"), "2021-08-03");
        EXPECT_EQ(formatPartialDate(MONTH, ""), "2021-08");
    }

    TEST(PartialDateFormatterTest, RegionOfTheLanguageIsIgnored)
    {
        EXPECT_EQ(formatPartialDate(FULL_DATE, "pl_PL"), "03.08.2021");
    }

    TEST(PartialDateFormatterTest, FormattedDatesParseBackToTheSameDate)
    {
        for (const auto* language : { "pl", "de", "en", "ja" })
        {
            for (const auto& date : { FULL_DATE, MONTH, YEAR })
                EXPECT_EQ(parsePartialDate(formatPartialDate(date, language), language), date) << language;
        }
    }

    TEST(PartialDateFormatterTest, ParsesDatesWithoutLeadingZeros)
    {
        EXPECT_EQ(parsePartialDate("3.8.2021", "pl"), FULL_DATE);
        EXPECT_EQ(parsePartialDate("8.2021", "pl"), MONTH);
        EXPECT_EQ(parsePartialDate("8/3/2021", "en"), FULL_DATE);
    }

    TEST(PartialDateFormatterTest, TheIsoFormIsUnderstoodInEveryLanguage)
    {
        for (const auto* language : { "pl", "de", "en", "ja" })
            EXPECT_EQ(parsePartialDate("2021-08-03", language), FULL_DATE) << language;
    }

    TEST(PartialDateFormatterTest, OrderAndSeparatorOfAnotherLanguageAreRejected)
    {
        EXPECT_FALSE(parsePartialDate("08/03/2021", "pl").has_value());
        EXPECT_FALSE(parsePartialDate("03.08.2021", "en").has_value());
        EXPECT_FALSE(parsePartialDate("23/08/2021", "en").has_value());
    }

    TEST(PartialDateFormatterTest, MalformedTextIsRejected)
    {
        for (const auto* text : { "", "32.01.2021", "30.02.2021", "1.2021.3", "03.08.21", "03.08.", ".2021", "a.b.cccc", "1-2.2021", "03.08.2021.1" })
            EXPECT_FALSE(parsePartialDate(text, "pl").has_value()) << text;
    }

    TEST(PartialDateFormatterTest, UnknownLanguageReadsOnlyTheIsoForm)
    {
        EXPECT_FALSE(parsePartialDate("03.08.2021", "ja").has_value());
    }
}
