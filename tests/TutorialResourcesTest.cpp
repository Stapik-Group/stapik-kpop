#include "stapik/tutorial/TutorialLoader.hpp"

#include <gtest/gtest.h>

#include <array>
#include <filesystem>
#include <string>
#include <vector>

namespace
{
    const std::filesystem::path TUTORIAL_DIR = std::filesystem::path(STAPIK_KPOP_RESOURCES_DIR) / "tutorial";
    constexpr std::array LANGUAGE_CODES = { "en", "pl", "de" };

    std::vector<std::string> chapterIds(const std::string& languageCode)
    {
        const auto tutorial = stapik::tutorial::loadTutorial(TUTORIAL_DIR, languageCode);

        std::vector<std::string> ids;
        for (const auto* chapter : tutorial.readingOrder())
            ids.push_back(chapter->id);
        return ids;
    }
}

TEST(TutorialResources, EveryLanguageHasTheSameChapters)
{
    const auto english = chapterIds("en");
    ASSERT_FALSE(english.empty());

    for (const auto* language : LANGUAGE_CODES)
        EXPECT_EQ(chapterIds(language), english) << language;
}

TEST(TutorialResources, ChaptersHaveTitlesAndContent)
{
    for (const auto* language : LANGUAGE_CODES)
    {
        const auto tutorial = stapik::tutorial::loadTutorial(TUTORIAL_DIR, language);
        for (const auto* chapter : tutorial.readingOrder())
        {
            EXPECT_FALSE(chapter->title.empty()) << language << ": " << chapter->id;
            EXPECT_GT(chapter->content.size(), 1U) << language << ": " << chapter->id;
        }
    }
}

TEST(TutorialResources, ChaptersLinkedFromTextExist)
{
    for (const auto* language : LANGUAGE_CODES)
    {
        const auto tutorial = stapik::tutorial::loadTutorial(TUTORIAL_DIR, language);
        for (const auto* chapter : tutorial.readingOrder())
        {
            for (const auto& block : chapter->content)
            {
                for (const auto& span : block.spans)
                {
                    if (span.link.rfind('#', 0) == 0)
                        EXPECT_NE(tutorial.find(span.link.substr(1)), nullptr) << language << ": " << chapter->id << " -> " << span.link;
                }
            }
        }
    }
}
