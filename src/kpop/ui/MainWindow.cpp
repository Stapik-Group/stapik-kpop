#include "MainWindow.hpp"

#include "Translate.hpp"

#include "kpop/domain/CollectionStatistics.hpp"
#include "kpop/domain/ItemFilter.hpp"
#include "kpop/domain/ItemSort.hpp"
#include "kpop/ui/ItemPresenter.hpp"
#include "kpop/ui/dialog/ArtistsDialog.hpp"
#include "kpop/ui/dialog/ItemDialog.hpp"

#include "stapik/storage/PathText.hpp"
#include "stapik/domain/MoneyFormatter.hpp"
#include "stapik/ui/dialog/ConfirmDialog.hpp"
#include "stapik/ui/dialog/ConnectDialog.hpp"
#include "stapik/ui/dialog/DialogUtils.hpp"

#include <giomm/menu.h>

#include <algorithm>

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
    }

    StandardMenuOptions MainWindow::menuOptions(const stapik::theme::ThemeRegistry& themes, stapik::command::UndoStack& undoStack)
    {
        StandardMenuOptions options;
        options.themes = &themes;
        options.undoStack = &undoStack;
        return options;
    }

    MainWindow::MainWindow(app::CollectionController& controller, app::ImageSync& imageSync, const stapik::theme::ThemeRegistry& themes) :
        m_controller(controller),
        m_menu(*this, menuOptions(themes, controller.undoStack())),
        m_rootBox(Gtk::Orientation::VERTICAL),
        m_topBar(Gtk::Orientation::HORIZONTAL, TOP_BAR_SPACING),
        m_rightBox(Gtk::Orientation::VERTICAL),
        m_footer(Gtk::Orientation::HORIZONTAL, FOOTER_SPACING),
        m_paned(Gtk::Orientation::HORIZONTAL)
    {
        set_title(STAPIK_APP_DISPLAY_NAME);
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
    }

    void MainWindow::showStartupProblems()
    {
        using stapik::document::LoadStatus;

        const auto path = stapik::storage::pathText(m_controller.documentPath());

        switch (m_controller.loadStatus())
        {
            case LoadStatus::Loaded:
            case LoadStatus::Missing:
                return;

            case LoadStatus::Corrupted:
                showMessageDialog(*this, translate("kpop.startup.corrupted.header"), translate("kpop.startup.corrupted.text", { { "path", path } }), Gtk::MessageType::WARNING);
                return;

            case LoadStatus::NewerVersion:
            case LoadStatus::MigrationFailed:
            {
                const bool isNewer = m_controller.loadStatus() == LoadStatus::NewerVersion;

                set_sensitive(false);

                auto* dialog = showAutoDeletingDialog<Gtk::MessageDialog>(
                    *this,
                    translate(isNewer ? "kpop.startup.newerVersion.header" : "kpop.startup.migrationFailed.header"),
                    false,
                    Gtk::MessageType::ERROR,
                    Gtk::ButtonsType::CLOSE,
                    true);
                dialog->set_secondary_text(translate(isNewer ? "kpop.startup.newerVersion.text" : "kpop.startup.migrationFailed.text", { { "path", path } }));
                dialog->signal_hide().connect([this] { close(); });
            }
        }
    }

    void MainWindow::initActions()
    {
        add_action("addItem", [this] { onAddRequested(); });
        add_action("manageArtists", [this] { onManageArtistsRequested(); });
        add_action("connect", [this] { onConnectRequested(); });
        add_action("sync", [this] { onSyncRequested(); });
        add_action("quit", [this] { close(); });
    }

    void MainWindow::initMenu()
    {
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

        m_topBar.set_margin(TOP_BAR_MARGIN);
        m_topBar.append(m_filterBar);
        m_topBar.append(m_addButton);

        m_sidebar.set_size_request(SIDEBAR_WIDTH, -1);

        m_rightBox.set_hexpand(true);
        m_rightBox.append(m_topBar);
        m_rightBox.append(m_listView);
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

        m_listView.signalEditRequested().connect([this](const std::string& itemId) { onEditRequested(itemId); });
        m_listView.signalDeleteRequested().connect([this](const std::string& itemId) { onDeleteRequested(itemId); });

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
        m_saveFailedConnection = m_controller.signalSaveFailed().connect([this]
        {
            showMessageDialog(
                *this,
                translate("kpop.error.saveFailed.header"),
                translate("kpop.error.saveFailed.text", { { "path", stapik::storage::pathText(m_controller.documentPath()) } }),
                Gtk::MessageType::ERROR);
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
        m_listView.refreshPlaceholder();
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

        domain::sortItems(matchingItems, m_filterBar.sortOrder());

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
            if (item.image)
            {
                if (const auto imagePath = m_controller.imageLibrary().pathOf(*item.image))
                    row.imagePath = *imagePath;
            }

            pageRows.push_back(std::move(row));
        }

        m_listView.setRows(pageRows);
        m_paginationBar.setPage(m_currentPage, pageCount);
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
                showMessageDialog(*this, translate("cloud.failed.header"), "", Gtk::MessageType::ERROR);
                return;
            }

            m_syncIndicator.set_visible(true);
            showMessageDialog(*this, translate("cloud.connected"), translate("cloud.connected.secondary"), Gtk::MessageType::INFO);
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
