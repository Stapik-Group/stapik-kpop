#pragma once

#include "Artist.hpp"
#include "CollectionItem.hpp"
#include "Enumerations.hpp"

#include <string_view>
#include <vector>

namespace kpop::domain
{
    struct SortableItem
    {
        const CollectionItem* item = nullptr;
        const Artist* artist = nullptr;
    };

    // Titles and artists are ordered by the alphabet of the language (a Polish "Ł" comes after "L", not after "Z").
    void sortItems(std::vector<SortableItem>& items, SortOrder order, std::string_view languageCode);
}
