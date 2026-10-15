#include "ItemDetails.hpp"

#include "stapik/text/TextFolding.hpp"

#include <array>
#include <utility>

namespace kpop::domain
{
    ItemKind kindOf(const ItemDetails& details)
    {
        return static_cast<ItemKind>(details.index());
    }

    Region regionOf(const std::string_view text)
    {
        using enum Region;

        const auto folded = stapik::text::foldForSearch(text);
        if (folded.empty() || folded.find_first_not_of(" \t") == std::string::npos)
            return Unspecified;

        static constexpr std::array<std::pair<std::string_view, Region>, 25> ALIASES{ {
            { "korea", Korea }, { "kr", Korea }, { "south korea", Korea }, { "republic of korea", Korea },
            { "korea pd", Korea }, { "korean", Korea },
            { "japan", Japan }, { "jp", Japan }, { "jpn", Japan }, { "japonia", Japan }, { "japanese", Japan },
            { "china", China }, { "cn", China }, { "chiny", China }, { "chinese", China },
            { "taiwan", Taiwan }, { "tw", Taiwan }, { "tajwan", Taiwan },
            { "usa", Usa }, { "us", Usa }, { "united states", Usa }, { "stany zjednoczone", Usa },
            { "europe", Europe }, { "eu", Europe }, { "europa", Europe } } };

        const auto start = folded.find_first_not_of(" \t");
        const auto end = folded.find_last_not_of(" \t");
        const std::string_view trimmed = std::string_view(folded).substr(start, end - start + 1);

        for (const auto& [alias, region] : ALIASES)
        {
            if (trimmed == alias)
                return region;
        }

        return Other;
    }

    Region regionOf(const AlbumDetails& album)
    {
        return regionOf(album.region);
    }

    std::string regionText(const Region region, const std::string_view otherText)
    {
        switch (region)
        {
            case Region::Unspecified: return {};
            case Region::Other: return std::string(otherText);
            default: return std::string(REGIONS.idOf(region));
        }
    }

    std::string_view regionFlag(const Region region)
    {
        switch (region)
        {
            case Region::Unspecified: return {};
            case Region::Korea: return "🇰🇷";
            case Region::Japan: return "🇯🇵";
            case Region::China: return "🇨🇳";
            case Region::Taiwan: return "🇹🇼";
            case Region::Usa: return "🇺🇸";
            case Region::Europe: return "🇪🇺";
            case Region::Other: return "🌐";
        }

        return {};
    }

    bool hasSize(const MerchandiseType type)
    {
        return type == MerchandiseType::Apparel;
    }

    ItemDetails defaultDetails(const ItemKind kind)
    {
        switch (kind)
        {
            using enum ItemKind;
            case Album: return AlbumDetails{};
            case Photocard: return PhotocardDetails{};
            case Merchandise: return MerchandiseDetails{};
            case Clip: return ClipDetails{};
            case Lyrics: return LyricsDetails{};
            case Event: return EventDetails{};
        }

        return AlbumDetails{};
    }
}
