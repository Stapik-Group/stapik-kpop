#pragma once

#include "ImageStore.hpp"

#include <filesystem>
#include <optional>

namespace kpop::image
{
    class ImageLibrary
    {
    public:
        explicit ImageLibrary(std::filesystem::path directory);
        [[nodiscard]] domain::ImageId importFile(const std::filesystem::path& sourceFile);
        [[nodiscard]] std::optional<std::filesystem::path> pathOf(const domain::ImageId& id) const;
        [[nodiscard]] ImageStore& store();
        [[nodiscard]] const ImageStore& store() const;
    private:
        ImageStore m_store;
    };
}
