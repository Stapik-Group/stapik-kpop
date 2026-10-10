#pragma once

#include <giomm/simpleactiongroup.h>

#include <gtkmm/popovermenu.h>
#include <gtkmm/widget.h>

#include <sigc++/signal.h>

#include <string>

namespace kpop::ui
{
    // The menu of an entry that opens on the right mouse button: edit, duplicate, duplicate and edit, remove.
    // It is a child of the widget it is attached to, so declare it after that widget in the owning class.
    class ItemContextMenu
    {
    public:
        explicit ItemContextMenu(Gtk::Widget& owner);
        ~ItemContextMenu();

        ItemContextMenu(const ItemContextMenu&) = delete;
        ItemContextMenu& operator=(const ItemContextMenu&) = delete;

        // x and y are in the coordinates of the widget the menu is attached to.
        void popup(const std::string& itemId, double x, double y);

        sigc::signal<void(const std::string&)>& signalEditRequested();
        sigc::signal<void(const std::string&)>& signalDuplicateRequested();
        sigc::signal<void(const std::string&)>& signalDuplicateAndEditRequested();
        sigc::signal<void(const std::string&)>& signalDeleteRequested();

    private:
        Gtk::PopoverMenu m_popover;
        Glib::RefPtr<Gio::SimpleActionGroup> m_actions;
        std::string m_itemId;
        sigc::signal<void(const std::string&)> m_signalEditRequested;
        sigc::signal<void(const std::string&)> m_signalDuplicateRequested;
        sigc::signal<void(const std::string&)> m_signalDuplicateAndEditRequested;
        sigc::signal<void(const std::string&)> m_signalDeleteRequested;
    };
}
