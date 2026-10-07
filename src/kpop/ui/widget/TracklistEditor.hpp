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
    // Edits a tracklist as a table (No., Title, Writer(s), Length) with automatic numbering.
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

        void rebuild(const domain::Tracklist& tracklist, std::optional<std::size_t> focusedRow);
        void scheduleRebuild(domain::Tracklist tracklist, std::optional<std::size_t> focusedRow);
        void addHeader();
        void onAddClicked();
        void onRemoveRequested(std::size_t index);
        void onMoveUpRequested(std::size_t index);
        void onMoveDownRequested(std::size_t index);
        void refreshTotal();
        [[nodiscard]] domain::Tracklist allRows() const;

        Gtk::Grid m_grid;
        Gtk::Button m_addButton;
        Gtk::Label m_totalLabel;
        std::vector<std::unique_ptr<Row>> m_rows;

        domain::Tracklist m_pendingTracklist;
        std::optional<std::size_t> m_pendingFocusedRow;
        sigc::connection m_pendingRebuild;

        sigc::signal<void()> m_signalChanged;
    };
}
