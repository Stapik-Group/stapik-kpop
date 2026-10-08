#pragma once

#include "Artist.hpp"
#include "CollectionItem.hpp"
#include "Enumerations.hpp"

#include <vector>

namespace kpop::domain
{
    struct SortableItem
    {
        const CollectionItem* item = nullptr;
        const Artist* artist = nullptr;
    };

    void sortItems(std::vector<SortableItem>& items, SortOrder order);
}
