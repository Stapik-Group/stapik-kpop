#pragma once

#include "ImageId.hpp"
#include "ItemDetails.hpp"

#include "stapik/domain/Money.hpp"

#include <optional>
#include <string>
#include <vector>

namespace kpop::domain
{
    // An additional photo of an entry with an optional caption, for example "Photobook".
    struct ItemPhoto
    {
        ImageId image;
        std::string caption;

        bool operator==(const ItemPhoto&) const = default;
    };

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
        std::vector<ItemPhoto> photos;
        ItemDetails details = AlbumDetails{};

        bool operator==(const CollectionItem&) const = default;

        [[nodiscard]] ItemKind kind() const
        {
            return kindOf(details);
        }
    };

    // A copy that becomes a new entry once it is added: it gets its id then.
    [[nodiscard]] inline CollectionItem duplicateOf(CollectionItem item)
    {
        item.id.clear();
        return item;
    }
}
