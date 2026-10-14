#include "MainWindow.hpp"

#include "Translate.hpp"

#include "kpop/domain/ArtistColor.hpp"
#include "kpop/domain/CollectionStatistics.hpp"
#include "kpop/domain/ItemFilter.hpp"
#include "kpop/domain/ItemSort.hpp"
#include "kpop/ui/ItemPresenter.hpp"
#include "kpop/ui/dialog/ArtistsDialog.hpp"
#include "kpop/ui/dialog/ItemDialog.hpp"

#include "stapik/app/AppContext.hpp"
#include "stapik/app/version.hpp"
#include "stapik/storage/AppPaths.hpp"
#include "stapik/storage/PathText.hpp"
#include "stapik/domain/MoneyFormatter.hpp"
#include "stapik/theme/FontManager.hpp"
#include "stapik/theme/ThemeManager.hpp"
#include "stapik/locale/DateFormatter.hpp"
#include "stapik/ui/dialog/ConfirmDialog.hpp"
#include "stapik/ui/dialog/ConnectDialog.hpp"
#include "stapik/ui/dialog/DialogUtils.hpp"
#include "stapik/ui/dialog/FileDialogs.hpp"
#include "stapik/ui/dialog/RestoreBackupDialog.hpp"
#include "stapik/update/UpdateChecker.hpp"

#include <giomm/asyncresult.h>
#include <giomm/menu.h>
#include <glibmm/error.h>
#include <gtkmm/alertdialog.h>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <string>

namespace kpop::ui
{
    namespace
    {
        constexpr int SIDEBAR_WIDTH = 210;
        constexpr int FOOTER_SPACING = 16;
        constexpr int FOOTER_MARGIN = 6;
        constexpr int TOP_BAR_SPACING = 8;
        constexpr int TOP_BAR_MARGIN = 8;
        constexpr auto ADD_ITEM_ACTION = "win.addItem";
        constexpr auto TUTORIAL_DIRECTORY = "tutorial";
        constexpr auto ARCHIVE_DIRECTORY_KEY = "archive.lastDirectory";
        constexpr auto ARCHIVE_EXTENSION = ".zip";

        std::string defaultArchiveName()
        {
            const auto now = std::time(nullptr);
            std::tm local{};
            localtime_r(&now, &local);

            char date[16] = {};
            std::strftime(date, sizeof(date), "%Y-%m-%d", &local);
            return std::string{ stapik::app::build::NAME } + "-" + date + ARCHIVE_EXTENSION;
        }

        std::string archiveErrorText(const stapik::archive::ArchiveError& error)
        {
            using stapik::archive::ArchiveErrorKind;

            switch (error.kind)
            {
                case ArchiveErrorKind::NotAnArchive:
                    return translate("kpop.archive.error.notAnArchive");
                case ArchiveErrorKind::Corrupted:
                    return translate("kpop.archive.error.corrupted");
                case ArchiveErrorKind::Unsupported:
                    return translate("kpop.archive.error.unsupported");
                case ArchiveErrorKind::WrongApplication:
                    return translate("kpop.archive.error.wrongApplication");
                case ArchiveErrorKind::Io:
                case ArchiveErrorKind::Cloud:
                default:
                    return translate("kpop.archive.error.io", { { "reason", error.message() } });
            }
        }
    }

    StandardMenuOptions MainWindow::menuOptions(const stapik::theme::ThemeRegistry& themes, stapik::command::UndoStack& undoStack)
    {
        StandardMenuOptions options;
        options.themes = &themes;
        options.undoStack = &undoStack;
        options.tutorialDirectory = AppPaths::resourcesDir() / TUTORIAL_DIRECTORY;
        return options;
    }

    MainWindow::MainWindow(app::CollectionController& controller, app::ImageSync& imageSync, const stapik::theme::ThemeRegistry& themes) :
        m_controller(controller),
        m_checkForUpdates(stapik::app::AppContext::instance().settings(), stapik::update::CHECK_ENABLED_SETTING, true),
        m_menu(*this, menuOptions(themes, controller.undoStack())),
        m_rootBox(Gtk::Orientation::VERTICAL),
        m_topBar(Gtk::Orientation::HORIZONTAL, TOP_BAR_SPACING),
        m_rightBox(Gtk::Orientation::VERTICAL),
        m_footer(Gtk::Orientation::HORIZONTAL, FOOTER_SPACING),
        m_paned(Gtk::Orientation::HORIZONTAL),
        m_viewSwitch(Gtk::Orientation::HORIZONTAL)
    {
        set_title(std::string{ stapik::app::build::DISPLAY_NAME });
        set_default_size(DEFAULT_WIDTH, DEFAULT_HEIGHT);

        initActions();
        initMenu();
        initLayout();
        initSignals(imageSync);

        m_syncIndicator.set_visible(m_controller.isCloudConnected());
        m_syncIndicator.setStatus(m_controller.syncStatus());

        refreshLanguage();
        refreshAll();
    }

    MainWindow::~MainWindow()
    {
        m_documentConnection.disconnect();
        m_imagesConnection.disconnect();
        m_syncConnection.disconnect();
        m_localeConnection.disconnect();
        m_saveFailedConnection.disconnect();
        m_apiKeyConnection.disconnect();
    }

    void MainWindow::showStartupProblems()
    {
        using stapik::document::LoadStatus;

        const auto path = stapik::storage::pathText(m_controller.documentPath());

        switch (m_controller.loadStatus())
        {
            using enum LoadStatus;
            case Loaded:
            case Missing:
                return;

            case Corrupted:
                showMessageDialog(*this, translate("kpop.startup.corrupted.header"), translate("kpop.startup.corrupted.text", { { "path", path } }));
                return;

            case NewerVersion:
            case MigrationFailed:
            {
                const bool isNewer = m_controller.loadStatus() == NewerVersion;

                set_sensitive(false);

                // The collection cannot be used, so closing the message closes the window too.
                const auto dialog = Gtk::AlertDialog::create(translate(isNewer ? "kpop.startup.newerVersion.header" : "kpop.startup.migrationFailed.header"));
                dialog->set_detail(translate(isNewer ? "kpop.startup.newerVersion.text" : "kpop.startup.migrationFailed.text", { { "path", path } }));
                dialog->set_modal(true);
                dialog->set_buttons({ translate("dialog.button.ok") });
                dialog->set_default_button(0);
                dialog->set_cancel_button(0);
                dialog->choose(*this, [this, dialog](const Glib::RefPtr<Gio::AsyncResult>& result)
                {
                    try
                    {
                        dialog->choose_finish(result);
                    }
                    catch (const Glib::Error&)
                    {
                        // Dismissing the message any other way ends up the same.
                    }

                    close();
                });
            }
        }
    }

    void MainWindow::initActions()
    {
        add_action("addItem", [this] { onAddRequested(); });
        add_action("manageArtists", [this] { onManageArtistsRequested(); });
        add_action("connect", [this] { onConnectRequested(); });
        add_action("sync", [this] { onSyncRequested(); });
        add_action("restoreBackup", [this] { onRestoreBackupRequested(); });
        add_action("exportArchive", [this] { onExportArchiveRequested(); });
        add_action("importArchive", [this] { onImportArchiveRequested(); });
    }

    void MainWindow::initMenu()
    {
        m_menu.settings().addBool("kpop.settings.checkForUpdates", m_checkForUpdates);

        m_menu.addToMenu(StandardMenu::Target::File, [](Gio::Menu& menu)
        {
            menu.append(translate("menu.file.restoreBackup"), "win.restoreBackup");
            menu.append(translate("menu.file.exportArchive"), "win.exportArchive");
            menu.append(translate("menu.file.importArchive"), "win.importArchive");
        });

        m_menu.addMenu("kpop.menu.collection", [](Gio::Menu& menu)
        {
            menu.append(translate("kpop.menu.addItem"), ADD_ITEM_ACTION);
            menu.append(translate("kpop.menu.manageArtists"), "win.manageArtists");
        });
    }

    void MainWindow::initLayout()
    {
        m_addButton.set_label(translate("kpop.button.addEntry"));
        m_filterBar.set_hexpand(true);

        m_shelfViewButton.set_group(m_listViewButton);
        m_listViewButton.set_active(true);
        m_viewSwitch.add_css_class("linked");
        m_viewSwitch.append(m_listViewButton);
        m_viewSwitch.append(m_shelfViewButton);

        m_viewStack.set_vexpand(true);
        m_viewStack.add(m_listView);
        m_viewStack.add(m_shelfView);
        m_viewStack.set_visible_child(m_listView);

        m_topBar.set_margin(TOP_BAR_MARGIN);
        m_topBar.append(m_filterBar);
        m_topBar.append(m_viewSwitch);
        m_topBar.append(m_addButton);

        m_sidebar.set_size_request(SIDEBAR_WIDTH, -1);

        m_rightBox.set_hexpand(true);
        m_rightBox.append(m_topBar);
        m_rightBox.append(m_viewStack);
        m_rightBox.append(m_paginationBar);

        m_paned.set_start_child(m_sidebar);
        m_paned.set_end_child(m_rightBox);
        m_paned.set_resize_start_child(false);
        m_paned.set_shrink_start_child(false);
        m_paned.set_vexpand(true);

        m_statisticsLabel.set_hexpand(true);
        m_statisticsLabel.set_halign(Gtk::Align::START);
        m_footer.set_margin(FOOTER_MARGIN);
        m_footer.add_css_class("kpop-footer");
        m_footer.append(m_statisticsLabel);
        m_footer.append(m_syncIndicator);

        m_rootBox.append(m_menu.menuBar());
        m_rootBox.append(m_paned);
        m_rootBox.append(m_footer);

        set_child(m_rootBox);
    }

    void MainWindow::initSignals(app::ImageSync& imageSync)
    {
        m_addButton.signal_clicked().connect([this] { onAddRequested(); });

        m_sidebar.signalKindSelected().connect([this](std::optional<domain::ItemKind>)
        {
            resetToFirstPage();
            refreshList();
        });

        m_filterBar.signalChanged().connect([this]
        {
            resetToFirstPage();
            refreshList();
        });

        m_shelfViewButton.signal_toggled().connect([this] { refreshViewMode(); });

        m_listView.signalAddRequested().connect([this] { onAddRequested(); });
        m_shelfView.signalAddRequested().connect([this] { onAddRequested(); });
        m_listView.signalEditRequested().connect([this](const std::string& itemId) { onEditRequested(itemId); });
        m_listView.signalDeleteRequested().connect([this](const std::string& itemId) { onDeleteRequested(itemId); });
        m_shelfView.signalEditRequested().connect([this](const std::string& itemId) { onEditRequested(itemId); });
        m_shelfView.signalDeleteRequested().connect([this](const std::string& itemId) { onDeleteRequested(itemId); });

        for (auto* signals : { &m_listView.signalDuplicateRequested(), &m_shelfView.signalDuplicateRequested() })
            signals->connect([this](const std::string& itemId) { onDuplicateRequested(itemId); });

        for (auto* signals : { &m_listView.signalDuplicateAndEditRequested(), &m_shelfView.signalDuplicateAndEditRequested() })
            signals->connect([this](const std::string& itemId) { onDuplicateAndEditRequested(itemId); });

        m_paginationBar.signalPreviousRequested().connect([this]
        {
            m_currentPage = std::max<std::size_t>(m_currentPage, 2) - 1;
            refreshList();
        });
        m_paginationBar.signalNextRequested().connect([this]
        {
            ++m_currentPage;
            refreshList();
        });

        m_documentConnection = m_controller.signalDocumentChanged().connect([this] { refreshAll(); });
        m_imagesConnection = imageSync.signalImagesDownloaded().connect([this] { refreshList(); });
        m_syncConnection = m_controller.signalSyncStatusChanged().connect([this](const stapik::sync::SyncStatus status)
        {
            m_syncIndicator.setStatus(status);
        });
        m_localeConnection = LocaleManager::instance().signalLocaleChanged().connect([this]
        {
            refreshLanguage();
            refreshAll();
        });
        m_apiKeyConnection = m_controller.signalApiKeyExpiring().connect([this](const std::chrono::system_clock::time_point expiresAt)
        {
            const auto date = stapik::locale::formatDate(stapik::locale::toLocalTime(expiresAt), LocaleManager::instance().languageCode());
            showMessageDialog(*this, translate("kpop.cloud.apiKeyExpiring.header"), translate("cloud.apiKeyExpiring", { { "date", date } }));
        });
        m_saveFailedConnection = m_controller.signalSaveFailed().connect([this](const std::string& reason)
        {
            showMessageDialog(
                *this,
                translate("kpop.error.saveFailed.header"),
                translate("kpop.error.saveFailed.text", { { "path", stapik::storage::pathText(m_controller.documentPath()) }, { "reason", reason } }));
        });

        signal_close_request().connect([this]
        {
            m_controller.flush();
            return false;
        }, false);
    }

    void MainWindow::refreshAll()
    {
        m_filterBar.setArtists(m_controller.document().artists());
        refreshStatistics();
        refreshList();
    }

    void MainWindow::refreshLanguage()
    {
        m_addButton.set_label(translate("kpop.button.addEntry"));
        m_sidebar.refreshLabels();
        m_filterBar.refreshLabels();
        m_listViewButton.set_label(translate("kpop.view.list"));
        m_shelfViewButton.set_label(translate("kpop.view.shelf"));
        m_paginationBar.refreshLabels();
    }

    void MainWindow::refreshList()
    {
        const auto& document = m_controller.document();

        auto filter = m_filterBar.filter();
        filter.kind = m_sidebar.selectedKind();

        std::vector<domain::SortableItem> matchingItems;
        for (auto item = document.items().rbegin(); item != document.items().rend(); ++item)
        {
            if (const auto* artist = document.findArtist(item->artistId); domain::matchesFilter(filter, *item, artist))
                matchingItems.push_back({ .item = &*item, .artist = artist });
        }

        domain::sortItems(matchingItems, m_filterBar.sortOrder(), LocaleManager::instance().languageCode());

        const auto pageCount = std::max<std::size_t>(1, (matchingItems.size() + PAGE_SIZE - 1) / PAGE_SIZE);
        m_currentPage = std::clamp<std::size_t>(m_currentPage, 1, pageCount);

        const auto firstIndex = (m_currentPage - 1) * PAGE_SIZE;
        const auto lastIndex = std::min(firstIndex + PAGE_SIZE, matchingItems.size());

        std::vector<ItemRow> pageRows;
        const auto& languageCode = LocaleManager::instance().languageCode();
        for (auto index = firstIndex; index < lastIndex; ++index)
        {
            const auto& item = *matchingItems[index].item;

            auto row = describeItem(item, matchingItems[index].artist, languageCode);
            if (matchingItems[index].artist != nullptr)
            {
                if (const auto artistIndex = m_controller.document().indexOfArtist(item.artistId))
                    row.artistColor = domain::artistColor(*artistIndex);
            }

            if (item.image)
            {
                if (const auto imagePath = m_controller.imageLibrary().pathOf(*item.image))
                    row.imagePath = *imagePath;
            }

            pageRows.push_back(std::move(row));
        }

        const auto emptyKind = document.items().empty() ? EmptyListKind::NoEntries : EmptyListKind::NoMatches;

        if (isShelfViewActive())
        {
            m_shelfView.setEmptyKind(emptyKind);
            m_shelfView.setRows(pageRows);
        }
        else
        {
            m_listView.setEmptyKind(emptyKind);
            m_listView.setRows(pageRows);
        }

        m_paginationBar.setPage(m_currentPage, pageCount);
    }

    void MainWindow::refreshViewMode()
    {
        m_viewStack.set_visible_child(isShelfViewActive() ? static_cast<Widget&>(m_shelfView) : m_listView);
        refreshList();
    }

    bool MainWindow::isShelfViewActive() const
    {
        return m_shelfViewButton.get_active();
    }

    void MainWindow::refreshStatistics()
    {
        const auto statistics = domain::computeStatistics(m_controller.document().items());
        m_sidebar.setStatistics(statistics);
        m_statisticsLabel.set_text(statisticsText(statistics));
    }

    void MainWindow::resetToFirstPage()
    {
        m_currentPage = 1;
    }

    void MainWindow::onAddRequested()
    {
        ItemDialogOptions options;
        if (const auto kind = m_sidebar.selectedKind())
            options.defaultKind = *kind;
        if (const auto artistId = m_filterBar.filter().artistId)
            options.defaultArtistId = *artistId;

        showItemDialog(*this, m_controller, options, [this](domain::CollectionItem item)
        {
            m_controller.addItem(std::move(item));
        });
    }

    void MainWindow::onEditRequested(const std::string& itemId)
    {
        const auto* item = m_controller.document().findItem(itemId);
        if (item == nullptr)
            return;

        ItemDialogOptions options;
        options.existing = *item;

        showItemDialog(*this, m_controller, options, [this](const domain::CollectionItem& edited)
        {
            m_controller.updateItem(edited);
        });
    }

    void MainWindow::onDuplicateRequested(const std::string& itemId)
    {
        m_controller.duplicateItem(itemId);
    }

    // The copy is only added when the dialog is accepted.
    void MainWindow::onDuplicateAndEditRequested(const std::string& itemId)
    {
        const auto* item = m_controller.document().findItem(itemId);
        if (item == nullptr)
            return;

        ItemDialogOptions options;
        options.prefill = domain::duplicateOf(*item);

        showItemDialog(*this, m_controller, options, [this](domain::CollectionItem copy)
        {
            m_controller.addItem(std::move(copy));
        });
    }

    void MainWindow::onDeleteRequested(const std::string& itemId)
    {
        const auto* item = m_controller.document().findItem(itemId);
        if (item == nullptr)
            return;

        showConfirmDialog(
            *this,
            ConfirmDialogOptions{
                .title = translate("kpop.dialog.deleteItem.title"),
                .message = translate("kpop.dialog.deleteItem.message", { { "title", item->title } }),
                .confirmLabel = translate("kpop.button.remove"),
                .destructive = true },
            [this, itemId] { m_controller.removeItem(itemId); });
    }

    void MainWindow::onConnectRequested()
    {
        showConnectDialog(*this, m_controller.cloudConfig(), [this](const CloudStorageConfig& config)
        {
            if (!m_controller.connectCloud(config))
            {
                showMessageDialog(*this, translate("cloud.failed.header"), "");
                return;
            }

            m_syncIndicator.set_visible(true);
            showMessageDialog(*this, translate("cloud.connected"), translate("cloud.connected.secondary"));
        });
    }

    void MainWindow::onSyncRequested()
    {
        if (!m_controller.isCloudConnected())
        {
            onConnectRequested();
            return;
        }

        m_controller.syncNow();
    }

    void MainWindow::onRestoreBackupRequested()
    {
        showRestoreBackupDialog(*this, m_controller.listBackups(), [this](const stapik::document::BackupInfo& backup)
        {
            if (m_controller.restoreBackup(backup) == stapik::document::LoadStatus::Loaded)
                return;

            showMessageDialog(
                *this,
                translate("kpop.restore.failed.header"),
                translate("kpop.restore.failed.text", { { "path", stapik::storage::pathText(backup.path) } }));
        });
    }

    app::ArchiveOptions MainWindow::archiveOptions() const
    {
        return app::ArchiveOptions{
            .appName = std::string{ stapik::app::build::NAME },
            .appVersion = std::string{ stapik::app::build::VERSION },
            .settings = &stapik::app::AppContext::instance().settings() };
    }

    void MainWindow::onExportArchiveRequested()
    {
        const FileDialogOptions options{
            .title = translate("kpop.archive.export.title"),
            .initialName = defaultArchiveName(),
            .lastDirectoryKey = ARCHIVE_DIRECTORY_KEY };

        showSaveFileDialog(*this, options, [this](const std::filesystem::path& chosen)
        {
            auto target = chosen;
            if (target.extension().empty())
                target += ARCHIVE_EXTENSION;

            exportArchive(target);
        }, [this](const std::string& reason)
        {
            showMessageDialog(*this, translate("kpop.archive.export.failed.header"), translate("kpop.archive.error.io", { { "reason", reason } }));
        });
    }

    void MainWindow::exportArchive(const std::filesystem::path& target)
    {
        // The window waits for the copy, so nothing is edited while it is made.
        set_sensitive(false);

        m_archiveRunner.submit(m_controller.prepareArchiveExport(target, archiveOptions()), [this, target](auto outcome)
        {
            set_sensitive(true);

            // The runner hands over an optional, which is always filled once the work has run.
            if (!outcome)
                return;

            const auto& result = *outcome;
            if (!result)
            {
                showMessageDialog(*this, translate("kpop.archive.export.failed.header"), archiveErrorText(result.error()));
                return;
            }

            showMessageDialog(
                *this,
                translate("kpop.archive.export.done.header"),
                translate("kpop.archive.export.done.text", {
                    { "path", stapik::storage::pathText(target) },
                    { "images", std::to_string(result->assetCount) } }));
        });
    }

    void MainWindow::onImportArchiveRequested()
    {
        const FileDialogOptions options{
            .title = translate("kpop.archive.import.title"),
            .initialName = {},
            .lastDirectoryKey = ARCHIVE_DIRECTORY_KEY };

        showOpenFileDialog(*this, options, [this](const std::filesystem::path& source)
        {
            showConfirmDialog(*this, ConfirmDialogOptions{
                .title = translate("dialog.archive.import.title"),
                .message = translate("kpop.archive.import.message"),
                .confirmLabel = translate("dialog.archive.import.button"),
                .destructive = true }, [this, source] { importArchive(source); });
        }, [this](const std::string& reason)
        {
            showMessageDialog(*this, translate("kpop.archive.import.failed.header"), translate("kpop.archive.error.io", { { "reason", reason } }));
        });
    }

    void MainWindow::importArchive(const std::filesystem::path& source)
    {
        const auto result = m_controller.importArchive(source, archiveOptions());
        if (!result)
        {
            showMessageDialog(*this, translate("kpop.archive.import.failed.header"), archiveErrorText(result.error()));
            return;
        }

        reloadSettings();

        auto text = translate("kpop.archive.import.done.text", {
            { "images", std::to_string(result->assetsUploaded + result->assetsUnchanged) } });
        if (!result->settingsRestored)
            text += "\n\n" + translate("kpop.archive.import.settingsSkipped");

        showMessageDialog(*this, translate("kpop.archive.import.done.header"), text);
    }

    // The settings in memory do not notice that the file was replaced, and these are the ones that are shown.
    void MainWindow::reloadSettings()
    {
        LocaleManager::instance().languageSetting().reload();
        ThemeManager::instance().themeSetting().reload();
        FontManager::instance().fontSetting().reload();
        m_checkForUpdates.reload();
    }

    void MainWindow::onManageArtistsRequested()
    {
        showAutoDeletingDialog<ArtistsDialog>(*this, m_controller);
    }

    std::string MainWindow::statisticsText(const domain::CollectionStatistics& statistics)
    {
        auto text = translate("kpop.stats.summary", {
            { "entries", std::to_string(statistics.totalEntries()) },
            { "copies", std::to_string(statistics.ownedCopies) } });

        if (statistics.spent.empty())
            return text;

        std::string amounts;
        for (const auto& spent : statistics.spent)
        {
            if (!amounts.empty())
                amounts += ", ";

            amounts += stapik::domain::formatMoney(spent, LocaleManager::instance().languageCode());
        }

        return text + "  ·  " + translate("kpop.stats.spent", { { "amount", amounts } });
    }
}
