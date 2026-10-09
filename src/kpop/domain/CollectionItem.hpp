#pragma once

#include "ImageId.hpp"
#include "ItemDetails.hpp"

#include "stapik/domain/Money.hpp"

#include <optional>
#include <string>

namespace kpop::domain
{
    struct CollectionItem
    {
        std::string id;
        std::string title;
        std::string artistId;
        ItemStatus status = ItemStatus::Owned;
        ItemCondition condition = ItemCondition::Unspecified;
        int quantity = 1;
        std::optional<stapik::domain::Money> price;
        std::optional<PartialDate> acquiredOn;
        std::string acquiredFrom;
        std::string notes;
        std::optional<ImageId> image;
        ItemDetails details = AlbumDetails{};

        bool operator==(const CollectionItem&) const = default;

        [[nodiscard]] ItemKind kind() const
        {
            return kindOf(details);
        }
    };
}
