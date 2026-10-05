#pragma once

#include "CollectionItem.hpp"

#include "stapik/domain/Money.hpp"

#include <array>
#include <cstddef>
#include <span>
#include <vector>

namespace kpop::domain
{
    struct CollectionStatistics
    {
        std::array<std::size_t, ITEM_KIND_COUNT> entriesPerKind{};
        std::size_t ownedCopies = 0;

        std::vector<stapik::domain::Money> spent;

        [[nodiscard]] std::size_t entriesOfKind(ItemKind kind) const;
        [[nodiscard]] std::size_t totalEntries() const;
    };

    [[nodiscard]] CollectionStatistics computeStatistics(std::span<const CollectionItem> items);
}
