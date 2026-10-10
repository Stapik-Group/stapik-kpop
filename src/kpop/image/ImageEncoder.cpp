#include "ImageEncoder.hpp"

#include "stapik/storage/PathText.hpp"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

namespace kpop::image
{
    namespace
    {
        constexpr auto JPEG_QUALITY = "85";
        constexpr guint32 WHITE = 0xFFFFFFFF;
        constexpr int ALPHA_OPAQUE = 255;

        struct PixbufDeleter
        {
            void operator()(GdkPixbuf* pixbuf) const
            {
                if (pixbuf != nullptr)
                    g_object_unref(pixbuf);
            }
        };

        using PixbufPtr = std::unique_ptr<GdkPixbuf, PixbufDeleter>;

        std::string takeMessage(GError*& error)
        {
            std::string message = error != nullptr ? error->message : "unknown error";
            g_clear_error(&error);
            return message;
        }

        PixbufPtr load(const std::string& path, const int maxDimension)
        {
            int width = 0;
            int height = 0;
            if (gdk_pixbuf_get_file_info(path.c_str(), &width, &height) == nullptr)
                throw ImageImportError("the file is not a supported image");

            GError* error = nullptr;
            GdkPixbuf* pixbuf = std::max(width, height) > maxDimension
                ? gdk_pixbuf_new_from_file_at_size(path.c_str(), maxDimension, maxDimension, &error)
                : gdk_pixbuf_new_from_file(path.c_str(), &error);

            if (pixbuf == nullptr)
                throw ImageImportError(takeMessage(error));

            return PixbufPtr(pixbuf);
        }

        PixbufPtr flattenOntoWhite(PixbufPtr source)
        {
            if (gdk_pixbuf_get_has_alpha(source.get()) == FALSE)
                return source;

            const int width = gdk_pixbuf_get_width(source.get());
            const int height = gdk_pixbuf_get_height(source.get());

            PixbufPtr flat(gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, width, height));
            if (!flat)
                throw ImageImportError("not enough memory for the image");

            gdk_pixbuf_fill(flat.get(), WHITE);
            gdk_pixbuf_composite(source.get(), flat.get(), 0, 0, width, height, 0.0, 0.0, 1.0, 1.0, GDK_INTERP_NEAREST, ALPHA_OPAQUE);
            return flat;
        }

        ImageBytes encodeAsJpeg(GdkPixbuf* pixbuf)
        {
            gchar* buffer = nullptr;
            gsize size = 0;

            if (GError* error = nullptr; gdk_pixbuf_save_to_buffer(pixbuf, &buffer, &size, "jpeg", &error, "quality", JPEG_QUALITY, nullptr) == FALSE)
                throw ImageImportError(takeMessage(error));

            const auto* bytes = reinterpret_cast<const std::uint8_t*>(buffer);
            ImageBytes result(bytes, bytes + size);
            g_free(buffer);
            return result;
        }
    }

    ImageBytes prepareImage(const std::filesystem::path& file, const int maxDimension)
    {
        auto pixbuf = load(stapik::storage::pathText(file), maxDimension);

        // Returns the very same image (with one more reference) when there is nothing to rotate.
        if (PixbufPtr oriented(gdk_pixbuf_apply_embedded_orientation(pixbuf.get())); oriented)
            pixbuf = std::move(oriented);

        pixbuf = flattenOntoWhite(std::move(pixbuf));
        return encodeAsJpeg(pixbuf.get());
    }
}
