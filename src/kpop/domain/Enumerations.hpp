#pragma once

#include "EnumCatalog.hpp"

#include "stapik/domain/CategoryColor.hpp"

#include <cstddef>

namespace kpop::domain
{
    enum class ItemKind
    {
        Album,
        Photocard,
        Merchandise,
        Clip,
        Lyrics,
        Event
    };

    enum class ItemStatus
    {
        Owned,
        Ordered,
        Wishlist,
        Sold
    };

    enum class ItemCondition
    {
        Unspecified,
        Sealed,
        Mint,
        Good,
        Fair,
        Poor
    };

    enum class ArtistType
    {
        Group,
        Soloist
    };

    enum class AlbumType
    {
        Single,
        Concert,
        LiveCD,
        Mini,
        Full,
        Repackage,
        Compilation,
        Other
    };

    enum class AlbumFormat
    {
        Cd,
        Dvd,
        Nfc,
        Cd_Dvd,
        Cd_2Dvd,
        Vinyl,
        Cassette,
        Digital,
        PlatformAlbum,
        Kit
    };

    enum class MerchandiseType
    {
        Lightstick,
        Apparel,
        Photobook,
        SeasonsGreetings,
        Poster,
        Keyring,
        Stationery,
        Other
    };

    enum class PhotocardOrigin
    {
        AlbumInclusion,
        PreOrderBenefit,
        LuckyDraw,
        Fansign,
        Event,
        Other
    };

    enum class ClipType
    {
        MusicVideo,
        Performance,
        Concert,
        Variety,
        BehindTheScenes,
        Other
    };

    enum class SortOrder
    {
        AddedNewest,
        AddedOldest,
        TitleAscending,
        TitleDescending,
        ArtistAscending,
        ArtistDescending,
        ReleaseDateNewest,
        ReleaseDateOldest
    };

    enum class EventType
    {
        Concert,
        Fansign,
        FanMeeting,
        ListeningParty,
        PopUpStore,
        Other
    };

    inline constexpr std::size_t ITEM_KIND_COUNT = 6;

    inline constexpr auto ITEM_KINDS = makeEnumCatalog(
        "kpop.kind",
        entry(ItemKind::Album, "album"),
        entry(ItemKind::Photocard, "photocard"),
        entry(ItemKind::Merchandise, "merchandise"),
        entry(ItemKind::Clip, "clip"),
        entry(ItemKind::Lyrics, "lyrics"),
        entry(ItemKind::Event, "event"));

    static_assert(ITEM_KINDS.entries().size() == ITEM_KIND_COUNT);

    inline constexpr auto ITEM_STATUSES = makeEnumCatalog(
        "kpop.status",
        entry(ItemStatus::Owned, "owned"),
        entry(ItemStatus::Ordered, "ordered"),
        entry(ItemStatus::Wishlist, "wishlist"),
        entry(ItemStatus::Sold, "sold"));

    inline constexpr auto ITEM_CONDITIONS = makeEnumCatalog(
        "kpop.condition",
        entry(ItemCondition::Unspecified, "unspecified"),
        entry(ItemCondition::Sealed, "sealed"),
        entry(ItemCondition::Mint, "mint"),
        entry(ItemCondition::Good, "good"),
        entry(ItemCondition::Fair, "fair"),
        entry(ItemCondition::Poor, "poor"));

    inline constexpr auto ARTIST_TYPES = makeEnumCatalog(
        "kpop.artistType",
        entry(ArtistType::Group, "group"),
        entry(ArtistType::Soloist, "soloist"));

    inline constexpr auto ALBUM_TYPES = makeEnumCatalog(
        "kpop.albumType",
        entry(AlbumType::Single, "single"),
        entry(AlbumType::Concert, "concert"),
        entry(AlbumType::LiveCD, "livecd"),
        entry(AlbumType::Mini, "mini"),
        entry(AlbumType::Full, "full"),
        entry(AlbumType::Repackage, "repackage"),
        entry(AlbumType::Compilation, "compilation"),
        entry(AlbumType::Other, "other"));

    inline constexpr auto ALBUM_FORMATS = makeEnumCatalog(
        "kpop.albumFormat",
        entry(AlbumFormat::Cd, "cd"),
        entry(AlbumFormat::Dvd, "dvd"),
        entry(AlbumFormat::Nfc, "nfc"),
        entry(AlbumFormat::Cd_Dvd, "cd_dvd"),
        entry(AlbumFormat::Cd_2Dvd, "cd_2dvd"),
        entry(AlbumFormat::Vinyl, "vinyl"),
        entry(AlbumFormat::Cassette, "cassette"),
        entry(AlbumFormat::Digital, "digital"),
        entry(AlbumFormat::PlatformAlbum, "platformAlbum"),
        entry(AlbumFormat::Kit, "kit"));

    inline constexpr auto MERCHANDISE_TYPES = makeEnumCatalog(
        "kpop.merchandiseType",
        entry(MerchandiseType::Lightstick, "lightstick"),
        entry(MerchandiseType::Apparel, "apparel"),
        entry(MerchandiseType::Photobook, "photobook"),
        entry(MerchandiseType::SeasonsGreetings, "seasonsGreetings"),
        entry(MerchandiseType::Poster, "poster"),
        entry(MerchandiseType::Keyring, "keyring"),
        entry(MerchandiseType::Stationery, "stationery"),
        entry(MerchandiseType::Other, "other"));

    inline constexpr auto PHOTOCARD_ORIGINS = makeEnumCatalog(
        "kpop.photocardOrigin",
        entry(PhotocardOrigin::AlbumInclusion, "albumInclusion"),
        entry(PhotocardOrigin::PreOrderBenefit, "preOrderBenefit"),
        entry(PhotocardOrigin::LuckyDraw, "luckyDraw"),
        entry(PhotocardOrigin::Fansign, "fansign"),
        entry(PhotocardOrigin::Event, "event"),
        entry(PhotocardOrigin::Other, "other"));

    inline constexpr auto CLIP_TYPES = makeEnumCatalog(
        "kpop.clipType",
        entry(ClipType::MusicVideo, "musicVideo"),
        entry(ClipType::Performance, "performance"),
        entry(ClipType::Concert, "concert"),
        entry(ClipType::Variety, "variety"),
        entry(ClipType::BehindTheScenes, "behindTheScenes"),
        entry(ClipType::Other, "other"));

    inline constexpr auto EVENT_TYPES = makeEnumCatalog(
        "kpop.eventType",
        entry(EventType::Concert, "concert"),
        entry(EventType::Fansign, "fansign"),
        entry(EventType::FanMeeting, "fanMeeting"),
        entry(EventType::ListeningParty, "listeningParty"),
        entry(EventType::PopUpStore, "popUpStore"),
        entry(EventType::Other, "other"));

    inline constexpr auto SORT_ORDERS = makeEnumCatalog(
        "kpop.sortOrder",
        entry(SortOrder::AddedNewest, "addedNewest"),
        entry(SortOrder::AddedOldest, "addedOldest"),
        entry(SortOrder::TitleAscending, "titleAscending"),
        entry(SortOrder::TitleDescending, "titleDescending"),
        entry(SortOrder::ArtistAscending, "artistAscending"),
        entry(SortOrder::ArtistDescending, "artistDescending"),
        entry(SortOrder::ReleaseDateNewest, "releaseDateNewest"),
        entry(SortOrder::ReleaseDateOldest, "releaseDateOldest"));

    [[nodiscard]] constexpr stapik::domain::CategoryColor colorOf(const ItemKind kind)
    {
        using stapik::domain::CategoryColor;

        switch (kind)
        {
            case ItemKind::Album: return CategoryColor::Blue;
            case ItemKind::Photocard: return CategoryColor::Pink;
            case ItemKind::Merchandise: return CategoryColor::Orange;
            case ItemKind::Clip: return CategoryColor::Red;
            case ItemKind::Lyrics: return CategoryColor::Purple;
            case ItemKind::Event: return CategoryColor::Green;
        }

        return CategoryColor::Gray;
    }
}
