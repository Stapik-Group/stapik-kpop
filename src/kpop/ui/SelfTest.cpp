#include "SelfTest.hpp"

#include "Translate.hpp"

#include "stapik/log/Log.hpp"
#include "stapik/storage/AppPaths.hpp"
#include "stapik/storage/AtomicFile.hpp"
#include "stapik/storage/PathText.hpp"

#include <gdkmm/pixbufloader.h>
#include <gio/gio.h>
#include <gtkmm/icontheme.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <string>
#include <string_view>

namespace kpop::ui
{
    namespace
    {
        constexpr std::array ICON_NAMES = { "edit-find-symbolic", "edit-clear-symbolic", "pan-down-symbolic" };
        constexpr std::string_view GTK_SCHEMA = "org.gtk.gtk4.Settings.FileChooser";
        constexpr std::string_view TEST_SVG = R"(<svg xmlns="http://www.w3.org/2000/svg" width="8" height="8"><rect width="8" height="8" fill="#000"/></svg>)";
        constexpr std::string_view TRANSLATION_KEY = "kpop.kind.album";

        class Report
        {
        public:
            void check(const std::string_view name, const std::function<bool()>& test)
            {
                bool passed = false;
                try
                {
                    passed = test();
                }
                catch (const std::exception& error)
                {
                    stapik::log::error("self-test: {} threw: {}", name, error.what());
                }

                if (passed)
                {
                    stapik::log::info("self-test: ok     {}", name);
                    return;
                }

                stapik::log::error("self-test: FAILED {}", name);
                ++m_failures;
            }

            static void advise(const std::string_view name, const std::function<bool()>& test)
            {
                bool available = false;
                try
                {
                    available = test();
                }
                catch (const std::exception&)
                {
                    available = false;
                }

                if (available)
                    stapik::log::info("self-test: ok     {}", name);
                else
                    stapik::log::warning("self-test: note   {} is not available", name);
            }

            [[nodiscard]] int failures() const
            {
                return m_failures;
            }

        private:
            int m_failures = 0;
        };

        bool canDecodeSvg()
        {
            const auto loader = Gdk::PixbufLoader::create("svg");
            loader->write(reinterpret_cast<const guint8*>(TEST_SVG.data()), TEST_SVG.size());
            loader->close();

            const auto pixbuf = loader->get_pixbuf();
            return pixbuf && pixbuf->get_width() == 8;
        }

        bool gtkSchemaIsInstalled()
        {
            auto* source = g_settings_schema_source_get_default();
            if (source == nullptr)
                return false;

            auto* schema = g_settings_schema_source_lookup(source, std::string(GTK_SCHEMA).c_str(), TRUE);
            if (schema == nullptr)
                return false;

            g_settings_schema_unref(schema);
            return true;
        }

        bool dataDirectoryIsWritable()
        {
            const auto file = AppPaths::userDataDir(STAPIK_APP_NAME) / "selftest.tmp";
            constexpr std::string_view CONTENT = "ąęł self-test\n";

            if (!stapik::storage::writeFileAtomically(file, CONTENT))
                return false;

            std::ifstream stream(file, std::ios::binary);
            const std::string readBack((std::istreambuf_iterator(stream)), std::istreambuf_iterator<char>());
            stream.close();

            std::error_code errorCode;
            std::filesystem::remove(file, errorCode);
            return readBack == CONTENT;
        }
    }

    int runSelfTest(Gtk::Window& window, const app::CollectionController& controller)
    {
        Report report;

        report.check("the main window is shown", [&window] { return window.get_mapped(); });

        const auto iconTheme = Gtk::IconTheme::get_for_display(window.get_display());
        for (const auto* iconName : ICON_NAMES)
            report.check(std::string("icon ") + iconName, [&iconTheme, iconName] { return iconTheme->has_icon(iconName); });

        // GTK draws its own widgets with built-in icons and this application uses no icons of the theme, so a missing
        // loader only matters for icons that other applications of the family may add.
        Report::advise("the SVG image loader of gdk-pixbuf", canDecodeSvg);
        report.check("GSettings schemas are installed", gtkSchemaIsInstalled);

        report.check("stapik-common resources are found", [] { return std::filesystem::is_directory(AppPaths::commonResourcesDir() / "locales"); });
        report.check("application resources are found", [] { return std::filesystem::is_directory(AppPaths::resourcesDir() / "themes"); });
        report.check("translations are loaded", [] { return translate(TRANSLATION_KEY) != TRANSLATION_KEY; });

        report.check("the collection file was read", [&controller]
        {
            return controller.loadStatus() == stapik::document::LoadStatus::Loaded || controller.loadStatus() == stapik::document::LoadStatus::Missing;
        });
        report.check("the data directory is writable", dataDirectoryIsWritable);

        if (report.failures() == 0)
            stapik::log::info("self-test: all checks passed");
        else
            stapik::log::error("self-test: {} check(s) failed", report.failures());

        return report.failures();
    }
}
