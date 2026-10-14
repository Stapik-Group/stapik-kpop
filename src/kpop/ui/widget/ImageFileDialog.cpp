#include "ImageFileDialog.hpp"

#include "kpop/ui/Translate.hpp"

#include "stapik/app/AppContext.hpp"
#include "stapik/log/Log.hpp"
#include "stapik/storage/PathText.hpp"
#include "stapik/ui/dialog/LastDirectory.hpp"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <giomm/file.h>
#include <giomm/liststore.h>

#include <gtkmm/error.h>
#include <gtkmm/filedialog.h>
#include <gtkmm/filefilter.h>
#include <gtkmm/window.h>

#include <glibmm/error.h>

#include <memory>
#include <utility>

namespace kpop::ui
{
    namespace
    {
        constexpr auto LAST_DIRECTORY_KEY = "images.lastDirectory";

        std::filesystem::path pathFromUtf8(const std::string& text)
        {
            return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(text.data()), text.size()));
        }

        void rememberDirectory(const std::filesystem::path& chosen)
        {
            if (!stapik::app::AppContext::isInitialized())
                return;

            if (const auto saved = stapik::ui::saveLastDirectory(stapik::app::AppContext::instance().settings(), LAST_DIRECTORY_KEY, chosen); !saved)
                stapik::log::warning("Cannot remember the folder of the last image: {}", saved.error().message());
        }
    }

    std::vector<std::string> imageFileExtensions()
    {
        std::vector<std::string> extensions;

        GSList* formats = gdk_pixbuf_get_formats();
        for (const GSList* node = formats; node != nullptr; node = node->next)
        {
            auto* format = static_cast<GdkPixbufFormat*>(node->data);
            if (gdk_pixbuf_format_is_disabled(format))
                continue;

            const std::unique_ptr<gchar*, decltype(&g_strfreev)> names(gdk_pixbuf_format_get_extensions(format), &g_strfreev);
            for (gchar* const* name = names.get(); name != nullptr && *name != nullptr; ++name)
                extensions.push_back(std::string(".") + *name);
        }

        g_slist_free(formats);
        return extensions;
    }

    void chooseImageFile(
        Gtk::Window& parent,
        const std::string& title,
        std::function<void(const std::filesystem::path&)> onChosen,
        std::function<void(const std::string&)> onError)
    {
        auto filter = Gtk::FileFilter::create();
        filter->set_name(translate("kpop.image.filter"));
        filter->add_pixbuf_formats();

        auto filters = Gio::ListStore<Gtk::FileFilter>::create();
        filters->append(filter);

        auto dialog = Gtk::FileDialog::create();
        dialog->set_title(title);
        dialog->set_filters(filters);
        dialog->set_default_filter(filter);

        if (stapik::app::AppContext::isInitialized())
        {
            if (const auto directory = stapik::ui::loadLastDirectory(stapik::app::AppContext::instance().settings(), LAST_DIRECTORY_KEY))
                dialog->set_initial_folder(Gio::File::create_for_path(stapik::storage::pathText(*directory)));
        }

        dialog->open(parent, [dialog, onChosen = std::move(onChosen), onError = std::move(onError)](const Glib::RefPtr<Gio::AsyncResult>& result)
        {
            try
            {
                if (const auto file = dialog->open_finish(result))
                {
                    const auto chosen = pathFromUtf8(file->get_path());
                    rememberDirectory(chosen);
                    onChosen(chosen);
                }
            }
            catch (const Gtk::DialogError&)
            {
                // The user closed the dialog without choosing anything.
            }
            catch (const Glib::Error& error)
            {
                onError(error.what());
            }
        });
    }
}
