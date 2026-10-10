#pragma once

#include "kpop/domain/Artist.hpp"
#include "kpop/domain/CollectionItem.hpp"

#include <nlohmann/json.hpp>

#include <chrono>
#include <cstddef>
#include <optional>
#include <string_view>
#include <vector>

namespace kpop::document
{
    class CollectionDocument
    {
    public:
        using TimePoint = std::chrono::system_clock::time_point;

        CollectionDocument() = default;

        [[nodiscard]] const std::vector<domain::Artist>& artists() const;
        [[nodiscard]] const std::vector<domain::CollectionItem>& items() const;
        // Covers and additional photos live in separate collections; both lists are sorted and without repeats.
        [[nodiscard]] std::vector<domain::ImageId> referencedImages() const;
        [[nodiscard]] std::vector<domain::ImageId> referencedPhotos() const;
        [[nodiscard]] const domain::Artist* findArtist(std::string_view artistId) const;
        [[nodiscard]] const domain::CollectionItem* findItem(std::string_view itemId) const;
        [[nodiscard]] std::optional<std::size_t> indexOfArtist(std::string_view artistId) const;
        [[nodiscard]] std::optional<std::size_t> indexOfItem(std::string_view itemId) const;
        [[nodiscard]] std::size_t countItemsOfArtist(std::string_view artistId) const;

        void insertArtist(std::size_t index, domain::Artist artist);
        void insertItem(std::size_t index, domain::CollectionItem item);

        bool replaceArtist(const domain::Artist& artist);
        bool replaceItem(const domain::CollectionItem& item);

        bool removeArtist(std::string_view artistId);
        bool removeItem(std::string_view itemId);

        void touch(TimePoint now);

        [[nodiscard]] TimePoint lastUpdate() const;
        [[nodiscard]] std::optional<TimePoint> lastKnownCloudUpdate() const;
        [[nodiscard]] CollectionDocument withLastKnownCloudUpdate(TimePoint cloudUpdate) const;

        [[nodiscard]] nlohmann::json toJson() const;

        [[nodiscard]] static CollectionDocument fromJson(const nlohmann::json& json);

    private:
        std::vector<domain::Artist> m_artists;
        std::vector<domain::CollectionItem> m_items;
        TimePoint m_lastUpdate;
        std::optional<TimePoint> m_lastKnownCloudUpdate;
    };
}
