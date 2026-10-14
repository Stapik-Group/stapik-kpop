#pragma once

#include "kpop/domain/Tracklist.hpp"

#include <gtkmm/adjustment.h>
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

        // Empty rows between and after the filled ones are kept: they hold the place of tracks that are not
        // filled in yet, and dropping them would shift the numbers of the tracks after them. A list in which
        // nothing is filled in at all is an empty tracklist.
        [[nodiscard]] domain::Tracklist value() const;
        [[nodiscard]] bool isValid() const;

        // The vertical adjustment of the scrolled window that holds the editor. Every change rebuilds the rows,
        // and with it the scroll position is put back where it was instead of jumping to the top.
        void setScrollAdjustment(const Glib::RefPtr<Gtk::Adjustment>& adjustment);

        sigc::signal<void()>& signalChanged();

    private:
        class Row;

        enum class FocusTarget
        {
            Title,
            MoveUp,
            MoveDown
        };

        struct FocusRequest
        {
            std::size_t row = 0;
            FocusTarget target = FocusTarget::Title;
        };

        struct DiscHeader
        {
            int disc = 1;
            Gtk::Label* totalLabel = nullptr;
        };

        void rebuild(const domain::Tracklist& tracklist, std::optional<FocusRequest> focus);
        void scheduleRebuild(domain::Tracklist tracklist, std::optional<FocusRequest> focus);
        void settleView(double scrollValue, std::optional<FocusRequest> focus);
        void focusRow(const std::optional<FocusRequest>& focus) const;
        void addHeader();
        void addDiscHeader(int disc, int gridRow);
        void onAddClicked();
        void onAddDiscClicked();
        void onAddTrackToDisc(int disc);
        void onRemoveDisc(int disc);
        void onRemoveRequested(std::size_t index);
        void onMoveUpRequested(std::size_t index);
        void onMoveDownRequested(std::size_t index);
        void onActivateRequested(std::size_t index);
        void refreshTotal();
        [[nodiscard]] domain::Tracklist allRows() const;

        Gtk::Grid m_grid;
        Gtk::Button m_addButton;
        Gtk::Button m_addDiscButton;
        Gtk::Label m_totalLabel;
        std::vector<std::unique_ptr<Row>> m_rows;
        std::vector<DiscHeader> m_discHeaders;

        domain::Tracklist m_pendingTracklist;
        std::optional<FocusRequest> m_pendingFocus;
        sigc::connection m_pendingRebuild;

        Glib::RefPtr<Gtk::Adjustment> m_scrollAdjustment;
        unsigned int m_settleTick = 0;
        int m_settleFrames = 0;

        sigc::signal<void()> m_signalChanged;
    };
}
