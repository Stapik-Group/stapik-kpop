#include "CollectionSerializer.hpp"

#include <format>

#include "stapik/domain/CurrencyCatalog.hpp"

#include <stdexcept>
#include <string>
#include <variant>

namespace kpop::document::serializer
{
    namespace
    {
        using nlohmann::json;

        template<typename Catalog>
        auto requiredEnum(const Catalog& catalog, const json& source, const char* key)
        {
            const auto id = source.at(key).get<std::string>();
            const auto value = catalog.fromId(id);
            if (!value)
                throw std::invalid_argument("Unknown value '" + id + "' for '" + key + "'");

            return *value;
        }

        template<typename Catalog, typename EnumType>
        EnumType enumOr(const Catalog& catalog, const json& source, const char* key, const EnumType fallback)
        {
            if (!source.contains(key))
                return fallback;

            return requiredEnum(catalog, source, key);
        }

        std::string stringOrEmpty(const json& source, const char* key)
        {
            return source.value(key, std::string());
        }

        void setDate(json& target, const char* key, const std::optional<domain::PartialDate>& date)
        {
            if (date)
                target[key] = date->toKey();
        }

        std::optional<domain::PartialDate> optionalDate(const json& source, const char* key)
        {
            if (!source.contains(key) || source.at(key).is_null())
                return std::nullopt;

            const auto text = source.at(key).get<std::string>();
            const auto date = domain::PartialDate::parse(text);
            if (!date)
                throw std::invalid_argument("Invalid date '" + text + "' for '" + key + "'");

            return date;
        }

        json moneyToJson(const stapik::domain::Money& money)
        {
            return {
                { "minorUnits", money.minorUnits() },
                { "currency", money.currency().code },
                { "decimalPlaces", money.currency().decimalPlaces }
            };
        }

        stapik::domain::Money moneyFromJson(const json& source)
        {
            const auto code = source.at("currency").get<std::string>();
            const auto decimalPlaces = source.at("decimalPlaces").get<int>();
            if (decimalPlaces < 0 || decimalPlaces > stapik::domain::MAX_DECIMAL_PLACES)
                throw std::invalid_argument("Invalid decimal places for currency " + code);

            const auto* knownCurrency = stapik::domain::CurrencyCatalog::instance().find(code);
            auto currency = knownCurrency != nullptr
                ? *knownCurrency
                : stapik::domain::Currency{ .code = code, .symbol = code, .symbolBeforeAmount = false, .decimalPlaces = decimalPlaces };
            currency.decimalPlaces = decimalPlaces;

            return {source.at("minorUnits").get<std::int64_t>(), std::move(currency)};
        }

        json trackToJson(const domain::Track& track)
        {
            json result = {
                { "title", track.title },
                { "writers", track.writers },
                { "titleTrack", track.titleTrack }
            };

            if (track.length)
                result["length"] = track.length->seconds();

            return result;
        }

        domain::Track trackFromJson(const json& source)
        {
            domain::Track track;

            // Tracklists used to be saved as plain titles.
            if (source.is_string())
            {
                track.title = source.get<std::string>();
                return track;
            }

            track.title = source.at("title").get<std::string>();
            track.writers = stringOrEmpty(source, "writers");
            track.titleTrack = source.value("titleTrack", false);

            if (source.contains("length") && !source.at("length").is_null())
            {
                const auto seconds = source.at("length").get<int>();
                track.length = domain::TrackLength::fromSeconds(seconds);
                if (!track.length)
                    throw std::invalid_argument(std::format("Invalid track length {}", std::to_string(seconds)));
            }

            return track;
        }

        json tracklistToJson(const domain::Tracklist& tracklist)
        {
            auto tracks = json::array();
            for (const auto& track : tracklist)
                tracks.push_back(trackToJson(track));

            return tracks;
        }

        domain::Tracklist tracklistFromJson(const json& source, const char* key)
        {
            domain::Tracklist tracklist;
            if (!source.contains(key))
                return tracklist;

            const auto& tracks = source.at(key);
            if (!tracks.is_array())
                throw std::invalid_argument(std::string("Expected a list for '") + key + "'");

            for (const auto& track : tracks)
                tracklist.push_back(trackFromJson(track));

            return tracklist;
        }

        json detailsToJson(const domain::AlbumDetails& album)
        {
            json details = {
                { "type", std::string(domain::ALBUM_TYPES.idOf(album.type)) },
                { "format", std::string(domain::ALBUM_FORMATS.idOf(album.format)) },
                { "edition", album.edition },
                { "label", album.label },
                { "region", album.region },
                { "catalogNumber", album.catalogNumber },
                { "inclusions", album.inclusions }
            };
            setDate(details, "releaseDate", album.releaseDate);

            if (!album.tracklist.empty())
                details["tracklist"] = tracklistToJson(album.tracklist);

            return details;
        }

        json detailsToJson(const domain::PhotocardDetails& photocard)
        {
            return {
                { "member", photocard.member },
                { "origin", std::string(domain::PHOTOCARD_ORIGINS.idOf(photocard.origin)) },
                { "source", photocard.source },
                { "forTrade", photocard.forTrade }
            };
        }

        json detailsToJson(const domain::MerchandiseDetails& merchandise)
        {
            json details = {
                { "type", std::string(domain::MERCHANDISE_TYPES.idOf(merchandise.type)) },
                { "version", merchandise.version },
                { "official", merchandise.official }
            };

            if (!merchandise.size.empty())
                details["size"] = merchandise.size;

            return details;
        }

        json detailsToJson(const domain::ClipDetails& clip)
        {
            json details = {
                { "type", std::string(domain::CLIP_TYPES.idOf(clip.type)) },
                { "platform", clip.platform },
                { "url", clip.url },
                { "albumTitle", clip.albumTitle },
                { "watched", clip.watched }
            };
            setDate(details, "releaseDate", clip.releaseDate);
            return details;
        }

        json detailsToJson(const domain::LyricsDetails& lyrics)
        {
            return {
                { "albumTitle", lyrics.albumTitle },
                { "writers", lyrics.writers },
                { "originalText", lyrics.originalText },
                { "romanizedText", lyrics.romanizedText },
                { "translatedText", lyrics.translatedText },
                { "translationLanguage", lyrics.translationLanguage }
            };
        }

        json detailsToJson(const domain::EventDetails& event)
        {
            json details = {
                { "type", std::string(domain::EVENT_TYPES.idOf(event.type)) },
                { "venue", event.venue },
                { "city", event.city },
                { "seat", event.seat }
            };
            setDate(details, "date", event.date);
            return details;
        }

        domain::ItemDetails detailsFromJson(const domain::ItemKind kind, const json& source)
        {
            using namespace domain;

            switch (kind)
            {
                case ItemKind::Album:
                    return AlbumDetails{
                        .type = enumOr(ALBUM_TYPES, source, "type", AlbumType::Mini),
                        .format = enumOr(ALBUM_FORMATS, source, "format", AlbumFormat::Cd),
                        .edition = stringOrEmpty(source, "edition"),
                        .releaseDate = optionalDate(source, "releaseDate"),
                        .label = stringOrEmpty(source, "label"),
                        .region = stringOrEmpty(source, "region"),
                        .catalogNumber = stringOrEmpty(source, "catalogNumber"),
                        .inclusions = stringOrEmpty(source, "inclusions"),
                        .tracklist = tracklistFromJson(source, "tracklist") };

                case ItemKind::Photocard:
                    return PhotocardDetails{
                        .member = stringOrEmpty(source, "member"),
                        .origin = enumOr(PHOTOCARD_ORIGINS, source, "origin", PhotocardOrigin::AlbumInclusion),
                        .source = stringOrEmpty(source, "source"),
                        .forTrade = source.value("forTrade", false) };

                case ItemKind::Merchandise:
                    return MerchandiseDetails{
                        .type = enumOr(MERCHANDISE_TYPES, source, "type", MerchandiseType::Lightstick),
                        .version = stringOrEmpty(source, "version"),
                        .official = source.value("official", true),
                        .size = stringOrEmpty(source, "size") };

                case ItemKind::Clip:
                    return ClipDetails{
                        .type = enumOr(CLIP_TYPES, source, "type", ClipType::MusicVideo),
                        .platform = stringOrEmpty(source, "platform"),
                        .url = stringOrEmpty(source, "url"),
                        .releaseDate = optionalDate(source, "releaseDate"),
                        .albumTitle = stringOrEmpty(source, "albumTitle"),
                        .watched = source.value("watched", false) };

                case ItemKind::Lyrics:
                    return LyricsDetails{
                        .albumTitle = stringOrEmpty(source, "albumTitle"),
                        .writers = stringOrEmpty(source, "writers"),
                        .originalText = stringOrEmpty(source, "originalText"),
                        .romanizedText = stringOrEmpty(source, "romanizedText"),
                        .translatedText = stringOrEmpty(source, "translatedText"),
                        .translationLanguage = stringOrEmpty(source, "translationLanguage") };

                case ItemKind::Event:
                    return EventDetails{
                        .type = enumOr(EVENT_TYPES, source, "type", EventType::Concert),
                        .date = optionalDate(source, "date"),
                        .venue = stringOrEmpty(source, "venue"),
                        .city = stringOrEmpty(source, "city"),
                        .seat = stringOrEmpty(source, "seat") };
            }

            throw std::invalid_argument("Unsupported item kind");
        }
    }

    json toJson(const domain::Artist& artist)
    {
        return {
            { "id", artist.id },
            { "name", artist.name },
            { "type", std::string(domain::ARTIST_TYPES.idOf(artist.type)) },
            { "members", artist.members }
        };
    }

    json toJson(const domain::CollectionItem& item)
    {
        json result = {
            { "id", item.id },
            { "kind", std::string(domain::ITEM_KINDS.idOf(item.kind())) },
            { "title", item.title },
            { "artistId", item.artistId },
            { "status", std::string(domain::ITEM_STATUSES.idOf(item.status)) },
            { "condition", std::string(domain::ITEM_CONDITIONS.idOf(item.condition)) },
            { "quantity", item.quantity },
            { "acquiredFrom", item.acquiredFrom },
            { "notes", item.notes },
            { "details", std::visit([](const auto& details) { return detailsToJson(details); }, item.details) }
        };

        if (item.price)
            result["price"] = moneyToJson(*item.price);

        if (item.image)
            result["image"] = item.image->hash();

        setDate(result, "acquiredOn", item.acquiredOn);
        return result;
    }

    domain::Artist artistFromJson(const json& json)
    {
        domain::Artist artist;
        artist.id = json.at("id").get<std::string>();
        artist.name = json.at("name").get<std::string>();
        artist.type = enumOr(domain::ARTIST_TYPES, json, "type", domain::ArtistType::Group);
        artist.members = json.value("members", std::vector<std::string>());
        return artist;
    }

    domain::CollectionItem itemFromJson(const json& json)
    {
        const auto kind = requiredEnum(domain::ITEM_KINDS, json, "kind");

        domain::CollectionItem item;
        item.id = json.at("id").get<std::string>();
        item.title = json.at("title").get<std::string>();
        item.artistId = stringOrEmpty(json, "artistId");
        item.status = enumOr(domain::ITEM_STATUSES, json, "status", domain::ItemStatus::Owned);
        item.condition = enumOr(domain::ITEM_CONDITIONS, json, "condition", domain::ItemCondition::Unspecified);
        item.quantity = json.value("quantity", 1);
        item.acquiredOn = optionalDate(json, "acquiredOn");
        item.acquiredFrom = stringOrEmpty(json, "acquiredFrom");
        item.notes = stringOrEmpty(json, "notes");
        item.details = detailsFromJson(kind, json.at("details"));

        if (json.contains("price") && !json.at("price").is_null())
            item.price = moneyFromJson(json.at("price"));

        if (json.contains("image") && !json.at("image").is_null())
        {
            item.image = domain::ImageId::parse(json.at("image").get<std::string>());
            if (!item.image)
                throw std::invalid_argument("Invalid image id");
        }

        return item;
    }
}
