#include "FilterBar.hpp"

#include "kpop/ui/Translate.hpp"

#include <glibmm/ustring.h>
#include <gtkmm/stringlist.h>

namespace kpop::ui
{
    namespace
    {
        constexpr int SPACING = 8;
        constexpr std::size_t ALL_ENTRY_COUNT = 1;
    }

    FilterBar::FilterBar() :
        Box(Gtk::Orientation::HORIZONTAL, SPACING)
    {
        m_searchEntry.set_hexpand(true);

        append(m_searchEntry);
        append(m_statusDropDown);
        append(m_artistDropDown);
        append(m_sortDropDown);

        rebuildStatusModel();
        rebuildArtistModel();
        rebuildSortModel();
        refreshLabels();

        m_searchEntry.signal_search_changed().connect([this] { m_signalChanged.emit(); });

        const auto onSelectionChanged = [this]
        {
            if (!m_updatingModels)
                m_signalChanged.emit();
        };
        m_statusDropDown.property_selected().signal_changed().connect(onSelectionChanged);
        m_artistDropDown.property_selected().signal_changed().connect(onSelectionChanged);
        m_sortDropDown.property_selected().signal_changed().connect(onSelectionChanged);
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
    }

    void FilterBar::refreshLabels()
    {
        const auto previous = filter();
        const auto previousSortOrder = sortOrder();

        m_searchEntry.set_placeholder_text(translate("kpop.filter.search"));
        m_sortDropDown.set_tooltip_text(translate("kpop.filter.sortTooltip"));

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
    }

    domain::ItemFilter FilterBar::filter() const
    {
        domain::ItemFilter result;
        result.text = trimmedText(m_searchEntry);

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
        const auto entries = domain::SORT_ORDERS.entries();
        const auto position = static_cast<std::size_t>(m_sortDropDown.get_selected());

        return position < entries.size() ? entries[position].value : domain::SortOrder::AddedNewest;
    }

    sigc::signal<void()>& FilterBar::signalChanged()
    {
        return m_signalChanged;
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
