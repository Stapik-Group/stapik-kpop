#include "ImageTexture.hpp"

#include "stapik/storage/PathText.hpp"

#include <gdkmm/pixbuf.h>

#include <glibmm/error.h>

namespace kpop::ui
{
    Glib::RefPtr<Gdk::Texture> loadTexture(const std::filesystem::path& file, const int maxSize)
    {
        return loadTexture(file, maxSize, maxSize);
    }

    Glib::RefPtr<Gdk::Texture> loadTexture(const std::filesystem::path& file, const int maxWidth, const int maxHeight)
    {
        try
        {
            const auto pixbuf = Gdk::Pixbuf::create_from_file(stapik::storage::pathText(file), maxWidth, maxHeight, true);
            return Gdk::Texture::create_for_pixbuf(pixbuf);
        }
        catch (const Glib::Error&)
        {
            return {};
        }
    }

    Gtk::Picture* makeExactPicture(const Glib::RefPtr<Gdk::Texture>& texture)
    {
        auto* picture = Gtk::make_managed<Gtk::Picture>();
        picture->set_paintable(texture);
        picture->set_can_shrink(false);
        picture->set_size_request(texture->get_width(), texture->get_height());
        picture->set_halign(Gtk::Align::CENTER);
        picture->set_valign(Gtk::Align::CENTER);
        return picture;
    }
}
