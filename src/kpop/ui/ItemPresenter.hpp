#pragma once

#include "kpop/domain/Artist.hpp"
#include "kpop/domain/CollectionItem.hpp"

#include <filesystem>
#include <string>
#include <string_view>

namespace kpop::ui
{
    struct ItemRow
    {
        std::string id;
        domain::ItemKind kind = domain::ItemKind::Album;
        domain::ItemStatus status = domain::ItemStatus::Owned;
        std::string title;
        std::string subtitle;
        std::string statusText;
        std::string quantityText;
        std::string priceText;

        std::filesystem::path imagePath;
    };

    [[nodiscard]] ItemRow describeItem(const domain::CollectionItem& item, const domain::Artist* artist, std::string_view languageCode);
}
