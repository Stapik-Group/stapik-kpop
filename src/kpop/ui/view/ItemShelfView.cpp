#include "ItemShelfView.hpp"

#include "kpop/ui/Translate.hpp"
#include "kpop/ui/widget/ImageTexture.hpp"

#include <gdk/gdk.h>
#include <gtkmm/eventcontrollerkey.h>
#include <gtkmm/gestureclick.h>
#include <gtkmm/overlay.h>
#include <gtkmm/picture.h>

namespace kpop::ui
{
    namespace
    {
        constexpr int PLACEHOLDER_MARGIN = 24;
        constexpr int COVER_SIZE = 110;
        constexpr int COVER_MARGIN_TOP = 8;
        constexpr int COVER_MARGIN_SIDE = 8;
        constexpr int BOARD_HEIGHT = 14;
        constexpr int ROW_SPACING = 12;
        constexpr int TILE_WIDTH = COVER_SIZE + 2 * COVER_MARGIN_SIDE;
        constexpr int TITLE_LINES = 2;
        constexpr int MAX_TILES_PER_LINE = 60;

        Gtk::Widget* makeCover(const ItemRow& row)
        {
            if (!row.imagePath.empty())
            {
                if (const Glib::RefPtr<Gdk::Texture> texture = loadTexture(row.imagePath, COVER_SIZE))
                {
                    auto* cover = Gtk::make_managed<Gtk::Picture>();
                    cover->set_size_request(COVER_SIZE, COVER_SIZE);
                    cover->set_content_fit(Gtk::ContentFit::CONTAIN);
                    cover->set_paintable(texture);
                    cover->add_css_class("kpop-shelf-cover");
                    return cover;
                }
            }

            auto* empty = Gtk::make_managed<Gtk::Label>("♪");
            empty->set_size_request(COVER_SIZE, COVER_SIZE);
            empty->add_css_class("kpop-shelf-cover");
            empty->add_css_class("kpop-shelf-cover-empty");
            return empty;
        }

        Gtk::Label* makeTitle(const std::string& text)
        {
            auto* title = Gtk::make_managed<Gtk::Label>(text);
            title->add_css_class("kpop-shelf-title");
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

            for (const auto* part : { &row.subtitle, &row.statusText })
            {
                if (!part->empty())
                    text += "\n" + *part;
            }

            return text;
        }
    }

    ItemShelfView::ItemShelfView() :
        m_content(Gtk::Orientation::VERTICAL),
        m_contextMenu(m_flowBox)
    {
        set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
        set_vexpand(true);
        set_child(m_content);
        add_css_class("kpop-shelf-scroller");

        m_placeholderLabel.set_margin(PLACEHOLDER_MARGIN);
        m_placeholderLabel.set_visible(false);

        m_content.add_css_class("kpop-shelf");

        m_flowBox.set_halign(Gtk::Align::START);
        m_flowBox.set_valign(Gtk::Align::START);
        m_flowBox.set_homogeneous(true);
        m_flowBox.set_selection_mode(Gtk::SelectionMode::SINGLE);
        m_flowBox.set_activate_on_single_click(false);
        m_flowBox.set_max_children_per_line(MAX_TILES_PER_LINE);
        m_flowBox.set_row_spacing(ROW_SPACING);
        m_flowBox.set_column_spacing(0);

        m_content.append(m_placeholderLabel);
        m_content.append(m_flowBox);

        refreshPlaceholder();

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

    void ItemShelfView::setRows(const std::vector<ItemRow>& rows)
    {
        clear();

        for (const auto& row : rows)
            appendRow(row);

        m_placeholderLabel.set_visible(rows.empty());
    }

    void ItemShelfView::refreshPlaceholder()
    {
        m_placeholderLabel.set_text(translate("kpop.list.empty"));
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
        while (auto* child = m_flowBox.get_child_at_index(0))
            m_flowBox.remove(*child);

        m_rowIds.clear();
    }

    void ItemShelfView::appendRow(const ItemRow& row)
    {
        auto* tile = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
        tile->set_size_request(TILE_WIDTH, -1);

        auto* cover = makeCover(row);
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

        auto* board = Gtk::make_managed<Gtk::Box>();
        board->set_size_request(-1, BOARD_HEIGHT);
        tile->append(*board);

        m_flowBox.append(*tile);
        m_rowIds.push_back(row.id);
    }
}
