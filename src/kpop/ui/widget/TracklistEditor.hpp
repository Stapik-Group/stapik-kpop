#pragma once

#include "kpop/domain/Tracklist.hpp"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>

#include <sigc++/connection.h>
#include <sigc++/signal.h>

#include <memory>
#include <optional>
#include <vector>

namespace kpop::ui
{
    class TracklistEditor : public Gtk::Box
    {
    public:
        TracklistEditor();
        ~TracklistEditor() override;

        void setValue(const domain::Tracklist& tracklist);

        // Rows that are completely empty are left out.
        [[nodiscard]] domain::Tracklist value() const;
        [[nodiscard]] bool isValid() const;

        sigc::signal<void()>& signalChanged();

    private:
        class Row;

        struct DiscHeader
        {
            int disc = 1;
            Gtk::Label* totalLabel = nullptr;
        };

        void rebuild(const domain::Tracklist& tracklist, std::optional<std::size_t> focusedRow);
        void scheduleRebuild(domain::Tracklist tracklist, std::optional<std::size_t> focusedRow);
        void addHeader();
        void addDiscHeader(int disc, int gridRow);
        void onAddClicked();
        void onAddDiscClicked();
        void onAddTrackToDisc(int disc);
        void onRemoveDisc(int disc);
        void onRemoveRequested(std::size_t index);
        void onMoveUpRequested(std::size_t index);
        void onMoveDownRequested(std::size_t index);
        void refreshTotal();
        [[nodiscard]] domain::Tracklist allRows() const;

        Gtk::Grid m_grid;
        Gtk::Button m_addButton;
        Gtk::Button m_addDiscButton;
        Gtk::Label m_totalLabel;
        std::vector<std::unique_ptr<Row>> m_rows;
        std::vector<DiscHeader> m_discHeaders;

        domain::Tracklist m_pendingTracklist;
        std::optional<std::size_t> m_pendingFocusedRow;
        sigc::connection m_pendingRebuild;

        sigc::signal<void()> m_signalChanged;
    };
}
