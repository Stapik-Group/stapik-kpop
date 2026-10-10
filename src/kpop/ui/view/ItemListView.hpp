#pragma once

#include "kpop/ui/ItemPresenter.hpp"
#include "kpop/ui/widget/ItemContextMenu.hpp"

#include <gtkmm/label.h>
#include <gtkmm/listbox.h>
#include <gtkmm/scrolledwindow.h>

#include <sigc++/signal.h>

#include <string>
#include <vector>

namespace kpop::ui
{
    class ItemListView : public Gtk::ScrolledWindow
    {
    public:
        ItemListView();

        void setRows(const std::vector<ItemRow>& rows);
        void refreshPlaceholder();

        sigc::signal<void(const std::string&)>& signalEditRequested();
        sigc::signal<void(const std::string&)>& signalDeleteRequested();
        sigc::signal<void(const std::string&)>& signalDuplicateRequested();
        sigc::signal<void(const std::string&)>& signalDuplicateAndEditRequested();

    private:
        void clear();
        void appendRow(const ItemRow& row, bool showThumbnail);

        Gtk::ListBox m_listBox;
        Gtk::Label m_placeholderLabel;
        std::vector<std::string> m_rowIds;
        sigc::signal<void(const std::string&)> m_signalEditRequested;
        sigc::signal<void(const std::string&)> m_signalDeleteRequested;
        sigc::signal<void(const std::string&)> m_signalDuplicateRequested;
        sigc::signal<void(const std::string&)> m_signalDuplicateAndEditRequested;

        // After the list box it is attached to, so it is destroyed first.
        ItemContextMenu m_contextMenu;
    };
}
