#pragma once

#include "Enumerations.hpp"
#include "PartialDate.hpp"
#include "Tracklist.hpp"

#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace kpop::domain
{
    struct AlbumDetails
    {
        AlbumType type = AlbumType::Mini;
        AlbumFormat format = AlbumFormat::Cd;
        std::string edition;
        std::optional<PartialDate> releaseDate;
        std::string label;
        std::string region;
        std::string catalogNumber;
        std::string inclusions;
        Tracklist tracklist;

        bool operator==(const AlbumDetails&) const = default;
    };

    struct PhotocardDetails
    {
        std::string member;
        PhotocardOrigin origin = PhotocardOrigin::AlbumInclusion;
        std::string source;
        bool forTrade = false;

        bool operator==(const PhotocardDetails&) const = default;
    };

    struct MerchandiseDetails
    {
        MerchandiseType type = MerchandiseType::Lightstick;
        std::string version;
        bool official = true;
        std::string size;

        bool operator==(const MerchandiseDetails&) const = default;
    };

    // Only some kinds of merchandise have a size; it stays empty for the others.
    [[nodiscard]] bool hasSize(MerchandiseType type);

    // Offered as shortcuts, any other size can be typed in.
    inline constexpr std::array<std::string_view, 7> APPAREL_SIZE_SUGGESTIONS{ "XS", "S", "M", "L", "XL", "XXL", "3XL" };

    struct ClipDetails
    {
        ClipType type = ClipType::MusicVideo;
        std::string platform;
        std::string url;
        std::optional<PartialDate> releaseDate;
        std::string albumTitle;
        bool watched = false;

        bool operator==(const ClipDetails&) const = default;
    };

    struct LyricsDetails
    {
        std::string albumTitle;
        std::string writers;
        std::string originalText;
        std::string romanizedText;
        std::string translatedText;
        std::string translationLanguage;

        bool operator==(const LyricsDetails&) const = default;
    };

    struct EventDetails
    {
        EventType type = EventType::Concert;
        std::optional<PartialDate> date;
        std::string venue;
        std::string city;
        std::string seat;

        bool operator==(const EventDetails&) const = default;
    };

    using ItemDetails = std::variant<AlbumDetails, PhotocardDetails, MerchandiseDetails, ClipDetails, LyricsDetails, EventDetails>;

    static_assert(std::variant_size_v<ItemDetails> == ITEM_KIND_COUNT);

    [[nodiscard]] ItemKind kindOf(const ItemDetails& details);
    [[nodiscard]] ItemDetails defaultDetails(ItemKind kind);
}
