#include "kpop/app/CollectionController.hpp"
#include "kpop/app/ImageSync.hpp"
#include "kpop/ui/MainWindow.hpp"
#include "kpop/ui/SelfTest.hpp"

#include "stapik/app/AppContext.hpp"
#include "stapik/app/SingleInstanceGuard.hpp"
#include "stapik/log/Log.hpp"
#include "stapik/storage/AppPaths.hpp"
#include "stapik/sync/CloudSessionHooks.hpp"
#include "stapik/theme/ThemeManager.hpp"
#include "stapik/ui/menu/StandardMenu.hpp"
#include "stapik/ui/style/AppStyleProvider.hpp"

#include <glib.h>
#include <glibmm/main.h>
#include <gtkmm/application.h>

#include <algorithm>
#include <cstring>
#include <memory>

namespace
{
    constexpr auto APPLICATION_ID = "pl.stapik.kpop";
    constexpr auto AUTHOR = "Sebastian Smoliński";
    constexpr auto REPOSITORY_URL = "https://github.com/Stapik-Group/stapik-kpop";
    constexpr auto COLLECTION_FILE_NAME = "collection.json";
    constexpr auto IMAGES_DIRECTORY_NAME = "images";
    constexpr auto CLOUD_SLOT_KEY = "stapikkpop.json";
    constexpr auto IMAGES_SLOT_KEY = "covers";
    constexpr auto SELF_TEST_OPTION = "--self-test";
    constexpr unsigned SELF_TEST_DELAY_MILLISECONDS = 2000;

    bool takeSelfTestOption(int& argumentCount, char* arguments[])
    {
        const auto end = arguments + argumentCount;
        const auto option = std::find_if(arguments + 1, end, [](const char* argument) { return std::strcmp(argument, SELF_TEST_OPTION) == 0; });
        if (option == end)
            return false;

        std::copy(option + 1, end, option);
        --argumentCount;
        return true;
    }
}

int main(int argumentCount, char* arguments[])
{
    const bool selfTest = takeSelfTestOption(argumentCount, arguments);

    if (selfTest)
        g_setenv("G_MESSAGES_DEBUG", "all", FALSE);

    stapik::app::AppContext::initialize(stapik::app::AppInfo{
        .applicationId = APPLICATION_ID,
        .internalName = STAPIK_APP_NAME,
        .displayName = STAPIK_APP_DISPLAY_NAME,
        .version = STAPIK_APP_VERSION,
        .author = AUTHOR,
        .repositoryUrl = REPOSITORY_URL });

    if (selfTest)
        stapik::log::setLevel(stapik::log::Level::Info);

    if (const auto instanceGuard = stapik::app::SingleInstanceGuard::acquire(APPLICATION_ID, STAPIK_APP_DISPLAY_NAME); !instanceGuard)
        return 0;

    const auto application = Gtk::Application::create(APPLICATION_ID);
    auto styleProvider = AppStyleProvider::withCommonThemes(AppPaths::resourcesDir());

    std::unique_ptr<kpop::app::CollectionController> controller;
    std::unique_ptr<kpop::app::ImageSync> imageSync;
    int selfTestFailures = 0;

    application->signal_activate().connect([&]
    {
        if (!application->get_windows().empty())
        {
            application->get_windows().front()->present();
            return;
        }

        styleProvider.apply(ThemeManager::instance().themeId());
        ThemeManager::instance().signalThemeChanged().connect([&styleProvider]
        {
            styleProvider.apply(ThemeManager::instance().themeId());
        });

        StandardMenu::installShortcuts(*application);
        application->set_accels_for_action("win.addItem", { "<Primary>n" });

        const auto dataDirectory = AppPaths::ensureUserDataDir(STAPIK_APP_NAME);
        controller = std::make_unique<kpop::app::CollectionController>(
            dataDirectory / COLLECTION_FILE_NAME,
            dataDirectory / IMAGES_DIRECTORY_NAME,
            stapik::sync::defaultCloudSessionHooks(CLOUD_SLOT_KEY));

        controller->removeUnusedImages();
        imageSync = std::make_unique<kpop::app::ImageSync>(*controller, IMAGES_SLOT_KEY);
        imageSync->requestSync();

        auto* window = new kpop::ui::MainWindow(*controller, *imageSync, styleProvider.themes());
        application->add_window(*window);
        window->signal_hide().connect([window] { delete window; });
        window->present();

        if (selfTest)
        {
            Glib::signal_timeout().connect_once([&application, &controller, &selfTestFailures, window]
            {
                selfTestFailures = kpop::ui::runSelfTest(*window, *controller);
                application->quit();
            }, SELF_TEST_DELAY_MILLISECONDS);
            return;
        }

        if (const auto loadStatus = controller->loadStatus(); loadStatus == stapik::document::LoadStatus::Loaded || loadStatus == stapik::document::LoadStatus::Missing || loadStatus == stapik::document::LoadStatus::Corrupted)
            controller->startCloudSync();

        window->showStartupProblems();
    });

    const int status = application->run(argumentCount, arguments);
    return selfTest && selfTestFailures > 0 ? 1 : status;
}
