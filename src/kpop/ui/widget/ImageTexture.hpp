#pragma once

#include <gdkmm/texture.h>

#include <glibmm/refptr.h>
#include <gtkmm/picture.h>

#include <filesystem>

namespace kpop::ui
{
    // The image of a file shrunk to fit the given size, or an empty pointer when it cannot be read.
    [[nodiscard]] Glib::RefPtr<Gdk::Texture> loadTexture(const std::filesystem::path& file, int maxSize);

    // The same, shrunk to fit a box that is not square.
    [[nodiscard]] Glib::RefPtr<Gdk::Texture> loadTexture(const std::filesystem::path& file, int maxWidth, int maxHeight);

    // A picture that is shown pixel for pixel. A Gtk::Picture with only a minimum size grows to the aspect ratio of
    // its image, which stretches a small texture and makes it blurry, so previews are sized to their texture instead.
    [[nodiscard]] Gtk::Picture* makeExactPicture(const Glib::RefPtr<Gdk::Texture>& texture);
}
