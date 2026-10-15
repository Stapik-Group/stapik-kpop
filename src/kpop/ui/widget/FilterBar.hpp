#pragma once

#include "kpop/domain/Artist.hpp"
#include "kpop/domain/Enumerations.hpp"
#include "kpop/domain/ItemFilter.hpp"

#include "stapik/ui/widget/SearchEntry.hpp"

#include <gtkmm/box.h>
#include <gtkmm/dropdown.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/popover.h>

#include <sigc++/signal.h>

#include <memory>
#include <vector>

namespace kpop::ui
{
    class FilterBar : public Gtk::Box
    {
    public:
        FilterBar();
        ~FilterBar() override;

        void setArtists(const std::vector<domain::Artist>& artists);
        void refreshLabels();

        [[nodiscard]] domain::ItemFilter filter() const;

        // The main order, and all the orders from the main one to the least important one.
        [[nodiscard]] domain::SortOrder sortOrder() const;
        [[nodiscard]] std::vector<domain::SortOrder> sortOrders() const;

        sigc::signal<void()>& signalChanged();

    private:
        void rebuildStatusModel();
        void rebuildArtistModel();
        void rebuildRegionModel();
        void rebuildSortModel();
        [[nodiscard]] domain::SortOrder selectedSortOrder() const;
        void selectSortOrder(domain::SortOrder order);
        void updateFilterButton();
        void scheduleFilterPopoverRegrab();

        SearchEntry m_searchEntry;

        // The filters and the sort order live in the popover of one button. Parents are declared before their
        // children, so the children are destroyed first.
        Gtk::MenuButton m_filterButton;
        Gtk::Popover m_filterPopover;
        Gtk::Grid m_filterGrid;
        Gtk::Label m_statusLabel;
        Gtk::Label m_artistLabel;
        Gtk::Label m_regionLabel;
        Gtk::Label m_sortLabel;
        Gtk::DropDown m_statusDropDown;
        Gtk::DropDown m_artistDropDown;
        Gtk::DropDown m_regionDropDown;
        Gtk::DropDown m_sortDropDown;
        std::vector<domain::Artist> m_artists;

        // Every order that was chosen stays behind the next one, so choosing the release date and then the artist
        // groups the items by artist and keeps each artist's items in chronological order.
        std::vector<domain::SortOrder> m_sortLevels{ domain::SortOrder::AddedNewest };
        bool m_updatingModels = false;
        bool m_regrabPending = false;
        std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
        sigc::signal<void()> m_signalChanged;
    };
}
