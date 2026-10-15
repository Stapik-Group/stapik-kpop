#include "FilterBar.hpp"

#include "kpop/domain/ItemSort.hpp"

#include "kpop/ui/Translate.hpp"

#include <glibmm/main.h>
#include <glibmm/ustring.h>
#include <gtkmm/stringlist.h>

namespace kpop::ui
{
    namespace
    {
        constexpr int SPACING = 8;
        constexpr int POPOVER_MARGIN = 12;
        constexpr int POPOVER_SPACING = 10;
        constexpr int DROPDOWN_WIDTH = 260;
        constexpr std::size_t ALL_ENTRY_COUNT = 1;

        void addFilterRow(Gtk::Grid& grid, const int row, Gtk::Label& label, Gtk::DropDown& dropDown)
        {
            label.set_xalign(0.0F);
            label.set_valign(Gtk::Align::CENTER);
            dropDown.set_hexpand(true);
            dropDown.set_size_request(DROPDOWN_WIDTH, -1);

            grid.attach(label, 0, row);
            grid.attach(dropDown, 1, row);
        }

        // The pop-up list of a drop-down is a popover that is a child of the drop-down.
        Gtk::Popover* findPopup(Gtk::Widget& widget)
        {
            for (auto* child = widget.get_first_child(); child != nullptr; child = child->get_next_sibling())
            {
                if (auto* popup = dynamic_cast<Gtk::Popover*>(child))
                    return popup;
            }

            return nullptr;
        }
    }

    FilterBar::FilterBar() :
        Box(Gtk::Orientation::HORIZONTAL, SPACING)
    {
        m_searchEntry.set_hexpand(true);

        m_filterGrid.set_margin(POPOVER_MARGIN);
        m_filterGrid.set_row_spacing(POPOVER_SPACING);
        m_filterGrid.set_column_spacing(POPOVER_SPACING);
        addFilterRow(m_filterGrid, 0, m_statusLabel, m_statusDropDown);
        addFilterRow(m_filterGrid, 1, m_artistLabel, m_artistDropDown);
        addFilterRow(m_filterGrid, 2, m_sortLabel, m_sortDropDown);
        m_filterPopover.set_child(m_filterGrid);
        m_filterButton.set_popover(m_filterPopover);

        append(m_searchEntry);
        append(m_filterButton);

        rebuildStatusModel();
        rebuildArtistModel();
        rebuildSortModel();
        refreshLabels();

        m_searchEntry.signalQueryChanged().connect([this] { m_signalChanged.emit(); });

        const auto onSelectionChanged = [this]
        {
            updateFilterButton();

            if (!m_updatingModels)
            {
                scheduleFilterPopoverRegrab();
                m_signalChanged.emit();
            }
        };
        m_statusDropDown.property_selected().signal_changed().connect(onSelectionChanged);
        m_artistDropDown.property_selected().signal_changed().connect(onSelectionChanged);
        m_sortDropDown.property_selected().signal_changed().connect([this, onSelectionChanged]
        {
            if (!m_updatingModels)
                m_sortLevels = domain::withSortLevel(m_sortLevels, selectedSortOrder());

            onSelectionChanged();
        });

        // Also when a list is closed without choosing anything.
        for (auto* dropDown : { &m_statusDropDown, &m_artistDropDown, &m_sortDropDown })
        {
            if (auto* popup = findPopup(*dropDown))
                popup->signal_closed().connect([this] { scheduleFilterPopoverRegrab(); });
        }
    }

    FilterBar::~FilterBar()
    {
        *m_alive = false;
    }

    void FilterBar::setArtists(const std::vector<domain::Artist>& artists)
    {
        const auto previous = filter();
        m_artists = artists;

        rebuildArtistModel();

        m_updatingModels = true;
        if (previous.artistId)
        {
            for (std::size_t index = 0; index < m_artists.size(); ++index)
            {
                if (m_artists[index].id == *previous.artistId)
                    m_artistDropDown.set_selected(static_cast<guint>(index + ALL_ENTRY_COUNT));
            }
        }
        m_updatingModels = false;

        updateFilterButton();
    }

    void FilterBar::refreshLabels()
    {
        const auto previous = filter();
        const auto previousSortOrder = sortOrder();

        m_searchEntry.set_placeholder_text(translate("kpop.filter.search"));
        m_statusLabel.set_text(translate("kpop.filter.status"));
        m_artistLabel.set_text(translate("kpop.filter.artist"));
        m_sortLabel.set_text(translate("kpop.filter.sort"));

        rebuildStatusModel();
        rebuildArtistModel();
        rebuildSortModel();

        m_updatingModels = true;
        if (previous.status)
        {
            for (const auto&[value, id] : domain::ITEM_STATUSES.entries())
            {
                if (value == *previous.status)
                    m_statusDropDown.set_selected(static_cast<guint>(static_cast<std::size_t>(value) + ALL_ENTRY_COUNT));
            }
        }

        if (previous.artistId)
        {
            for (std::size_t index = 0; index < m_artists.size(); ++index)
            {
                if (m_artists[index].id == *previous.artistId)
                    m_artistDropDown.set_selected(static_cast<guint>(index + ALL_ENTRY_COUNT));
            }
        }

        selectSortOrder(previousSortOrder);
        m_updatingModels = false;

        updateFilterButton();
    }

    domain::ItemFilter FilterBar::filter() const
    {
        domain::ItemFilter result;
        result.text = m_searchEntry.query();

        if (const auto statusPosition = static_cast<std::size_t>(m_statusDropDown.get_selected());
            statusPosition >= ALL_ENTRY_COUNT && statusPosition - ALL_ENTRY_COUNT < domain::ITEM_STATUSES.entries().size())
        {
            result.status = domain::ITEM_STATUSES.entries()[statusPosition - ALL_ENTRY_COUNT].value;
        }

        if (const auto artistPosition = static_cast<std::size_t>(m_artistDropDown.get_selected());
            artistPosition >= ALL_ENTRY_COUNT && artistPosition - ALL_ENTRY_COUNT < m_artists.size())
        {
            result.artistId = m_artists[artistPosition - ALL_ENTRY_COUNT].id;
        }

        return result;
    }

    domain::SortOrder FilterBar::sortOrder() const
    {
        return m_sortLevels.front();
    }

    std::vector<domain::SortOrder> FilterBar::sortOrders() const
    {
        return m_sortLevels;
    }

    domain::SortOrder FilterBar::selectedSortOrder() const
    {
        const auto entries = domain::SORT_ORDERS.entries();
        const auto position = static_cast<std::size_t>(m_sortDropDown.get_selected());

        return position < entries.size() ? entries[position].value : domain::SortOrder::AddedNewest;
    }

    sigc::signal<void()>& FilterBar::signalChanged()
    {
        return m_signalChanged;
    }

    // Shows how many of the settings in the popover differ from the default, so a filter that is on is never hidden.
    void FilterBar::updateFilterButton()
    {
        const auto current = filter();

        const bool defaultSort = m_sortLevels.size() == 1 && m_sortLevels.front() == domain::SortOrder::AddedNewest;
        const int activeCount = (current.status ? 1 : 0) + (current.artistId ? 1 : 0) + (defaultSort ? 0 : 1);

        // The list shows only the main order, so the whole chain is spelled out in the tooltips.
        std::string sortChain;
        if (m_sortLevels.size() > 1)
        {
            for (const auto order : m_sortLevels)
            {
                if (!sortChain.empty())
                    sortChain += " → ";

                sortChain += translate(domain::SORT_ORDERS.nameKey(order));
            }
        }
        m_sortDropDown.set_tooltip_text(sortChain);
        m_filterButton.set_tooltip_text(sortChain);

        m_filterButton.set_label(activeCount > 0
            ? translate("kpop.filter.buttonActive", { { "count", std::to_string(activeCount) } })
            : translate("kpop.filter.button"));
    }

    // GTK 4 bug (gtk issues 4369, 4529 and 5414): once a drop-down inside a popover has used its own pop-up list,
    // the popover loses its grab and nothing closes it any more, not a click outside, not Esc, not the button.
    // Showing the popover again, after GTK is done with the list, gives it a fresh grab.
    void FilterBar::scheduleFilterPopoverRegrab()
    {
        if (m_regrabPending)
            return;

        m_regrabPending = true;

        Glib::signal_idle().connect_once([this, alive = m_alive]
        {
            if (!*alive)
                return;

            m_regrabPending = false;

            if (!m_filterPopover.get_visible())
                return;

            m_filterPopover.popdown();
            m_filterPopover.popup();
        });
    }

    void FilterBar::rebuildStatusModel()
    {
        std::vector<Glib::ustring> labels{ translate("kpop.filter.allStatuses") };
        for (const auto&[value, id] : domain::ITEM_STATUSES.entries())
            labels.emplace_back(translate(domain::ITEM_STATUSES.nameKey(value)));

        m_updatingModels = true;
        m_statusDropDown.set_model(Gtk::StringList::create(labels));
        m_updatingModels = false;
    }

    void FilterBar::rebuildSortModel()
    {
        std::vector<Glib::ustring> labels;
        for (const auto&[value, id] : domain::SORT_ORDERS.entries())
            labels.emplace_back(translate(domain::SORT_ORDERS.nameKey(value)));

        m_updatingModels = true;
        m_sortDropDown.set_model(Gtk::StringList::create(labels));
        m_updatingModels = false;
    }

    void FilterBar::selectSortOrder(const domain::SortOrder order)
    {
        const auto entries = domain::SORT_ORDERS.entries();
        for (std::size_t index = 0; index < entries.size(); ++index)
        {
            if (entries[index].value == order)
                m_sortDropDown.set_selected(static_cast<guint>(index));
        }
    }

    void FilterBar::rebuildArtistModel()
    {
        std::vector<Glib::ustring> labels{ translate("kpop.filter.allArtists") };
        for (const auto& artist : m_artists)
            labels.emplace_back(artist.name);

        m_updatingModels = true;
        m_artistDropDown.set_model(Gtk::StringList::create(labels));
        m_updatingModels = false;
    }
}
