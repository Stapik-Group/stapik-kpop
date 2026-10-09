#include "ImageTexture.hpp"

#include "stapik/storage/PathText.hpp"

#include <gdkmm/pixbuf.h>

#include <glibmm/error.h>

namespace kpop::ui
{
    Glib::RefPtr<Gdk::Texture> loadTexture(const std::filesystem::path& file, const int maxSize)
    {
        try
        {
            const auto pixbuf = Gdk::Pixbuf::create_from_file(stapik::storage::pathText(file), maxSize, maxSize, true);
            return Gdk::Texture::create_for_pixbuf(pixbuf);
        }
        catch (const Glib::Error&)
        {
            return {};
        }
    }
}
