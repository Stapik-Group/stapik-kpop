#include "kpop/domain/Tracklist.hpp"

#include <gtest/gtest.h>

namespace
{
    using namespace kpop::domain;

    TEST(TrackLengthTest, ParsesMinutesAndSeconds)
    {
        for (const auto* text : { "3:45", "03:45", "45:00" })
            EXPECT_TRUE(TrackLength::parse(text).has_value()) << text;

        EXPECT_EQ(TrackLength::parse("3:45")->seconds(), 225);
        EXPECT_EQ(TrackLength::parse("1:02:03")->seconds(), 3723);
    }

    TEST(TrackLengthTest, RejectsMalformedText)
    {
        for (const auto* text : { "", "3", "3:5", "3:60", "a:bc", "1:2:3:4", "-1:30", ":30", "3:45:", "1:60:00", "0:00" })
            EXPECT_FALSE(TrackLength::parse(text).has_value()) << text;
    }

    TEST(TrackLengthTest, FormatsAsMinutesOrHours)
    {
        EXPECT_EQ(TrackLength::fromSeconds(225)->toText(), "3:45");
        EXPECT_EQ(TrackLength::fromSeconds(65)->toText(), "1:05");
        EXPECT_EQ(TrackLength::fromSeconds(3723)->toText(), "1:02:03");
    }

    TEST(TrackLengthTest, FormattedTextParsesBackToTheSameLength)
    {
        for (const int seconds : { 1, 59, 60, 225, 3599, 3600, 3723 })
        {
            const auto length = TrackLength::fromSeconds(seconds);
            ASSERT_TRUE(length.has_value());
            EXPECT_EQ(TrackLength::parse(length->toText()), length) << seconds;
        }
    }

    TEST(TrackLengthTest, RejectsOutOfRangeSeconds)
    {
        EXPECT_FALSE(TrackLength::fromSeconds(0).has_value());
        EXPECT_FALSE(TrackLength::fromSeconds(-5).has_value());
        EXPECT_FALSE(TrackLength::fromSeconds(100 * 3600).has_value());
    }

    TEST(TracklistTest, TotalLengthSumsEveryTrack)
    {
        const Tracklist tracklist = {
            { .title = "A", .writers = "", .length = TrackLength::fromSeconds(65), .titleTrack = false, .disc = 1 },
            { .title = "B", .writers = "", .length = TrackLength::fromSeconds(201), .titleTrack = true, .disc = 1 } };

        const auto total = totalLength(tracklist);
        ASSERT_TRUE(total.has_value());
        EXPECT_EQ(total->seconds(), 266);
    }

    TEST(TracklistTest, TotalLengthIsUnknownWhenATrackHasNoLength)
    {
        const Tracklist tracklist = {
            { .title = "A", .writers = "", .length = TrackLength::fromSeconds(65), .titleTrack = false, .disc = 1 },
            { .title = "B", .writers = "", .length = std::nullopt, .titleTrack = false, .disc = 1 } };

        EXPECT_FALSE(totalLength(tracklist).has_value());
    }

    TEST(TracklistTest, TotalLengthIsUnknownForEmptyTracklist)
    {
        EXPECT_FALSE(totalLength({}).has_value());
    }

    TEST(TracklistTest, TotalLengthOfADiscCountsOnlyItsTracks)
    {
        const Tracklist tracklist = {
            { .title = "A", .writers = "", .length = TrackLength::fromSeconds(60), .titleTrack = false, .disc = 1 },
            { .title = "B", .writers = "", .length = TrackLength::fromSeconds(100), .titleTrack = false, .disc = 2 },
            { .title = "C", .writers = "", .length = std::nullopt, .titleTrack = false, .disc = 3 } };

        EXPECT_EQ(totalLength(tracklist, 1)->seconds(), 60);
        EXPECT_EQ(totalLength(tracklist, 2)->seconds(), 100);
        EXPECT_FALSE(totalLength(tracklist, 3).has_value());
        EXPECT_FALSE(totalLength(tracklist, 4).has_value());
        EXPECT_FALSE(totalLength(tracklist).has_value());
    }

    TEST(TracklistTest, DiscCountIsTheLastDisc)
    {
        EXPECT_EQ(discCount({}), 0);

        const Tracklist tracklist = { { .title = "A", .disc = 1 }, { .title = "B", .disc = 2 } };
        EXPECT_EQ(discCount(tracklist), 2);
    }

    TEST(TracklistTest, NormalizingKeepsAnOrderedTracklistAsItIs)
    {
        Tracklist tracklist = { { .title = "A", .disc = 1 }, { .title = "B", .disc = 1 }, { .title = "C", .disc = 2 } };
        const auto expected = tracklist;

        normalizeDiscs(tracklist);
        EXPECT_EQ(tracklist, expected);
    }

    TEST(TracklistTest, NormalizingClosesGapsAndGroupsTheDiscs)
    {
        Tracklist tracklist = {
            { .title = "A", .disc = 5 }, { .title = "B", .disc = 2 }, { .title = "C", .disc = 5 }, { .title = "D", .disc = 0 } };

        normalizeDiscs(tracklist);

        ASSERT_EQ(tracklist.size(), 4U);
        EXPECT_EQ(tracklist[0].title, "D");
        EXPECT_EQ(tracklist[0].disc, 1);
        EXPECT_EQ(tracklist[1].title, "B");
        EXPECT_EQ(tracklist[1].disc, 2);
        EXPECT_EQ(tracklist[2].title, "A");
        EXPECT_EQ(tracklist[2].disc, 3);
        EXPECT_EQ(tracklist[3].title, "C");
        EXPECT_EQ(tracklist[3].disc, 3);
    }
}
