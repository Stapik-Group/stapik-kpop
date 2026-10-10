#pragma once

#include "kpop/ui/ItemPresenter.hpp"

#include <gtkmm/box.h>
#include <gtkmm/flowbox.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>

#include <sigc++/signal.h>

#include <string>
#include <vector>

namespace kpop::ui
{
    class ItemShelfView : public Gtk::ScrolledWindow
    {
    public:
        ItemShelfView();

        void setRows(const std::vector<ItemRow>& rows);
        void refreshPlaceholder();

        sigc::signal<void(const std::string&)>& signalEditRequested();
        sigc::signal<void(const std::string&)>& signalDeleteRequested();

    private:
        void clear();
        void appendRow(const ItemRow& row);

        Gtk::Box m_content;
        Gtk::Label m_placeholderLabel;
        Gtk::FlowBox m_flowBox;
        std::vector<std::string> m_rowIds;
        sigc::signal<void(const std::string&)> m_signalEditRequested;
        sigc::signal<void(const std::string&)> m_signalDeleteRequested;
    };
}
