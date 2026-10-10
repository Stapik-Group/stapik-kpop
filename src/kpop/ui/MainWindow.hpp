#pragma once

#include "kpop/app/CollectionController.hpp"
#include "kpop/app/ImageSync.hpp"
#include "kpop/ui/view/ItemListView.hpp"
#include "kpop/ui/view/ItemShelfView.hpp"
#include "kpop/ui/view/KindSidebar.hpp"
#include "kpop/ui/widget/FilterBar.hpp"
#include "kpop/ui/widget/PaginationBar.hpp"

#include "stapik/theme/ThemeRegistry.hpp"
#include "stapik/ui/menu/StandardMenu.hpp"
#include "stapik/ui/widget/StatusIndicator.hpp"

#include <gtkmm/applicationwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/paned.h>
#include <gtkmm/stack.h>
#include <gtkmm/togglebutton.h>

#include <sigc++/connection.h>

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

        [[nodiscard]] bool isShelfViewActive() const;

        void onAddRequested();
        void onEditRequested(const std::string& itemId);
        void onDeleteRequested(const std::string& itemId);
        void onDuplicateRequested(const std::string& itemId);
        void onDuplicateAndEditRequested(const std::string& itemId);
        void onConnectRequested();
        void onSyncRequested();
        void onManageArtistsRequested();

        static std::string statisticsText(const domain::CollectionStatistics &statistics);
        [[nodiscard]] static StandardMenuOptions menuOptions(const stapik::theme::ThemeRegistry& themes, stapik::command::UndoStack& undoStack);

        app::CollectionController& m_controller;
        StandardMenu m_menu;

        Gtk::Box m_rootBox;
        Gtk::Box m_topBar;
        Gtk::Box m_rightBox;
        Gtk::Box m_footer;
        Gtk::Paned m_paned;
        FilterBar m_filterBar;
        Gtk::Box m_viewSwitch;
        Gtk::ToggleButton m_listViewButton;
        Gtk::ToggleButton m_shelfViewButton;
        Gtk::Button m_addButton;
        KindSidebar m_sidebar;
        Gtk::Stack m_viewStack;
        ItemListView m_listView;
        ItemShelfView m_shelfView;
        PaginationBar m_paginationBar;
        Gtk::Label m_statisticsLabel;
        StatusIndicator m_syncIndicator;

        std::size_t m_currentPage = 1;
        sigc::connection m_documentConnection;
        sigc::connection m_imagesConnection;
        sigc::connection m_syncConnection;
        sigc::connection m_localeConnection;
        sigc::connection m_saveFailedConnection;
    };
}
