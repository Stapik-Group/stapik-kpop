#pragma once

#include "kpop/domain/Artist.hpp"
#include "kpop/domain/CollectionItem.hpp"

#include <nlohmann/json.hpp>

namespace kpop::document::serializer
{
    [[nodiscard]] nlohmann::json toJson(const domain::Artist& artist);
    [[nodiscard]] nlohmann::json toJson(const domain::CollectionItem& item);

    // Both throw std::exception (std::invalid_argument or nlohmann::json::exception) on malformed input.
    [[nodiscard]] domain::Artist artistFromJson(const nlohmann::json& json);
    [[nodiscard]] domain::CollectionItem itemFromJson(const nlohmann::json& json);
}
