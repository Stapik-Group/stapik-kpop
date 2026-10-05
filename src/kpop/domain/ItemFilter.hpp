#pragma once

#include "Artist.hpp"
#include "CollectionItem.hpp"

#include <optional>
#include <string>

namespace kpop::domain
{
    struct ItemFilter
    {
        std::string text = {};
        std::optional<ItemKind> kind = std::nullopt;
        std::optional<ItemStatus> status = std::nullopt;
        std::optional<std::string> artistId = std::nullopt;

        bool operator==(const ItemFilter&) const = default;
    };

    [[nodiscard]] bool matchesFilter(const ItemFilter& filter, const CollectionItem& item, const Artist* artist);
}
