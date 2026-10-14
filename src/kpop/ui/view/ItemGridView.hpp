#pragma once

#include "EmptyListKind.hpp"

#include "kpop/ui/ItemPresenter.hpp"
#include "kpop/ui/widget/ItemContextMenu.hpp"

#include <gtkmm/box.h>
#include <gtkmm/flowbox.h>
#include <gtkmm/scrolledwindow.h>

#include <sigc++/signal.h>

#include <string>
#include <vector>

namespace kpop::ui
{
    // The entries as a grid of tiles: a square cover with the title and the subtitle below it.
    class ItemGridView : public Gtk::ScrolledWindow
    {
    public:
        ItemGridView();

        void setRows(const std::vector<ItemRow>& rows);
        void setEmptyKind(EmptyListKind kind);

        sigc::signal<void()>& signalAddRequested();

        sigc::signal<void(const std::string&)>& signalEditRequested();
        sigc::signal<void(const std::string&)>& signalDeleteRequested();
        sigc::signal<void(const std::string&)>& signalDuplicateRequested();
        sigc::signal<void(const std::string&)>& signalDuplicateAndEditRequested();

    private:
        void clear();
        void appendRow(const ItemRow& row);

        Gtk::Box m_content;
        ::EmptyState m_emptyState;
        Gtk::FlowBox m_flowBox;
        std::vector<std::string> m_rowIds;
        sigc::signal<void(const std::string&)> m_signalEditRequested;
        sigc::signal<void(const std::string&)> m_signalDeleteRequested;
        sigc::signal<void(const std::string&)> m_signalDuplicateRequested;
        sigc::signal<void(const std::string&)> m_signalDuplicateAndEditRequested;

        // After the flow box it is attached to, so it is destroyed first.
        ItemContextMenu m_contextMenu;
    };
}
