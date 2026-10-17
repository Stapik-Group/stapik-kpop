#pragma once

#include "EmptyListKind.hpp"

#include "kpop/ui/ItemPresenter.hpp"
#include "kpop/ui/widget/ItemContextMenu.hpp"

#include <gdkmm/texture.h>
#include <glibmm/refptr.h>
#include <gtkmm/box.h>
#include <gtkmm/scrolledwindow.h>

#include <sigc++/connection.h>
#include <sigc++/signal.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace kpop::ui
{
    // The entries as covers standing on shelves. Every shelf is a centred row of covers with a board below it that
    // spans the whole width, so the boards always line up with the covers. The number of covers on a shelf follows the
    // width of the window; the covers themselves keep their size.
    class ItemShelfView : public Gtk::ScrolledWindow
    {
    public:
        ItemShelfView();
        ~ItemShelfView() override;

        void setRows(const std::vector<ItemRow>& rows);
        void setEmptyKind(EmptyListKind kind);

        sigc::signal<void()>& signalAddRequested();

        sigc::signal<void(const std::string&)>& signalEditRequested();
        sigc::signal<void(const std::string&)>& signalDeleteRequested();
        sigc::signal<void(const std::string&)>& signalDuplicateRequested();
        sigc::signal<void(const std::string&)>& signalDuplicateAndEditRequested();

    private:
        void clear();
        void rebuildShelves();
        void updateColumns();
        void scheduleRelayout();
        void select(std::optional<std::size_t> index);
        [[nodiscard]] Gtk::Widget* createTile(std::size_t index);
        [[nodiscard]] std::optional<std::size_t> tileIndexAt(double x, double y);
        [[nodiscard]] bool onKeyPressed(guint keyval);

        Gtk::Box m_content;
        ::EmptyState m_emptyState;
        Gtk::Box m_shelves;

        std::vector<ItemRow> m_rows;
        std::vector<Glib::RefPtr<Gdk::Texture>> m_textures;
        std::vector<Gtk::Widget*> m_tiles;
        std::optional<std::size_t> m_selected;
        int m_columns;

        sigc::connection m_relayoutConnection;
        std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
        sigc::signal<void(const std::string&)> m_signalEditRequested;
        sigc::signal<void(const std::string&)> m_signalDeleteRequested;
        sigc::signal<void(const std::string&)> m_signalDuplicateRequested;
        sigc::signal<void(const std::string&)> m_signalDuplicateAndEditRequested;

        // After the box it is attached to, so it is destroyed first.
        ItemContextMenu m_contextMenu;
    };
}
