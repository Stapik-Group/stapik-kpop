#include "KindSidebar.hpp"

#include "kpop/ui/Translate.hpp"

#include "stapik/domain/Category.hpp"

#include <gtkmm/box.h>

namespace kpop::ui
{
    namespace
    {
        constexpr int ROW_SPACING = 8;
        constexpr int ROW_MARGIN = 6;
        constexpr int SWATCH_WIDTH = 8;
        constexpr int SIDEBAR_MIN_WIDTH = 190;
    }

    KindSidebar::KindSidebar()
    {
        set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
        set_min_content_width(SIDEBAR_MIN_WIDTH);
        set_child(m_listBox);
        add_css_class("kpop-sidebar");

        m_listBox.set_selection_mode(Gtk::SelectionMode::SINGLE);

        appendRow(std::nullopt, 0);
        for (const auto&[value, id] : domain::ITEM_KINDS.entries())
            appendRow(value, ALL_ROW_COUNT + static_cast<std::size_t>(value));

        m_listBox.select_row(*m_listBox.get_row_at_index(0));

        m_listBox.signal_row_selected().connect([this](const Gtk::ListBoxRow* row)
        {
            if (row == nullptr || m_updatingSelection)
                return;

            m_signalKindSelected.emit(selectedKind());
        });

        refreshLabels();
    }

    void KindSidebar::setStatistics(const domain::CollectionStatistics& statistics) const
    {
        m_rows[0].count->set_text(std::to_string(statistics.totalEntries()));

        for (const auto&[value, id] : domain::ITEM_KINDS.entries())
        {
            const auto rowIndex = ALL_ROW_COUNT + static_cast<std::size_t>(value);
            m_rows[rowIndex].count->set_text(std::to_string(statistics.entriesOfKind(value)));
        }
    }

    void KindSidebar::refreshLabels() const
    {
        for (std::size_t rowIndex = 0; rowIndex < ROW_COUNT; ++rowIndex)
            m_rows[rowIndex].name->set_text(nameOfRow(rowIndex));
    }

    std::optional<domain::ItemKind> KindSidebar::selectedKind() const
    {
        const auto* row = m_listBox.get_selected_row();
        if (row == nullptr || row->get_index() <= 0)
            return std::nullopt;

        return static_cast<domain::ItemKind>(static_cast<std::size_t>(row->get_index()) - ALL_ROW_COUNT);
    }

    void KindSidebar::selectKind(const std::optional<domain::ItemKind> kind)
    {
        const auto rowIndex = kind ? ALL_ROW_COUNT + static_cast<std::size_t>(*kind) : 0;

        m_updatingSelection = true;
        m_listBox.select_row(*m_listBox.get_row_at_index(static_cast<int>(rowIndex)));
        m_updatingSelection = false;
    }

    sigc::signal<void(std::optional<domain::ItemKind>)>& KindSidebar::signalKindSelected()
    {
        return m_signalKindSelected;
    }

    void KindSidebar::appendRow(const std::optional<domain::ItemKind> kind, const std::size_t rowIndex)
    {
        auto* box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, ROW_SPACING);
        box->set_margin(ROW_MARGIN);

        auto* swatch = Gtk::make_managed<Gtk::Label>();
        swatch->set_size_request(SWATCH_WIDTH, -1);
        swatch->add_css_class("kpop-kind-swatch");
        swatch->add_css_class(kind ? stapik::domain::categoryColorCssClass(domain::colorOf(*kind)) : "stapik-category-default");
        box->append(*swatch);

        auto* name = Gtk::make_managed<Gtk::Label>();
        name->set_hexpand(true);
        name->set_halign(Gtk::Align::START);
        name->set_xalign(0.0F);
        name->add_css_class("kpop-sidebar-name");
        box->append(*name);

        auto* count = Gtk::make_managed<Gtk::Label>("0");
        count->add_css_class("kpop-sidebar-count");
        box->append(*count);

        m_listBox.append(*box);
        m_rows[rowIndex] = RowLabels{ .name = name, .count = count };
    }

    std::string KindSidebar::nameOfRow(const std::size_t rowIndex)
    {
        if (rowIndex == 0)
            return translate("kpop.kind.all");

        return translate(domain::ITEM_KINDS.nameKey(static_cast<domain::ItemKind>(rowIndex - ALL_ROW_COUNT)));
    }
}
