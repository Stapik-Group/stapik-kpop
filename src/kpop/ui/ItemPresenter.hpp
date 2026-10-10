#pragma once

#include "kpop/domain/Artist.hpp"
#include "kpop/domain/CollectionItem.hpp"

#include "stapik/domain/CategoryColor.hpp"

#include <filesystem>
#include <optional>
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

        std::string artistName;
        std::optional<stapik::domain::CategoryColor> artistColor;

        std::filesystem::path imagePath;
    };

    [[nodiscard]] const char* statusCssClass(domain::ItemStatus status);
    [[nodiscard]] ItemRow describeItem(const domain::CollectionItem& item, const domain::Artist* artist, std::string_view languageCode);
}
