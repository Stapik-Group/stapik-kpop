#pragma once

#include <gdkmm/texture.h>

#include <glibmm/refptr.h>

#include <filesystem>

namespace kpop::ui
{
    // The image of a file shrunk to fit the given size, or an empty pointer when it cannot be read.
    [[nodiscard]] Glib::RefPtr<Gdk::Texture> loadTexture(const std::filesystem::path& file, int maxSize);
}
