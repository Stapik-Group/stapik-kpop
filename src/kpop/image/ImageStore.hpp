#pragma once

#include "kpop/domain/ImageId.hpp"

#include "stapik/cloud/IAssetStorage.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace kpop::image
{
    using ImageBytes = stapik::cloud::AssetBytes;

    class ImageStore
    {
    public:
        static constexpr std::string_view MIME_TYPE = "image/jpeg";

        explicit ImageStore(std::filesystem::path directory);
        [[nodiscard]] domain::ImageId add(const ImageBytes& content);
        [[nodiscard]] bool contains(const domain::ImageId& id) const;
        [[nodiscard]] std::optional<ImageBytes> read(const domain::ImageId& id) const;
        [[nodiscard]] std::filesystem::path pathOf(const domain::ImageId& id) const;
        [[nodiscard]] std::vector<domain::ImageId> ids() const;
        [[nodiscard]] static std::string fileName(const domain::ImageId& id);
        [[nodiscard]] static std::optional<domain::ImageId> idFromFileName(std::string_view name);
        bool remove(const domain::ImageId& id);

    private:
        std::filesystem::path m_directory;
    };
}
