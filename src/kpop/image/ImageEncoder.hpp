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

    inline constexpr int MAX_IMAGE_DIMENSION = 800;
    [[nodiscard]] ImageBytes prepareImage(const std::filesystem::path& file);
}
