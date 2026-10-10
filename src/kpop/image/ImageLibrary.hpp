#pragma once

#include "ImageEncoder.hpp"
#include "ImageStore.hpp"

#include <filesystem>
#include <optional>

namespace kpop::image
{
    class ImageLibrary
    {
    public:
        // Imported images are shrunk to fit maxDimension.
        explicit ImageLibrary(std::filesystem::path directory, int maxDimension = MAX_IMAGE_DIMENSION);
        [[nodiscard]] domain::ImageId importFile(const std::filesystem::path& sourceFile);
        [[nodiscard]] std::optional<std::filesystem::path> pathOf(const domain::ImageId& id) const;
        [[nodiscard]] ImageStore& store();
        [[nodiscard]] const ImageStore& store() const;
    private:
        ImageStore m_store;
        int m_maxDimension;
    };
}
