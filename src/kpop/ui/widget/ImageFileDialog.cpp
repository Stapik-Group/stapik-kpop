#include "ImageFileDialog.hpp"

#include "kpop/ui/Translate.hpp"

#include <giomm/liststore.h>

#include <gtkmm/error.h>
#include <gtkmm/filedialog.h>
#include <gtkmm/filefilter.h>
#include <gtkmm/window.h>

#include <glibmm/error.h>

#include <utility>

namespace kpop::ui
{
    namespace
    {
        std::filesystem::path pathFromUtf8(const std::string& text)
        {
            return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(text.data()), text.size()));
        }
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

        dialog->open(parent, [dialog, onChosen = std::move(onChosen), onError = std::move(onError)](const Glib::RefPtr<Gio::AsyncResult>& result)
        {
            try
            {
                if (const auto file = dialog->open_finish(result))
                    onChosen(pathFromUtf8(file->get_path()));
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
