#pragma once

#include "kpop/domain/Artist.hpp"
#include "kpop/domain/Enumerations.hpp"
#include "kpop/domain/ItemFilter.hpp"

#include <gtkmm/box.h>
#include <gtkmm/dropdown.h>
#include <gtkmm/searchentry.h>

#include <sigc++/signal.h>

#include <vector>

namespace kpop::ui
{
    class FilterBar : public Gtk::Box
    {
    public:
        FilterBar();

        void setArtists(const std::vector<domain::Artist>& artists);
        void refreshLabels();

        [[nodiscard]] domain::ItemFilter filter() const;
        [[nodiscard]] domain::SortOrder sortOrder() const;

        sigc::signal<void()>& signalChanged();

    private:
        void rebuildStatusModel();
        void rebuildArtistModel();
        void rebuildSortModel();
        void selectSortOrder(domain::SortOrder order);

        Gtk::SearchEntry m_searchEntry;
        Gtk::DropDown m_statusDropDown;
        Gtk::DropDown m_artistDropDown;
        Gtk::DropDown m_sortDropDown;
        std::vector<domain::Artist> m_artists;
        bool m_updatingModels = false;
        sigc::signal<void()> m_signalChanged;
    };
}
