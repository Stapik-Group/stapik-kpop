#pragma once

#include "kpop/app/CollectionController.hpp"
#include "kpop/app/ImageSync.hpp"
#include "kpop/ui/view/ItemGridView.hpp"
#include "kpop/ui/view/ItemListView.hpp"
#include "kpop/ui/view/ItemShelfView.hpp"
#include "kpop/ui/view/KindSidebar.hpp"
#include "kpop/ui/widget/FilterBar.hpp"
#include "kpop/ui/widget/PaginationBar.hpp"

#include "stapik/settings/ObservableSetting.hpp"
#include "stapik/task/BackgroundTaskRunner.hpp"
#include "stapik/theme/ThemeRegistry.hpp"
#include "stapik/ui/menu/StandardMenu.hpp"
#include "stapik/ui/widget/StatusIndicator.hpp"

#include <giomm/simpleaction.h>

#include <gtkmm/applicationwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/paned.h>
#include <gtkmm/stack.h>

#include <sigc++/connection.h>

#include <filesystem>
#include <string>

namespace kpop::ui
{
    class MainWindow : public Gtk::ApplicationWindow
    {
    public:
        MainWindow(app::CollectionController& controller, app::ImageSync& imageSync, const stapik::theme::ThemeRegistry& themes);
        ~MainWindow() override;

        MainWindow(const MainWindow&) = delete;
        MainWindow& operator=(const MainWindow&) = delete;
        void showStartupProblems();

        // The divider between the sidebar and the list, for StapikApplication to remember its position.
        [[nodiscard]] Gtk::Paned& sidebarPaned()
        {
            return m_paned;
        }

    private:
        static constexpr int DEFAULT_WIDTH = 1180;
        static constexpr int DEFAULT_HEIGHT = 760;
        static constexpr std::size_t PAGE_SIZE = 50;

        void initActions();
        void initMenu();
        void initLayout();
        void initSignals(app::ImageSync& imageSync);

        void refreshAll();
        void refreshLanguage();
        void refreshList();
        void refreshViewMode();
        void refreshStatistics();
        void resetToFirstPage();

        enum class ViewMode
        {
            List,
            Shelf,
            Grid
        };

        void onViewModeRequested(const Glib::ustring& mode);
        [[nodiscard]] Gtk::Widget& activeView();

        void onAddRequested();
        void onEditRequested(const std::string& itemId);
        void onDeleteRequested(const std::string& itemId);
        void onDuplicateRequested(const std::string& itemId);
        void onDuplicateAndEditRequested(const std::string& itemId);
        void onConnectRequested();
        void onSyncRequested();
        void onManageArtistsRequested();
        void onRestoreBackupRequested();
        void onExportArchiveRequested();
        void onImportArchiveRequested();
        void exportArchive(const std::filesystem::path& target);
        void importArchive(const std::filesystem::path& source);
        void reloadSettings();
        [[nodiscard]] app::ArchiveOptions archiveOptions() const;

        static std::string statisticsText(const domain::CollectionStatistics &statistics);
        [[nodiscard]] static StandardMenuOptions menuOptions(const stapik::theme::ThemeRegistry& themes, stapik::command::UndoStack& undoStack);

        app::CollectionController& m_controller;

        // Declared before the menu, whose settings window refers to it.
        stapik::settings::ObservableSetting<bool> m_checkForUpdates;
        StandardMenu m_menu;

        Gtk::Box m_rootBox;
        Gtk::Box m_topBar;
        Gtk::Box m_rightBox;
        Gtk::Box m_footer;
        Gtk::Paned m_paned;
        FilterBar m_filterBar;
        Gtk::Button m_addButton;
        KindSidebar m_sidebar;
        Gtk::Stack m_viewStack;
        ItemListView m_listView;
        ItemShelfView m_shelfView;
        ItemGridView m_gridView;
        PaginationBar m_paginationBar;
        Gtk::Label m_statisticsLabel;
        StatusIndicator m_syncIndicator;

        ViewMode m_viewMode = ViewMode::List;
        Glib::RefPtr<Gio::SimpleAction> m_viewModeAction;

        std::size_t m_currentPage = 1;
        sigc::connection m_documentConnection;
        sigc::connection m_imagesConnection;
        sigc::connection m_syncConnection;
        sigc::connection m_localeConnection;
        sigc::connection m_saveFailedConnection;
        sigc::connection m_apiKeyConnection;

        // Last, so that it is destroyed first and waits for a running export before the rest goes away.
        stapik::task::BackgroundTaskRunner m_archiveRunner;
    };
}
