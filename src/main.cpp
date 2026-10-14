#include "kpop/app/CollectionController.hpp"
#include "kpop/app/ImageSync.hpp"
#include "kpop/ui/MainWindow.hpp"
#include "kpop/ui/SelfTest.hpp"

#include "stapik/app/StapikApplication.hpp"
#include "stapik/app/version.hpp"
#include "stapik/log/Log.hpp"
#include "stapik/storage/AppPaths.hpp"
#include "stapik/sync/CloudSessionHooks.hpp"

#include <glib.h>
#include <glibmm/main.h>

#include <algorithm>
#include <cstring>
#include <memory>
#include <string>

namespace
{
    constexpr auto APPLICATION_ID = "pl.stapik.kpop";
    constexpr auto AUTHOR = "Sebastian Smoliński";
    constexpr auto REPOSITORY_URL = "https://github.com/Stapik-Group/stapik-kpop";
    constexpr auto COLLECTION_FILE_NAME = "collection.json";
    constexpr auto IMAGES_DIRECTORY_NAME = "images";
    constexpr auto CLOUD_SLOT_KEY = "stapikkpop.json";
    constexpr auto IMAGES_SLOT_KEY = "covers";
    constexpr auto PHOTOS_DIRECTORY_NAME = "photos";
    constexpr auto PHOTOS_SLOT_KEY = "photos";
    constexpr auto SIDEBAR_PANED_NAME = "sidebar";
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

    // The self-test has to work without a network, so it does not look for a newer version.
    stapik::app::StapikApplicationOptions options;
    options.checkForUpdates = !selfTest;

    const auto application = stapik::app::StapikApplication::create(stapik::app::AppInfo{
        .applicationId = APPLICATION_ID,
        .internalName = std::string{ stapik::app::build::NAME },
        .displayName = std::string{ stapik::app::build::DISPLAY_NAME },
        .version = std::string{ stapik::app::build::VERSION },
        .author = AUTHOR,
        .repositoryUrl = REPOSITORY_URL },
        options);

    // --log-level, when given, is applied later and wins over this.
    if (selfTest)
        stapik::log::setLevel(stapik::log::Level::Info);

    std::unique_ptr<kpop::app::CollectionController> controller;
    std::unique_ptr<kpop::app::ImageSync> imageSync;
    std::unique_ptr<kpop::app::ImageSync> photoSync;
    int selfTestFailures = 0;

    application->signal_activate().connect([&]
    {
        if (!application->get_windows().empty())
        {
            application->get_windows().front()->present();
            return;
        }

        // The theme, the font and the standard shortcuts are handled by StapikApplication.
        application->set_accels_for_action("win.addItem", { "<Primary>n" });

        const auto dataDirectory = AppPaths::ensureUserDataDir(std::string{ stapik::app::build::NAME });
        controller = std::make_unique<kpop::app::CollectionController>(
            dataDirectory / COLLECTION_FILE_NAME,
            dataDirectory / IMAGES_DIRECTORY_NAME,
            dataDirectory / PHOTOS_DIRECTORY_NAME,
            stapik::sync::defaultCloudSessionHooks(CLOUD_SLOT_KEY));

        // Safe mode is for troubleshooting: nothing is cleaned up and nothing is synchronised on its own.
        const bool safeMode = application->commandLine().safeMode;

        if (!safeMode)
            controller->removeUnusedImages();

        imageSync = std::make_unique<kpop::app::ImageSync>(
            *controller,
            controller->imageLibrary(),
            [&controller] { return controller->document().referencedImages(); },
            IMAGES_SLOT_KEY);
        if (!safeMode)
            imageSync->requestSync();

        photoSync = std::make_unique<kpop::app::ImageSync>(
            *controller,
            controller->photoLibrary(),
            [&controller] { return controller->document().referencedPhotos(); },
            PHOTOS_SLOT_KEY);
        if (!safeMode)
            photoSync->requestSync();

        auto* window = new kpop::ui::MainWindow(*controller, *imageSync, application->themes());
        application->rememberPanedPosition(window->sidebarPaned(), SIDEBAR_PANED_NAME);
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

        if (const auto loadStatus = controller->loadStatus(); !safeMode && (loadStatus == stapik::document::LoadStatus::Loaded || loadStatus == stapik::document::LoadStatus::Missing || loadStatus == stapik::document::LoadStatus::Corrupted))
            controller->startCloudSync();

        window->showStartupProblems();
    });

    const int status = application->run(argumentCount, arguments);
    return selfTest && selfTestFailures > 0 ? 1 : status;
}
