#pragma once

#include "EmptyListKind.hpp"

#include "kpop/ui/ItemPresenter.hpp"
#include "kpop/ui/widget/ItemContextMenu.hpp"

#include <gtkmm/box.h>
#include <gtkmm/flowbox.h>
#include <gtkmm/scrolledwindow.h>

#include <sigc++/connection.h>
#include <sigc++/signal.h>

#include <string>
#include <vector>

namespace kpop::ui
{
    // The entries as a grid of tiles: a square cover with the title and the subtitle below it.
    class ItemGridView : public Gtk::ScrolledWindow
    {
    public:
        static constexpr int MIN_COVER_SIZE = 80;
        static constexpr int DEFAULT_COVER_SIZE = 140;
        static constexpr int MAX_COVER_SIZE = 280;

        ItemGridView();
        ~ItemGridView() override;

        void setRows(const std::vector<ItemRow>& rows);

        // The side of the square cover in pixels. The tiles are rebuilt shortly after the last change.
        void setCoverSize(int size);
        void setEmptyKind(EmptyListKind kind);

        sigc::signal<void()>& signalAddRequested();

        sigc::signal<void(const std::string&)>& signalEditRequested();
        sigc::signal<void(const std::string&)>& signalDeleteRequested();
        sigc::signal<void(const std::string&)>& signalDuplicateRequested();
        sigc::signal<void(const std::string&)>& signalDuplicateAndEditRequested();

    private:
        void populate();
        void clear();
        void appendRow(const ItemRow& row);

        Gtk::Box m_content;
        ::EmptyState m_emptyState;
        Gtk::FlowBox m_flowBox;
        std::vector<std::string> m_rowIds;
        std::vector<ItemRow> m_rows;
        int m_coverSize = DEFAULT_COVER_SIZE;
        sigc::connection m_rebuildConnection;
        sigc::signal<void(const std::string&)> m_signalEditRequested;
        sigc::signal<void(const std::string&)> m_signalDeleteRequested;
        sigc::signal<void(const std::string&)> m_signalDuplicateRequested;
        sigc::signal<void(const std::string&)> m_signalDuplicateAndEditRequested;

        // After the flow box it is attached to, so it is destroyed first.
        ItemContextMenu m_contextMenu;
    };
}
