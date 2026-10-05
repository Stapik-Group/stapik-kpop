#include "kpop/domain/Enumerations.hpp"

#include <nlohmann/json.hpp>

#include <gtest/gtest.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <regex>
#include <set>
#include <sstream>
#include <string>

namespace
{
    using namespace kpop::domain;

    const std::filesystem::path RESOURCES_DIR = STAPIK_KPOP_RESOURCES_DIR;
    const std::filesystem::path SOURCE_DIR = STAPIK_KPOP_SOURCE_DIR;
    constexpr std::array LANGUAGE_CODES = { "en", "pl", "de" };

    std::set<std::string> keysOf(const std::string& languageCode)
    {
        std::ifstream file(RESOURCES_DIR / "locales" / (languageCode + ".json"));
        const auto json = nlohmann::json::parse(file);

        std::set<std::string> keys;
        for (const auto& [key, value] : json.items())
        {
            EXPECT_TRUE(value.is_string()) << languageCode << ": " << key;
            EXPECT_FALSE(value.get<std::string>().empty()) << languageCode << ": " << key;
            keys.insert(key);
        }

        return keys;
    }

    template<typename Catalog>
    void collectCatalogKeys(const Catalog& catalog, std::set<std::string>& keys)
    {
        for (const auto& catalogEntry : catalog.entries())
            keys.insert(catalog.nameKey(catalogEntry.value));
    }

    // Keys written as complete string literals in the sources, e.g. translate("kpop.list.empty").
    std::set<std::string> keysUsedInSources()
    {
        const std::regex literalKey(R"regex("(kpop\.[A-Za-z0-9.]+)")regex");
        std::set<std::string> keys;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(SOURCE_DIR))
        {
            // The catalogs only spell the prefix of their keys; they are covered by collectCatalogKeys.
            if (!entry.is_regular_file() || entry.path().filename() == "Enumerations.hpp")
                continue;

            std::ifstream file(entry.path());
            std::stringstream content;
            content << file.rdbuf();
            const auto text = content.str();

            for (std::sregex_iterator match(text.begin(), text.end(), literalKey), end; match != end; ++match)
                keys.insert((*match)[1].str());
        }

        return keys;
    }

    TEST(LocaleResourcesTest, AllLanguagesDefineTheSameKeys)
    {
        const auto englishKeys = keysOf("en");
        ASSERT_FALSE(englishKeys.empty());

        for (const auto* languageCode : LANGUAGE_CODES)
            EXPECT_EQ(keysOf(languageCode), englishKeys) << languageCode;
    }

    TEST(LocaleResourcesTest, EveryValueOfEveryEnumerationHasAName)
    {
        std::set<std::string> requiredKeys;
        collectCatalogKeys(ITEM_KINDS, requiredKeys);
        collectCatalogKeys(ITEM_STATUSES, requiredKeys);
        collectCatalogKeys(ITEM_CONDITIONS, requiredKeys);
        collectCatalogKeys(ARTIST_TYPES, requiredKeys);
        collectCatalogKeys(ALBUM_TYPES, requiredKeys);
        collectCatalogKeys(ALBUM_FORMATS, requiredKeys);
        collectCatalogKeys(MERCHANDISE_TYPES, requiredKeys);
        collectCatalogKeys(PHOTOCARD_ORIGINS, requiredKeys);
        collectCatalogKeys(CLIP_TYPES, requiredKeys);
        collectCatalogKeys(EVENT_TYPES, requiredKeys);

        const auto availableKeys = keysOf("en");
        for (const auto& key : requiredKeys)
            EXPECT_TRUE(availableKeys.contains(key)) << "missing " << key;
    }

    TEST(LocaleResourcesTest, EveryKeyUsedInTheSourcesIsTranslated)
    {
        const auto usedKeys = keysUsedInSources();
        ASSERT_FALSE(usedKeys.empty());

        const auto availableKeys = keysOf("en");
        for (const auto& key : usedKeys)
            EXPECT_TRUE(availableKeys.contains(key)) << "missing " << key;
    }

    TEST(LocaleResourcesTest, PlaceholdersMatchBetweenLanguages)
    {
        const std::regex placeholder(R"(\{[a-zA-Z]+\})");

        const auto placeholdersOf = [&placeholder](const std::string& text)
        {
            std::set<std::string> found;
            for (std::sregex_iterator match(text.begin(), text.end(), placeholder), end; match != end; ++match)
                found.insert(match->str());
            return found;
        };

        std::ifstream englishFile(RESOURCES_DIR / "locales" / "en.json");
        const auto english = nlohmann::json::parse(englishFile);

        for (const auto* languageCode : LANGUAGE_CODES)
        {
            std::ifstream file(RESOURCES_DIR / "locales" / (std::string(languageCode) + ".json"));
            const auto translated = nlohmann::json::parse(file);

            for (const auto& [key, value] : english.items())
                EXPECT_EQ(placeholdersOf(translated.at(key).get<std::string>()), placeholdersOf(value.get<std::string>())) << languageCode << ": " << key;
        }
    }
}
