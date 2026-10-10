#pragma once

#include "ImageStore.hpp"

#include <filesystem>
#include <stdexcept>

namespace kpop::image
{
    class ImageImportError : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };

    // Covers are small, additional photos keep more detail.
    inline constexpr int MAX_IMAGE_DIMENSION = 800;
    inline constexpr int MAX_PHOTO_DIMENSION = 1600;

    [[nodiscard]] ImageBytes prepareImage(const std::filesystem::path& file, int maxDimension = MAX_IMAGE_DIMENSION);
}
