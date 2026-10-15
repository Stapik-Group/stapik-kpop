#include "ItemGridView.hpp"

#include "kpop/ui/Translate.hpp"
#include "kpop/ui/widget/ImageTexture.hpp"

#include <gdk/gdk.h>
#include <glibmm/main.h>
#include <glibmm/markup.h>
#include <gtkmm/box.h>
#include <gtkmm/eventcontrollerkey.h>
#include <gtkmm/gestureclick.h>
#include <gtkmm/label.h>
#include <gtkmm/picture.h>

#include <algorithm>

namespace kpop::ui
{
    namespace
    {
        constexpr int TILE_MARGIN = 8;
        constexpr int REBUILD_DELAY_MS = 150;
        constexpr int TILE_SPACING = 2;
        constexpr int TEXT_MARGIN = 2;
        constexpr int ROW_SPACING = 4;
        constexpr int COLUMN_SPACING = 4;
        constexpr int VIEW_MARGIN = 8;
        constexpr int MAX_TILES_PER_LINE = 60;

        // Every cover occupies exactly coverSize x coverSize; the picture inside is scaled to fit it
        // and keeps its aspect ratio, so odd shapes are never stretched.
        Gtk::Widget* makeCover(const ItemRow& row, const int coverSize)
        {
            if (!row.imagePath.empty())
            {
                // The texture is never larger than coverSize, so its natural size cannot grow the slot.
                if (const Glib::RefPtr<Gdk::Texture> texture = loadTexture(row.imagePath, coverSize))
                {
                    auto* slot = Gtk::make_managed<Gtk::Box>();
                    slot->set_size_request(coverSize, coverSize);
                    slot->set_halign(Gtk::Align::CENTER);
                    slot->set_valign(Gtk::Align::START);
                    slot->set_hexpand(false);
                    slot->set_vexpand(false);
                    slot->set_overflow(Gtk::Overflow::HIDDEN);
                    slot->add_css_class("kpop-shelf-cover");

                    auto* picture = Gtk::make_managed<Gtk::Picture>();
                    picture->set_can_shrink(true);
                    picture->set_content_fit(Gtk::ContentFit::CONTAIN);
                    picture->set_hexpand(true);
                    picture->set_vexpand(true);
                    picture->set_paintable(texture);
                    slot->append(*picture);
                    return slot;
                }
            }

            auto* empty = Gtk::make_managed<Gtk::Label>("♪");
            empty->set_size_request(coverSize, coverSize);
            empty->set_halign(Gtk::Align::CENTER);
            empty->set_valign(Gtk::Align::START);
            empty->add_css_class("kpop-shelf-cover");
            empty->add_css_class("kpop-shelf-cover-empty");
            return empty;
        }

        // A line that is cut with an ellipsis. The tiny maximum width keeps a long text from widening the tile:
        // the width comes from the cover, the label only takes what it is given.
        Gtk::Label* makeLine()
        {
            auto* line = Gtk::make_managed<Gtk::Label>();
            line->set_halign(Gtk::Align::FILL);
            line->set_xalign(0.5F);
            line->set_justify(Gtk::Justification::CENTER);
            line->set_ellipsize(Pango::EllipsizeMode::END);
            line->set_max_width_chars(1);
            line->set_margin_start(TEXT_MARGIN);
            line->set_margin_end(TEXT_MARGIN);
            return line;
        }

        std::string tooltipOf(const ItemRow& row)
        {
            std::string text = row.title;

            for (const auto* part : { &row.subtitle, &row.statusText })
            {
                if (!part->empty())
                    text += "\n" + *part;
            }

            return text;
        }
    }

    ItemGridView::ItemGridView() :
        m_content(Gtk::Orientation::VERTICAL),
        m_contextMenu(m_flowBox)
    {
        set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
        set_vexpand(true);
        set_child(m_content);
        add_css_class("kpop-grid-scroller");

        showEmptyListKind(m_emptyState, EmptyListKind::NoEntries);
        m_emptyState.set_visible(false);

        m_content.add_css_class("kpop-grid");

        m_flowBox.set_halign(Gtk::Align::START);
        m_flowBox.set_valign(Gtk::Align::START);
        m_flowBox.set_margin(VIEW_MARGIN);
        m_flowBox.set_homogeneous(true);
        m_flowBox.set_selection_mode(Gtk::SelectionMode::SINGLE);
        m_flowBox.set_activate_on_single_click(false);
        m_flowBox.set_max_children_per_line(MAX_TILES_PER_LINE);
        m_flowBox.set_row_spacing(ROW_SPACING);
        m_flowBox.set_column_spacing(COLUMN_SPACING);

        m_content.append(m_emptyState);
        m_content.append(m_flowBox);

        m_flowBox.signal_child_activated().connect([this](const Gtk::FlowBoxChild* child)
        {
            if (child == nullptr)
                return;

            if (const auto index = static_cast<std::size_t>(child->get_index()); index < m_rowIds.size())
                m_signalEditRequested.emit(m_rowIds[index]);
        });

        const auto keyController = Gtk::EventControllerKey::create();
        keyController->signal_key_pressed().connect([this](const guint keyval, guint, Gdk::ModifierType)
        {
            if (keyval != GDK_KEY_Delete)
                return false;

            const auto selected = m_flowBox.get_selected_children();
            if (selected.empty())
                return false;

            if (const auto index = static_cast<std::size_t>(selected.front()->get_index()); index < m_rowIds.size())
                m_signalDeleteRequested.emit(m_rowIds[index]);

            return true;
        }, false);
        m_flowBox.add_controller(keyController);

        m_contextMenu.signalEditRequested().connect([this](const std::string& itemId) { m_signalEditRequested.emit(itemId); });
        m_contextMenu.signalDeleteRequested().connect([this](const std::string& itemId) { m_signalDeleteRequested.emit(itemId); });
        m_contextMenu.signalDuplicateRequested().connect([this](const std::string& itemId) { m_signalDuplicateRequested.emit(itemId); });
        m_contextMenu.signalDuplicateAndEditRequested().connect([this](const std::string& itemId) { m_signalDuplicateAndEditRequested.emit(itemId); });

        const auto rightClick = Gtk::GestureClick::create();
        rightClick->set_button(GDK_BUTTON_SECONDARY);
        rightClick->signal_pressed().connect([this](const int, const double x, const double y)
        {
            auto* child = m_flowBox.get_child_at_pos(static_cast<int>(x), static_cast<int>(y));
            if (child == nullptr)
                return;

            const auto index = static_cast<std::size_t>(child->get_index());
            if (index >= m_rowIds.size())
                return;

            m_flowBox.select_child(*child);
            m_contextMenu.popup(m_rowIds[index], x, y);
        });
        m_flowBox.add_controller(rightClick);
    }

    ItemGridView::~ItemGridView()
    {
        m_rebuildConnection.disconnect();
    }

    void ItemGridView::setRows(const std::vector<ItemRow>& rows)
    {
        m_rows = rows;
        populate();
    }

    void ItemGridView::setCoverSize(const int size)
    {
        const int clamped = std::clamp(size, MIN_COVER_SIZE, MAX_COVER_SIZE);
        if (clamped == m_coverSize)
            return;

        m_coverSize = clamped;

        // While the slider is dragged the tiles are rebuilt only when it pauses, because every rebuild reads the covers again.
        m_rebuildConnection.disconnect();
        m_rebuildConnection = Glib::signal_timeout().connect([this]
        {
            populate();
            return false;
        }, REBUILD_DELAY_MS);
    }

    void ItemGridView::populate()
    {
        clear();

        for (const auto& row : m_rows)
            appendRow(row);

        m_emptyState.set_visible(m_rows.empty());
    }

    void ItemGridView::setEmptyKind(const EmptyListKind kind)
    {
        showEmptyListKind(m_emptyState, kind);
    }

    sigc::signal<void()>& ItemGridView::signalAddRequested()
    {
        return m_emptyState.signalAction();
    }

    sigc::signal<void(const std::string&)>& ItemGridView::signalEditRequested()
    {
        return m_signalEditRequested;
    }

    sigc::signal<void(const std::string&)>& ItemGridView::signalDeleteRequested()
    {
        return m_signalDeleteRequested;
    }

    sigc::signal<void(const std::string&)>& ItemGridView::signalDuplicateRequested()
    {
        return m_signalDuplicateRequested;
    }

    sigc::signal<void(const std::string&)>& ItemGridView::signalDuplicateAndEditRequested()
    {
        return m_signalDuplicateAndEditRequested;
    }

    void ItemGridView::clear()
    {
        while (auto* child = m_flowBox.get_child_at_index(0))
            m_flowBox.remove(*child);

        m_rowIds.clear();
    }

    void ItemGridView::appendRow(const ItemRow& row)
    {
        auto* tile = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, TILE_SPACING);
        tile->set_size_request(m_coverSize + 2 * TILE_MARGIN, -1);
        tile->set_margin(TILE_MARGIN);
        tile->set_tooltip_text(tooltipOf(row));

        auto* cover = makeCover(row, m_coverSize);
        if (row.status != domain::ItemStatus::Owned)
            cover->add_css_class("kpop-shelf-unowned");
        tile->append(*cover);

        auto* title = makeLine();
        title->set_markup("<b>" + Glib::Markup::escape_text(row.title) + "</b>");
        title->add_css_class("kpop-grid-title");
        tile->append(*title);

        auto* subtitle = makeLine();
        subtitle->set_text(row.subtitle);
        subtitle->add_css_class("kpop-grid-subtitle");
        subtitle->add_css_class("dim-label");
        tile->append(*subtitle);

        m_flowBox.append(*tile);
        m_rowIds.push_back(row.id);
    }
}
