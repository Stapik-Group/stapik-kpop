#include "ItemShelfView.hpp"

#include "kpop/ui/Translate.hpp"
#include "kpop/ui/widget/ImageTexture.hpp"

#include <gdk/gdk.h>
#include <glibmm/main.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/eventcontrollerkey.h>
#include <gtkmm/gestureclick.h>
#include <gtkmm/overlay.h>
#include <gtkmm/picture.h>
#include <gtkmm/scrolledwindow.h>

#include <algorithm>

namespace kpop::ui
{
    namespace
    {
        constexpr int COVER_SIZE = 110;
        constexpr int COVER_MARGIN_TOP = 8;
        constexpr int COVER_MARGIN_SIDE = 8;
        constexpr int BOARD_HEIGHT = 14;
        constexpr int ROW_SPACING = 12;
        constexpr int TILE_WIDTH = COVER_SIZE + 2 * COVER_MARGIN_SIDE;
        constexpr int TITLE_LINES = 2;
        constexpr int SHELF_SIDE_PADDING = 8;
        constexpr int DEFAULT_COLUMNS = 6;
        constexpr int RELAYOUT_DELAY_MS = 100;

        Gtk::Widget* makeCover(const Glib::RefPtr<Gdk::Texture>& texture)
        {
            // The texture is never larger than COVER_SIZE, so its natural size cannot grow the slot.
            if (texture)
            {
                // Fixed-size slot: every cover occupies exactly COVER_SIZE x COVER_SIZE.
                // The picture inside is centred in it and keeps its aspect ratio.
                auto* slot = Gtk::make_managed<Gtk::Box>();
                slot->set_size_request(COVER_SIZE, COVER_SIZE);
                slot->set_halign(Gtk::Align::CENTER);
                slot->set_valign(Gtk::Align::START);
                slot->set_hexpand(false);
                slot->set_vexpand(false);
                slot->set_overflow(Gtk::Overflow::HIDDEN);
                slot->add_css_class("kpop-shelf-cover");

                // Sized to its texture: a picture that only has a minimum size grows to the proportions of its image,
                // and a wide cover would then make its tile wider than the others.
                slot->append(*makeExactPicture(texture));
                return slot;
            }

            auto* empty = Gtk::make_managed<Gtk::Label>("♪");
            empty->set_size_request(COVER_SIZE, COVER_SIZE);
            empty->set_halign(Gtk::Align::CENTER);
            empty->set_valign(Gtk::Align::START);
            empty->add_css_class("kpop-shelf-cover");
            empty->add_css_class("kpop-shelf-cover-empty");
            return empty;
        }

        Gtk::Label* makeTitle(const std::string& text)
        {
            auto* title = Gtk::make_managed<Gtk::Label>(text);
            title->add_css_class("kpop-shelf-title");
            // The tiny maximum width keeps a long title from widening the tile; it only takes the width it is given.
            title->set_max_width_chars(1);
            title->set_halign(Gtk::Align::FILL);
            title->set_valign(Gtk::Align::END);
            title->set_wrap(true);
            title->set_wrap_mode(Pango::WrapMode::WORD_CHAR);
            title->set_lines(TITLE_LINES);
            title->set_ellipsize(Pango::EllipsizeMode::END);
            title->set_justify(Gtk::Justification::CENTER);
            return title;
        }

        std::string tooltipOf(const ItemRow& row)
        {
            std::string text = row.title;

            for (const auto* part : { &row.subtitle, &row.regionName, &row.statusText })
            {
                if (!part->empty())
                    text += "\n" + *part;
            }

            return text;
        }
    }

    ItemShelfView::ItemShelfView() :
        m_content(Gtk::Orientation::VERTICAL),
        m_shelves(Gtk::Orientation::VERTICAL),
        m_columns(DEFAULT_COLUMNS),
        m_contextMenu(m_content)
    {
        set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
        set_vexpand(true);
        set_child(m_content);
        add_css_class("kpop-shelf-scroller");

        showEmptyListKind(m_emptyState, EmptyListKind::NoEntries);
        m_emptyState.set_visible(false);

        m_content.add_css_class("kpop-shelf");
        m_content.set_hexpand(true);
        m_content.set_focusable(true);
        m_content.append(m_emptyState);
        m_content.append(m_shelves);

        // The viewport changes its size whenever the window does; the number of covers per shelf follows it.
        get_hadjustment()->signal_changed().connect([this] { updateColumns(); });

        const auto leftClick = Gtk::GestureClick::create();
        leftClick->set_button(GDK_BUTTON_PRIMARY);
        leftClick->signal_pressed().connect([this](const int pressCount, const double x, const double y)
        {
            m_content.grab_focus();

            const auto index = tileIndexAt(x, y);
            if (!index)
                return;

            // A click on the highlighted cover removes the highlight again; the second click of a double click
            // opens the entry and keeps it highlighted.
            if (pressCount == 1)
                select(m_selected == index ? std::nullopt : index);
            else if (pressCount == 2)
            {
                select(index);
                m_signalEditRequested.emit(m_rows[*index].id);
            }
        });
        m_content.add_controller(leftClick);

        const auto rightClick = Gtk::GestureClick::create();
        rightClick->set_button(GDK_BUTTON_SECONDARY);
        rightClick->signal_pressed().connect([this](const int, const double x, const double y)
        {
            if (const auto index = tileIndexAt(x, y))
            {
                select(index);
                m_contextMenu.popup(m_rows[*index].id, x, y);
            }
        });
        m_content.add_controller(rightClick);

        const auto keyController = Gtk::EventControllerKey::create();
        keyController->signal_key_pressed().connect([this](const guint keyval, guint, Gdk::ModifierType) { return onKeyPressed(keyval); }, false);
        m_content.add_controller(keyController);

        m_contextMenu.signalEditRequested().connect([this](const std::string& itemId) { m_signalEditRequested.emit(itemId); });
        m_contextMenu.signalDeleteRequested().connect([this](const std::string& itemId) { m_signalDeleteRequested.emit(itemId); });
        m_contextMenu.signalDuplicateRequested().connect([this](const std::string& itemId) { m_signalDuplicateRequested.emit(itemId); });
        m_contextMenu.signalDuplicateAndEditRequested().connect([this](const std::string& itemId) { m_signalDuplicateAndEditRequested.emit(itemId); });
    }

    ItemShelfView::~ItemShelfView()
    {
        *m_alive = false;
        m_relayoutConnection.disconnect();
    }

    void ItemShelfView::setRows(const std::vector<ItemRow>& rows)
    {
        clear();

        m_rows = rows;
        m_textures.reserve(rows.size());
        for (const auto& row : m_rows)
            m_textures.push_back(row.imagePath.empty() ? Glib::RefPtr<Gdk::Texture>() : loadTexture(row.imagePath, COVER_SIZE));

        rebuildShelves();
        m_emptyState.set_visible(rows.empty());
    }

    void ItemShelfView::setEmptyKind(const EmptyListKind kind)
    {
        showEmptyListKind(m_emptyState, kind);
    }

    sigc::signal<void()>& ItemShelfView::signalAddRequested()
    {
        return m_emptyState.signalAction();
    }

    sigc::signal<void(const std::string&)>& ItemShelfView::signalEditRequested()
    {
        return m_signalEditRequested;
    }

    sigc::signal<void(const std::string&)>& ItemShelfView::signalDeleteRequested()
    {
        return m_signalDeleteRequested;
    }

    sigc::signal<void(const std::string&)>& ItemShelfView::signalDuplicateRequested()
    {
        return m_signalDuplicateRequested;
    }

    sigc::signal<void(const std::string&)>& ItemShelfView::signalDuplicateAndEditRequested()
    {
        return m_signalDuplicateAndEditRequested;
    }

    void ItemShelfView::clear()
    {
        m_relayoutConnection.disconnect();
        m_selected.reset();
        m_tiles.clear();

        while (auto* child = m_shelves.get_first_child())
            m_shelves.remove(*child);

        m_rows.clear();
        m_textures.clear();
    }

    // Lays the covers out on shelves of m_columns covers. Only the arrangement changes, so the decoded covers are reused.
    void ItemShelfView::rebuildShelves()
    {
        m_tiles.clear();

        while (auto* child = m_shelves.get_first_child())
            m_shelves.remove(*child);

        const auto columns = static_cast<std::size_t>(std::max(1, m_columns));
        for (std::size_t first = 0; first < m_rows.size(); first += columns)
        {
            auto* covers = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
            covers->set_halign(Gtk::Align::CENTER);
            covers->set_homogeneous(true);

            for (std::size_t index = first; index < std::min(first + columns, m_rows.size()); ++index)
            {
                auto* tile = createTile(index);
                m_tiles.push_back(tile);
                covers->append(*tile);
            }

            auto* board = Gtk::make_managed<Gtk::Box>();
            board->add_css_class("kpop-shelf-board");
            board->set_size_request(-1, BOARD_HEIGHT);

            auto* shelf = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
            shelf->set_margin_bottom(ROW_SPACING);
            shelf->append(*covers);
            shelf->append(*board);
            m_shelves.append(*shelf);
        }

        if (m_selected && *m_selected < m_tiles.size())
            m_tiles[*m_selected]->add_css_class("kpop-shelf-tile-selected");
    }

    void ItemShelfView::updateColumns()
    {
        const int width = get_width();
        if (width <= 0)
            return;

        const int columns = std::max(1, (width - 2 * SHELF_SIDE_PADDING) / TILE_WIDTH);
        if (columns == m_columns)
            return;

        m_columns = columns;
        scheduleRelayout();
    }

    // Deferred and combined, because the size changes many times while a window is dragged and rebuilding the shelves
    // inside the allocation of the window would restart it.
    void ItemShelfView::scheduleRelayout()
    {
        m_relayoutConnection.disconnect();
        m_relayoutConnection = Glib::signal_timeout().connect([this, alive = m_alive]
        {
            if (*alive)
                rebuildShelves();

            return false;
        }, RELAYOUT_DELAY_MS);
    }

    void ItemShelfView::select(const std::optional<std::size_t> index)
    {
        if (m_selected && *m_selected < m_tiles.size())
            m_tiles[*m_selected]->remove_css_class("kpop-shelf-tile-selected");

        m_selected = index;

        if (m_selected && *m_selected < m_tiles.size())
            m_tiles[*m_selected]->add_css_class("kpop-shelf-tile-selected");
    }

    std::optional<std::size_t> ItemShelfView::tileIndexAt(const double x, const double y)
    {
        for (auto* widget = m_content.pick(x, y); widget != nullptr && widget != &m_content; widget = widget->get_parent())
        {
            if (const auto tile = std::ranges::find(m_tiles, widget); tile != m_tiles.end())
                return static_cast<std::size_t>(tile - m_tiles.begin());
        }

        return std::nullopt;
    }

    bool ItemShelfView::onKeyPressed(const guint keyval)
    {
        if (!m_selected || *m_selected >= m_rows.size())
            return false;

        switch (keyval)
        {
            case GDK_KEY_Delete:
                m_signalDeleteRequested.emit(m_rows[*m_selected].id);
                return true;
            case GDK_KEY_Return:
            case GDK_KEY_KP_Enter:
                m_signalEditRequested.emit(m_rows[*m_selected].id);
                return true;
            default:
                return false;
        }
    }

    Gtk::Widget* ItemShelfView::createTile(const std::size_t index)
    {
        const auto& row = m_rows[index];

        auto* tile = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);

        auto* cover = makeCover(m_textures[index]);
        if (row.status != domain::ItemStatus::Owned)
            cover->add_css_class("kpop-shelf-unowned");

        auto* standing = Gtk::make_managed<Gtk::Overlay>();
        standing->set_child(*cover);
        standing->add_overlay(*makeTitle(row.title));
        standing->set_margin_top(COVER_MARGIN_TOP);
        standing->set_margin_start(COVER_MARGIN_SIDE);
        standing->set_margin_end(COVER_MARGIN_SIDE);
        standing->set_tooltip_text(tooltipOf(row));
        tile->append(*standing);

        // A scrolled window that never scrolls is the one container that does not pass the natural width of its child
        // on. Whatever a cover or a title asks for, every tile stays exactly TILE_WIDTH wide, so the shelves line up.
        auto* frame = Gtk::make_managed<Gtk::ScrolledWindow>();
        frame->set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::NEVER);
        frame->set_min_content_width(TILE_WIDTH);
        frame->set_propagate_natural_width(false);
        frame->set_propagate_natural_height(true);
        frame->set_hexpand(false);
        frame->set_child(*tile);

        return frame;
    }
}
