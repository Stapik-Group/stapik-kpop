#pragma once

#include "kpop/image/ImageStore.hpp"

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

namespace kpop::test
{
    inline bool writePng(const std::filesystem::path& file, const int width, const int height, const bool withAlpha, const guint32 color)
    {
        GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, withAlpha ? TRUE : FALSE, 8, width, height);
        if (pixbuf == nullptr)
            return false;

        gdk_pixbuf_fill(pixbuf, color);

        GError* error = nullptr;
        const gboolean saved = gdk_pixbuf_save(pixbuf, file.string().c_str(), "png", &error, nullptr);

        g_clear_error(&error);
        g_object_unref(pixbuf);
        return saved == TRUE;
    }

    inline void writeBytes(const std::filesystem::path& file, const image::ImageBytes& bytes)
    {
        std::ofstream stream(file, std::ios::binary);
        stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    struct ImageInfo
    {
        int width = 0;
        int height = 0;
        std::string format;
    };

    inline std::optional<ImageInfo> inspectImage(const std::filesystem::path& file)
    {
        ImageInfo info;
        GdkPixbufFormat* format = gdk_pixbuf_get_file_info(file.string().c_str(), &info.width, &info.height);
        if (format == nullptr)
            return std::nullopt;

        gchar* name = gdk_pixbuf_format_get_name(format);
        info.format = name;
        g_free(name);
        return info;
    }
}
