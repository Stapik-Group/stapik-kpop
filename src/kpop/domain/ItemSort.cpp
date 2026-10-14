#include "ItemSort.hpp"

#include "stapik/text/Collation.hpp"

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace kpop::domain
{
    namespace
    {
        std::optional<PartialDate> releaseDateOf(const ItemDetails& details)
        {
            if (const auto* album = std::get_if<AlbumDetails>(&details))
                return album->releaseDate;

            if (const auto* clip = std::get_if<ClipDetails>(&details))
                return clip->releaseDate;

            return std::nullopt;
        }

        std::optional<std::string> textKey(const std::string& text, const std::string_view languageCode)
        {
            if (text.empty())
                return std::nullopt;

            return stapik::text::collationKey(text, languageCode);
        }

        std::optional<PartialDate> releaseDateKeyOf(const SortableItem& sortable)
        {
            return releaseDateOf(sortable.item->details);
        }

        template<typename Key>
        bool precedes(const std::optional<Key>& left, const std::optional<Key>& right, const bool ascending)
        {
            if (!left || !right)
                return left.has_value() && !right.has_value();

            return ascending ? *left < *right : *right < *left;
        }

        template<typename Key, typename KeyOf>
        void sortByKey(std::vector<SortableItem>& items, const KeyOf& keyOf, const bool ascending)
        {
            struct Entry
            {
                std::optional<Key> key;
                SortableItem item;
            };

            std::vector<Entry> entries;
            entries.reserve(items.size());
            for (const auto& sortable : items)
                entries.push_back(Entry{ .key = keyOf(sortable), .item = sortable });

            std::ranges::stable_sort(entries, [ascending](const Entry& left, const Entry& right)
            {
                return precedes(left.key, right.key, ascending);
            });

            std::ranges::transform(entries, items.begin(), [](const Entry& entry) { return entry.item; });
        }
    }

    void sortItems(std::vector<SortableItem>& items, const SortOrder order, const std::string_view languageCode)
    {
        const auto titleKeyOf = [languageCode](const SortableItem& sortable)
        {
            return textKey(sortable.item->title, languageCode);
        };
        const auto artistKeyOf = [languageCode](const SortableItem& sortable) -> std::optional<std::string>
        {
            if (sortable.artist == nullptr)
                return std::nullopt;

            return textKey(sortable.artist->name, languageCode);
        };

        switch (order)
        {
            using enum SortOrder;
            case AddedNewest:
                return;
            case AddedOldest:
                std::ranges::reverse(items);
                return;
            case TitleAscending:
                sortByKey<std::string>(items, titleKeyOf, true);
                return;
            case TitleDescending:
                sortByKey<std::string>(items, titleKeyOf, false);
                return;
            case ArtistAscending:
                sortByKey<std::string>(items, artistKeyOf, true);
                return;
            case ArtistDescending:
                sortByKey<std::string>(items, artistKeyOf, false);
                return;
            case ReleaseDateOldest:
                sortByKey<PartialDate>(items, releaseDateKeyOf, true);
                return;
            case ReleaseDateNewest:
                sortByKey<PartialDate>(items, releaseDateKeyOf, false);
        }
    }
}
