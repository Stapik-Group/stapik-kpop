#include "CollectionDocument.hpp"

#include "CollectionSchema.hpp"
#include "CollectionSerializer.hpp"

#include "stapik/sync/Timestamp.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

namespace kpop::document
{
    namespace
    {
        std::vector<domain::ImageId> sortedWithoutRepeats(std::vector<domain::ImageId> images)
        {
            std::ranges::sort(images);
            const auto repeated = std::ranges::unique(images);
            images.erase(repeated.begin(), repeated.end());
            return images;
        }

        template<typename Entry>
        std::optional<std::size_t> indexById(const std::vector<Entry>& entries, const std::string_view id)
        {
            const auto found = std::ranges::find(entries, id, &Entry::id);
            if (found == entries.end())
                return std::nullopt;

            return static_cast<std::size_t>(found - entries.begin());
        }

        template<typename Entry>
        void insertAt(std::vector<Entry>& entries, const std::size_t index, Entry entry)
        {
            const auto position = std::min(index, entries.size());
            entries.insert(entries.begin() + static_cast<std::ptrdiff_t>(position), std::move(entry));
        }

        template<typename Entry>
        bool replaceById(std::vector<Entry>& entries, const Entry& entry)
        {
            const auto index = indexById(entries, entry.id);
            if (!index)
                return false;

            entries[*index] = entry;
            return true;
        }

        template<typename Entry>
        bool removeById(std::vector<Entry>& entries, const std::string_view id)
        {
            const auto index = indexById(entries, id);
            if (!index)
                return false;

            entries.erase(entries.begin() + static_cast<std::ptrdiff_t>(*index));
            return true;
        }
    }

    const std::vector<domain::Artist>& CollectionDocument::artists() const
    {
        return m_artists;
    }

    const std::vector<domain::CollectionItem>& CollectionDocument::items() const
    {
        return m_items;
    }

    std::vector<domain::ImageId> CollectionDocument::referencedImages() const
    {
        std::vector<domain::ImageId> images;
        for (const auto& item : m_items)
        {
            if (item.image)
                images.push_back(*item.image);
        }

        return sortedWithoutRepeats(std::move(images));
    }

    std::vector<domain::ImageId> CollectionDocument::referencedPhotos() const
    {
        std::vector<domain::ImageId> photos;
        for (const auto& item : m_items)
            photos.insert(photos.end(), item.photos.begin(), item.photos.end());

        return sortedWithoutRepeats(std::move(photos));
    }

    const domain::Artist* CollectionDocument::findArtist(const std::string_view artistId) const
    {
        const auto index = indexById(m_artists, artistId);
        return index ? &m_artists[*index] : nullptr;
    }

    const domain::CollectionItem* CollectionDocument::findItem(const std::string_view itemId) const
    {
        const auto index = indexById(m_items, itemId);
        return index ? &m_items[*index] : nullptr;
    }

    std::optional<std::size_t> CollectionDocument::indexOfArtist(const std::string_view artistId) const
    {
        return indexById(m_artists, artistId);
    }

    std::optional<std::size_t> CollectionDocument::indexOfItem(const std::string_view itemId) const
    {
        return indexById(m_items, itemId);
    }

    std::size_t CollectionDocument::countItemsOfArtist(const std::string_view artistId) const
    {
        return static_cast<std::size_t>(std::ranges::count(m_items, artistId, &domain::CollectionItem::artistId));
    }

    void CollectionDocument::insertArtist(const std::size_t index, domain::Artist artist)
    {
        insertAt(m_artists, index, std::move(artist));
    }

    void CollectionDocument::insertItem(const std::size_t index, domain::CollectionItem item)
    {
        insertAt(m_items, index, std::move(item));
    }

    bool CollectionDocument::replaceArtist(const domain::Artist& artist)
    {
        return replaceById(m_artists, artist);
    }

    bool CollectionDocument::replaceItem(const domain::CollectionItem& item)
    {
        return replaceById(m_items, item);
    }

    bool CollectionDocument::removeArtist(const std::string_view artistId)
    {
        return removeById(m_artists, artistId);
    }

    bool CollectionDocument::removeItem(const std::string_view itemId)
    {
        return removeById(m_items, itemId);
    }

    void CollectionDocument::touch(const TimePoint now)
    {
        m_lastUpdate = now;
    }

    CollectionDocument::TimePoint CollectionDocument::lastUpdate() const
    {
        return m_lastUpdate;
    }

    std::optional<CollectionDocument::TimePoint> CollectionDocument::lastKnownCloudUpdate() const
    {
        return m_lastKnownCloudUpdate;
    }

    CollectionDocument CollectionDocument::withLastKnownCloudUpdate(const TimePoint cloudUpdate) const
    {
        auto copy = *this;
        copy.m_lastKnownCloudUpdate = cloudUpdate;
        return copy;
    }

    nlohmann::json CollectionDocument::toJson() const
    {
        auto artists = nlohmann::json::array();
        for (const auto& artist : m_artists)
            artists.push_back(serializer::toJson(artist));

        auto items = nlohmann::json::array();
        for (const auto& item : m_items)
            items.push_back(serializer::toJson(item));

        nlohmann::json json = {
            { "schemaVersion", COLLECTION_SCHEMA_VERSION },
            { "lastUpdate", stapik::sync::toIso8601(m_lastUpdate) },
            { "artists", std::move(artists) },
            { "items", std::move(items) }
        };

        if (m_lastKnownCloudUpdate)
            json["lastKnownCloudUpdate"] = stapik::sync::toIso8601(*m_lastKnownCloudUpdate, stapik::sync::TimestampPrecision::Microseconds);

        return json;
    }

    CollectionDocument CollectionDocument::fromJson(const nlohmann::json& json)
    {
        if (const auto schemaVersion = json.value("schemaVersion", COLLECTION_SCHEMA_VERSION); schemaVersion > COLLECTION_SCHEMA_VERSION)
            throw std::invalid_argument("Document schema version " + std::to_string(schemaVersion) + " is newer than supported");

        CollectionDocument document;

        for (const auto& artistJson : json.at("artists"))
            document.m_artists.push_back(serializer::artistFromJson(artistJson));

        for (const auto& itemJson : json.at("items"))
            document.m_items.push_back(serializer::itemFromJson(itemJson));

        if (json.contains("lastUpdate"))
        {
            const auto lastUpdate = stapik::sync::parseIso8601(json.at("lastUpdate").get<std::string>());
            if (!lastUpdate)
                throw std::invalid_argument("Invalid lastUpdate timestamp");

            document.m_lastUpdate = *lastUpdate;
        }

        if (json.contains("lastKnownCloudUpdate"))
        {
            const auto baseline = stapik::sync::parseIso8601(json.at("lastKnownCloudUpdate").get<std::string>());
            if (!baseline)
                throw std::invalid_argument("Invalid lastKnownCloudUpdate timestamp");

            document.m_lastKnownCloudUpdate = *baseline;
        }

        return document;
    }
}
