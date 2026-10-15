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

    // Covers are smaller, additional photos keep more detail. Both are large enough to stay sharp on a high-resolution
    // screen, and images that are smaller than this are never enlarged.
    inline constexpr int MAX_IMAGE_DIMENSION = 1600;
    inline constexpr int MAX_PHOTO_DIMENSION = 2400;

    [[nodiscard]] ImageBytes prepareImage(const std::filesystem::path& file, int maxDimension = MAX_IMAGE_DIMENSION);
}
