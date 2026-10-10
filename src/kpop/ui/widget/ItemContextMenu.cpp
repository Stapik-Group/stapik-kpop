#include "ItemContextMenu.hpp"

#include "kpop/ui/Translate.hpp"

#include <giomm/menu.h>

#include <gdkmm/rectangle.h>

namespace kpop::ui
{
    namespace
    {
        constexpr auto ACTION_GROUP = "item";
    }

    ItemContextMenu::ItemContextMenu(Gtk::Widget& owner) :
        m_popover(Gio::Menu::create()),
        m_actions(Gio::SimpleActionGroup::create())
    {
        m_popover.set_parent(owner);
        m_popover.set_has_arrow(false);
        m_popover.set_halign(Gtk::Align::START);

        m_actions->add_action("edit", [this] { m_signalEditRequested.emit(m_itemId); });
        m_actions->add_action("duplicate", [this] { m_signalDuplicateRequested.emit(m_itemId); });
        m_actions->add_action("duplicateAndEdit", [this] { m_signalDuplicateAndEditRequested.emit(m_itemId); });
        m_actions->add_action("remove", [this] { m_signalDeleteRequested.emit(m_itemId); });
        owner.insert_action_group(ACTION_GROUP, m_actions);
    }

    ItemContextMenu::~ItemContextMenu()
    {
        m_popover.unparent();
    }

    void ItemContextMenu::popup(const std::string& itemId, const double x, const double y)
    {
        // Built on every use, so the labels are always in the current language.
        auto menu = Gio::Menu::create();
        menu->append(translate("kpop.button.edit"), "item.edit");
        menu->append(translate("kpop.menu.item.duplicate"), "item.duplicate");
        menu->append(translate("kpop.menu.item.duplicateAndEdit"), "item.duplicateAndEdit");

        auto destructive = Gio::Menu::create();
        destructive->append(translate("kpop.button.remove"), "item.remove");
        menu->append_section(destructive);

        m_itemId = itemId;
        m_popover.set_menu_model(menu);
        m_popover.set_pointing_to(Gdk::Rectangle(static_cast<int>(x), static_cast<int>(y), 1, 1));
        m_popover.popup();
    }

    sigc::signal<void(const std::string&)>& ItemContextMenu::signalEditRequested()
    {
        return m_signalEditRequested;
    }

    sigc::signal<void(const std::string&)>& ItemContextMenu::signalDuplicateRequested()
    {
        return m_signalDuplicateRequested;
    }

    sigc::signal<void(const std::string&)>& ItemContextMenu::signalDuplicateAndEditRequested()
    {
        return m_signalDuplicateAndEditRequested;
    }

    sigc::signal<void(const std::string&)>& ItemContextMenu::signalDeleteRequested()
    {
        return m_signalDeleteRequested;
    }
}
