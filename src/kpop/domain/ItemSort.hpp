#pragma once

#include "Artist.hpp"
#include "CollectionItem.hpp"
#include "Enumerations.hpp"

#include <span>
#include <string_view>
#include <vector>

namespace kpop::domain
{
    struct SortableItem
    {
        const CollectionItem* item = nullptr;
        const Artist* artist = nullptr;
    };

    // What an order sorts by; the two directions of one field belong to the same field.
    enum class SortField
    {
        Added,
        Title,
        Artist,
        ReleaseDate
    };

    [[nodiscard]] SortField sortFieldOf(SortOrder order);

    // Titles and artists are ordered by the alphabet of the language (a Polish "Ł" comes after "L", not after "Z").
    void sortItems(std::vector<SortableItem>& items, SortOrder order, std::string_view languageCode);

    // Sorts by several orders at once: the first is the main one, each next one only orders the items that
    // all the earlier ones consider equal. For example artist first and release date second groups the items
    // by artist and puts each artist's items in chronological order.
    void sortItems(std::vector<SortableItem>& items, std::span<const SortOrder> orders, std::string_view languageCode);

    // The orders after one more has been chosen: it becomes the main one and the earlier ones stay behind it.
    // An earlier order of the same field is replaced by it. Choosing one of the "added" orders starts over,
    // because "added" is the order the items arrive in and it already settles every tie.
    [[nodiscard]] std::vector<SortOrder> withSortLevel(const std::vector<SortOrder>& levels, SortOrder chosen);
}
