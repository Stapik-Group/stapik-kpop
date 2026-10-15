#pragma once

#include "EmptyListKind.hpp"

#include "kpop/ui/ItemPresenter.hpp"
#include "kpop/ui/widget/ItemContextMenu.hpp"

#include <gtkmm/listbox.h>
#include <gtkmm/scrolledwindow.h>

#include <sigc++/signal.h>

#include <memory>
#include <string>
#include <vector>

namespace kpop::ui
{
    class ItemListView : public Gtk::ScrolledWindow
    {
    public:
        ItemListView();
        ~ItemListView() override;

        void setRows(const std::vector<ItemRow>& rows);
        void setEmptyKind(EmptyListKind kind);

        sigc::signal<void()>& signalAddRequested();

        sigc::signal<void(const std::string&)>& signalEditRequested();
        sigc::signal<void(const std::string&)>& signalDeleteRequested();
        sigc::signal<void(const std::string&)>& signalDuplicateRequested();
        sigc::signal<void(const std::string&)>& signalDuplicateAndEditRequested();

    private:
        void clear();
        void appendRow(const ItemRow& row, bool showThumbnail);

        Gtk::ListBox m_listBox;
        ::EmptyState m_emptyState;
        std::vector<std::string> m_rowIds;
        std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
        int m_toggleOffRowIndex = -1;
        sigc::signal<void(const std::string&)> m_signalEditRequested;
        sigc::signal<void(const std::string&)> m_signalDeleteRequested;
        sigc::signal<void(const std::string&)> m_signalDuplicateRequested;
        sigc::signal<void(const std::string&)> m_signalDuplicateAndEditRequested;

        // After the list box it is attached to, so it is destroyed first.
        ItemContextMenu m_contextMenu;
    };
}
