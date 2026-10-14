#include "TracklistEditor.hpp"

#include "TrackLengthEntry.hpp"

#include "kpop/ui/Translate.hpp"

#include <gdkmm/frameclock.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/togglebutton.h>

#include <glibmm/main.h>

#include <algorithm>
#include <string>
#include <utility>

namespace kpop::ui
{
    namespace
    {
        constexpr int EDITOR_SPACING = 6;
        constexpr int GRID_COLUMN_SPACING = 6;
        constexpr int GRID_ROW_SPACING = 4;
        constexpr int TEXT_ENTRY_WIDTH_CHARS = 10;
        constexpr int HEADER_GRID_ROW = 0;
        constexpr int FIRST_TRACK_GRID_ROW = 1;
        constexpr int DISC_HEADER_MARGIN_TOP = 8;

        constexpr int NUMBER_COLUMN = 0;
        constexpr int TITLE_COLUMN = 1;
        constexpr int WRITERS_COLUMN = 2;
        constexpr int LENGTH_COLUMN = 3;
        constexpr int TITLE_TRACK_COLUMN = 4;
        constexpr int BONUS_COLUMN = 5;
        constexpr int MOVE_UP_COLUMN = 6;
        constexpr int MOVE_DOWN_COLUMN = 7;
        constexpr int REMOVE_COLUMN = 8;
        constexpr int COLUMN_COUNT = 9;

        // After a rebuild the first frame tick comes before the new rows are laid out, the second one after it.
        constexpr int SETTLE_FRAMES = 2;

        constexpr auto TITLE_TRACK_SYMBOL = "★";

        Gtk::Label* createHeaderLabel(const std::string& text, const Gtk::Align alignment)
        {
            auto* label = Gtk::make_managed<Gtk::Label>(text);
            label->set_halign(alignment);
            label->add_css_class("dim-label");
            return label;
        }

        Gtk::Button* createFlatButton(const char* const symbol, const std::string& tooltip)
        {
            auto* button = Gtk::make_managed<Gtk::Button>(symbol);
            button->add_css_class("flat");
            button->set_tooltip_text(tooltip);
            return button;
        }
    }

    class TracklistEditor::Row final
    {
    public:
        // "number" is the position on the disc, "index" the position in the whole tracklist.
        Row(Gtk::Grid& grid, const int gridRow, const int disc, const std::size_t number, const std::size_t index, const std::size_t count) :
            m_disc(disc),
            m_numberLabel(Gtk::make_managed<Gtk::Label>(std::to_string(number))),
            m_titleEntry(Gtk::make_managed<Gtk::Entry>()),
            m_writersEntry(Gtk::make_managed<Gtk::Entry>()),
            m_lengthEntry(Gtk::make_managed<TrackLengthEntry>()),
            m_titleTrackToggle(Gtk::make_managed<Gtk::ToggleButton>(TITLE_TRACK_SYMBOL)),
            m_bonusCheck(Gtk::make_managed<Gtk::CheckButton>()),
            m_moveUpButton(createFlatButton("↑", translate("kpop.tracklist.moveUpTooltip"))),
            m_moveDownButton(createFlatButton("↓", translate("kpop.tracklist.moveDownTooltip"))),
            m_removeButton(createFlatButton("✕", translate("kpop.tracklist.removeTooltip")))
        {
            m_numberLabel->set_xalign(1.0F);
            m_titleEntry->set_hexpand(true);
            m_titleEntry->set_width_chars(TEXT_ENTRY_WIDTH_CHARS);
            m_writersEntry->set_hexpand(true);
            m_writersEntry->set_width_chars(TEXT_ENTRY_WIDTH_CHARS);
            m_titleTrackToggle->add_css_class("flat");
            m_titleTrackToggle->set_tooltip_text(translate("kpop.tracklist.titleTrackTooltip"));
            m_bonusCheck->set_halign(Gtk::Align::CENTER);
            m_bonusCheck->set_tooltip_text(translate("kpop.tracklist.bonusTooltip"));
            m_moveUpButton->set_sensitive(index > 0);
            m_moveDownButton->set_sensitive(index + 1 < count);

            grid.attach(*m_numberLabel, NUMBER_COLUMN, gridRow);
            grid.attach(*m_titleEntry, TITLE_COLUMN, gridRow);
            grid.attach(*m_writersEntry, WRITERS_COLUMN, gridRow);
            grid.attach(*m_lengthEntry, LENGTH_COLUMN, gridRow);
            grid.attach(*m_titleTrackToggle, TITLE_TRACK_COLUMN, gridRow);
            grid.attach(*m_bonusCheck, BONUS_COLUMN, gridRow);
            grid.attach(*m_moveUpButton, MOVE_UP_COLUMN, gridRow);
            grid.attach(*m_moveDownButton, MOVE_DOWN_COLUMN, gridRow);
            grid.attach(*m_removeButton, REMOVE_COLUMN, gridRow);

            const auto onEdited = [this]
            {
                refreshValidityStyle();
                m_signalChanged.emit();
            };
            m_titleEntry->signal_changed().connect(onEdited);
            m_writersEntry->signal_changed().connect(onEdited);
            m_lengthEntry->signal_changed().connect(onEdited);
            m_titleTrackToggle->signal_toggled().connect(onEdited);
            m_bonusCheck->signal_toggled().connect(onEdited);

            m_titleEntry->signal_activate().connect([this] { m_signalActivate.emit(); });

            m_moveUpButton->signal_clicked().connect([this] { m_signalMoveUp.emit(); });
            m_moveDownButton->signal_clicked().connect([this] { m_signalMoveDown.emit(); });
            m_removeButton->signal_clicked().connect([this] { m_signalRemove.emit(); });
        }

        void setValue(const domain::Track& track) const
        {
            m_titleEntry->set_text(track.title);
            m_writersEntry->set_text(track.writers);
            m_lengthEntry->setValue(track.length);
            m_titleTrackToggle->set_active(track.titleTrack);
            m_bonusCheck->set_active(track.bonus);
        }

        [[nodiscard]] domain::Track value() const
        {
            domain::Track track;
            track.title = trimmedText(*m_titleEntry);
            track.writers = trimmedText(*m_writersEntry);
            track.length = m_lengthEntry->value();
            track.titleTrack = m_titleTrackToggle->get_active();
            track.disc = m_disc;
            track.bonus = m_bonusCheck->get_active();
            return track;
        }

        [[nodiscard]] bool isBlank() const
        {
            return trimmedText(*m_titleEntry).empty()
                && trimmedText(*m_writersEntry).empty()
                && m_lengthEntry->isEmpty()
                && !m_titleTrackToggle->get_active()
                && !m_bonusCheck->get_active();
        }

        // A row is either completely empty (a place that is filled in later) or has a title and a readable length.
        [[nodiscard]] bool isValid() const
        {
            return isBlank() || (!trimmedText(*m_titleEntry).empty() && m_lengthEntry->isValid());
        }

        // The arrow that was asked for is not available at the ends of the list, then the other one or the title is used.
        void focus(const FocusTarget target) const
        {
            if (target != FocusTarget::Title)
            {
                Gtk::Button* const first = target == FocusTarget::MoveUp ? m_moveUpButton : m_moveDownButton;
                Gtk::Button* const second = target == FocusTarget::MoveUp ? m_moveDownButton : m_moveUpButton;

                for (auto* button : { first, second })
                {
                    if (button->get_sensitive())
                    {
                        button->grab_focus();
                        return;
                    }
                }
            }

            m_titleEntry->grab_focus();
        }

        sigc::signal<void()>& signalChanged() { return m_signalChanged; }
        sigc::signal<void()>& signalActivate() { return m_signalActivate; }
        sigc::signal<void()>& signalMoveUp() { return m_signalMoveUp; }
        sigc::signal<void()>& signalMoveDown() { return m_signalMoveDown; }
        sigc::signal<void()>& signalRemove() { return m_signalRemove; }

    private:
        void refreshValidityStyle() const
        {
            if (!isBlank() && trimmedText(*m_titleEntry).empty())
                m_titleEntry->add_css_class("kpop-invalid");
            else
                m_titleEntry->remove_css_class("kpop-invalid");
        }

        int m_disc;
        Gtk::Label* m_numberLabel;
        Gtk::Entry* m_titleEntry;
        Gtk::Entry* m_writersEntry;
        TrackLengthEntry* m_lengthEntry;
        Gtk::ToggleButton* m_titleTrackToggle;
        Gtk::CheckButton* m_bonusCheck;
        Gtk::Button* m_moveUpButton;
        Gtk::Button* m_moveDownButton;
        Gtk::Button* m_removeButton;

        sigc::signal<void()> m_signalChanged;
        sigc::signal<void()> m_signalActivate;
        sigc::signal<void()> m_signalMoveUp;
        sigc::signal<void()> m_signalMoveDown;
        sigc::signal<void()> m_signalRemove;
    };

    TracklistEditor::TracklistEditor() :
        Box(Gtk::Orientation::VERTICAL, EDITOR_SPACING),
        m_addButton(translate("kpop.tracklist.add")),
        m_addDiscButton(translate("kpop.tracklist.addDisc"))
    {
        add_css_class("kpop-tracklist");

        m_grid.set_column_spacing(GRID_COLUMN_SPACING);
        m_grid.set_row_spacing(GRID_ROW_SPACING);

        m_addButton.set_halign(Gtk::Align::START);
        m_addButton.signal_clicked().connect([this] { onAddClicked(); });

        m_addDiscButton.set_halign(Gtk::Align::START);
        m_addDiscButton.signal_clicked().connect([this] { onAddDiscClicked(); });

        m_totalLabel.set_hexpand(true);
        m_totalLabel.set_halign(Gtk::Align::END);

        auto* footer = Gtk::make_managed<Box>(Gtk::Orientation::HORIZONTAL, EDITOR_SPACING);
        footer->append(m_addButton);
        footer->append(m_addDiscButton);
        footer->append(m_totalLabel);

        append(m_grid);
        append(*footer);

        rebuild({}, std::nullopt);
    }

    TracklistEditor::~TracklistEditor()
    {
        m_pendingRebuild.disconnect();

        if (m_settleTick != 0)
            remove_tick_callback(m_settleTick);
    }

    void TracklistEditor::setScrollAdjustment(const Glib::RefPtr<Gtk::Adjustment>& adjustment)
    {
        m_scrollAdjustment = adjustment;
    }

    void TracklistEditor::setValue(const domain::Tracklist& tracklist)
    {
        m_pendingRebuild.disconnect();
        rebuild(tracklist, std::nullopt);
    }

    domain::Tracklist TracklistEditor::value() const
    {
        auto tracklist = allRows();

        // Empty rows are places held for tracks that are not filled in yet. Dropping them would shift the numbers
        // of every track after them, so they stay as long as there is anything filled in at all.
        if (std::ranges::all_of(tracklist, domain::isPlaceholder))
            return {};

        domain::normalizeDiscs(tracklist);
        return tracklist;
    }

    bool TracklistEditor::isValid() const
    {
        return std::ranges::all_of(m_rows, [](const std::unique_ptr<Row>& row) { return row->isValid(); });
    }

    sigc::signal<void()>& TracklistEditor::signalChanged()
    {
        return m_signalChanged;
    }

    domain::Tracklist TracklistEditor::allRows() const
    {
        domain::Tracklist tracklist;
        for (const auto& row : m_rows)
            tracklist.push_back(row->value());

        return tracklist;
    }

    void TracklistEditor::rebuild(const domain::Tracklist& source, const std::optional<FocusRequest> focus)
    {
        const double scrollValue = m_scrollAdjustment ? m_scrollAdjustment->get_value() : 0.0;

        // Only reorders when the discs are out of order and closes gaps in their numbers, so the rows keep their indices.
        auto tracklist = source;
        domain::normalizeDiscs(tracklist);

        m_rows.clear();
        m_discHeaders.clear();
        while (auto* child = m_grid.get_first_child())
            m_grid.remove(*child);

        m_grid.set_visible(!tracklist.empty());
        if (!tracklist.empty())
            addHeader();

        const bool showDiscHeaders = domain::discCount(tracklist) > 1;
        int gridRow = FIRST_TRACK_GRID_ROW;
        int currentDisc = 0;
        std::size_t numberOnDisc = 0;

        for (std::size_t index = 0; index < tracklist.size(); ++index)
        {
            if (tracklist[index].disc != currentDisc)
            {
                currentDisc = tracklist[index].disc;
                numberOnDisc = 0;

                if (showDiscHeaders)
                    addDiscHeader(currentDisc, gridRow++);
            }

            ++numberOnDisc;

            auto row = std::make_unique<Row>(m_grid, gridRow++, currentDisc, numberOnDisc, index, tracklist.size());
            row->setValue(tracklist[index]);

            row->signalChanged().connect([this]
            {
                refreshTotal();
                m_signalChanged.emit();
            });
            row->signalActivate().connect([this, index] { onActivateRequested(index); });
            row->signalMoveUp().connect([this, index] { onMoveUpRequested(index); });
            row->signalMoveDown().connect([this, index] { onMoveDownRequested(index); });
            row->signalRemove().connect([this, index] { onRemoveRequested(index); });

            m_rows.push_back(std::move(row));
        }

        refreshTotal();
        settleView(scrollValue, focus);
    }

    // The new rows have no size until the next layout. Focusing one of them before that makes the scrolled window
    // scroll to a place that does not exist yet, which is what threw the view to the top. So the scroll position is
    // put back first and the focus is given after the layout, when the window can scroll to the row properly.
    void TracklistEditor::settleView(const double scrollValue, const std::optional<FocusRequest> focus)
    {
        if (m_settleTick != 0)
        {
            remove_tick_callback(m_settleTick);
            m_settleTick = 0;
        }

        if (!m_scrollAdjustment)
        {
            focusRow(focus);
            return;
        }

        if (!focus && scrollValue <= 0.0)
            return;

        m_settleFrames = 0;
        m_settleTick = add_tick_callback([this, scrollValue, focus](const Glib::RefPtr<Gdk::FrameClock>&)
        {
            m_scrollAdjustment->set_value(scrollValue);

            if (++m_settleFrames < SETTLE_FRAMES)
                return true;

            m_settleTick = 0;
            focusRow(focus);
            return false;
        });
    }

    void TracklistEditor::focusRow(const std::optional<FocusRequest>& focus) const
    {
        if (focus && focus->row < m_rows.size())
            m_rows[focus->row]->focus(focus->target);
    }

    // Rows are destroyed by a rebuild, so it must never run inside a signal emitted by one of them.
    void TracklistEditor::scheduleRebuild(domain::Tracklist tracklist, const std::optional<FocusRequest> focus)
    {
        m_pendingTracklist = std::move(tracklist);
        m_pendingFocus = focus;

        if (m_pendingRebuild.connected())
            return;

        m_pendingRebuild = Glib::signal_idle().connect([this]
        {
            rebuild(m_pendingTracklist, m_pendingFocus);
            m_signalChanged.emit();
            return false;
        });
    }

    void TracklistEditor::addHeader()
    {
        auto* titleTrackHeader = createHeaderLabel(TITLE_TRACK_SYMBOL, Gtk::Align::CENTER);
        titleTrackHeader->set_tooltip_text(translate("kpop.tracklist.titleTrackTooltip"));

        auto* bonusHeader = createHeaderLabel(translate("kpop.tracklist.bonus"), Gtk::Align::CENTER);
        bonusHeader->set_tooltip_text(translate("kpop.tracklist.bonusTooltip"));

        m_grid.attach(*createHeaderLabel(translate("kpop.tracklist.number"), Gtk::Align::END), NUMBER_COLUMN, HEADER_GRID_ROW);
        m_grid.attach(*createHeaderLabel(translate("kpop.tracklist.title"), Gtk::Align::START), TITLE_COLUMN, HEADER_GRID_ROW);
        m_grid.attach(*createHeaderLabel(translate("kpop.tracklist.writers"), Gtk::Align::START), WRITERS_COLUMN, HEADER_GRID_ROW);
        m_grid.attach(*createHeaderLabel(translate("kpop.tracklist.length"), Gtk::Align::START), LENGTH_COLUMN, HEADER_GRID_ROW);
        m_grid.attach(*titleTrackHeader, TITLE_TRACK_COLUMN, HEADER_GRID_ROW);
        m_grid.attach(*bonusHeader, BONUS_COLUMN, HEADER_GRID_ROW);
    }

    void TracklistEditor::addDiscHeader(const int disc, const int gridRow)
    {
        auto* header = Gtk::make_managed<Box>(Gtk::Orientation::HORIZONTAL, EDITOR_SPACING);
        header->set_margin_top(DISC_HEADER_MARGIN_TOP);

        auto* title = Gtk::make_managed<Gtk::Label>(translate("kpop.tracklist.disc", { { "number", std::to_string(disc) } }));
        title->add_css_class("heading");

        auto* total = Gtk::make_managed<Gtk::Label>();
        total->add_css_class("dim-label");
        total->set_hexpand(true);
        total->set_halign(Gtk::Align::START);

        auto* addTrackButton = Gtk::make_managed<Gtk::Button>(translate("kpop.tracklist.add"));
        addTrackButton->add_css_class("flat");
        addTrackButton->signal_clicked().connect([this, disc] { onAddTrackToDisc(disc); });

        auto* removeDiscButton = Gtk::make_managed<Gtk::Button>(translate("kpop.tracklist.removeDisc"));
        removeDiscButton->add_css_class("flat");
        removeDiscButton->signal_clicked().connect([this, disc] { onRemoveDisc(disc); });

        header->append(*title);
        header->append(*total);
        header->append(*addTrackButton);
        header->append(*removeDiscButton);

        m_grid.attach(*header, NUMBER_COLUMN, gridRow, COLUMN_COUNT, 1);
        m_discHeaders.push_back({ .disc = disc, .totalLabel = total });
    }

    // New tracks go to the end of the last disc.
    void TracklistEditor::onAddClicked()
    {
        auto tracklist = allRows();

        domain::Track track;
        track.disc = tracklist.empty() ? 1 : tracklist.back().disc;
        tracklist.push_back(track);

        const auto newRow = tracklist.size() - 1;
        scheduleRebuild(std::move(tracklist), FocusRequest{ .row = newRow });
    }

    void TracklistEditor::onAddDiscClicked()
    {
        auto tracklist = allRows();

        domain::Track track;
        track.disc = tracklist.empty() ? 1 : tracklist.back().disc + 1;
        tracklist.push_back(track);

        const auto newRow = tracklist.size() - 1;
        scheduleRebuild(std::move(tracklist), FocusRequest{ .row = newRow });
    }

    void TracklistEditor::onAddTrackToDisc(const int disc)
    {
        auto tracklist = allRows();

        std::size_t insertAt = tracklist.size();
        for (std::size_t index = 0; index < tracklist.size(); ++index)
        {
            if (tracklist[index].disc == disc)
                insertAt = index + 1;
        }

        domain::Track track;
        track.disc = disc;
        tracklist.insert(tracklist.begin() + static_cast<std::ptrdiff_t>(insertAt), track);
        scheduleRebuild(std::move(tracklist), FocusRequest{ .row = insertAt });
    }

    void TracklistEditor::onRemoveDisc(const int disc)
    {
        auto tracklist = allRows();
        std::erase_if(tracklist, [disc](const domain::Track& track) { return track.disc == disc; });
        scheduleRebuild(std::move(tracklist), std::nullopt);
    }

    void TracklistEditor::onRemoveRequested(const std::size_t index)
    {
        auto tracklist = allRows();
        if (index >= tracklist.size())
            return;

        tracklist.erase(tracklist.begin() + static_cast<std::ptrdiff_t>(index));
        scheduleRebuild(std::move(tracklist), std::nullopt);
    }

    // The focus stays on the arrow that was clicked, so a track can be moved far with a series of clicks.
    void TracklistEditor::onMoveUpRequested(const std::size_t index)
    {
        auto tracklist = allRows();
        if (index == 0 || index >= tracklist.size())
            return;

        // The first track of a disc moves to the end of the disc before it.
        auto focusedRow = index - 1;
        if (tracklist[index - 1].disc == tracklist[index].disc)
        {
            std::swap(tracklist[index - 1], tracklist[index]);
        }
        else
        {
            tracklist[index].disc = tracklist[index - 1].disc;
            focusedRow = index;
        }

        scheduleRebuild(std::move(tracklist), FocusRequest{ .row = focusedRow, .target = FocusTarget::MoveUp });
    }

    void TracklistEditor::onMoveDownRequested(const std::size_t index)
    {
        auto tracklist = allRows();
        if (index + 1 >= tracklist.size())
            return;

        // The last track of a disc moves to the start of the disc after it.
        auto focusedRow = index + 1;
        if (tracklist[index].disc == tracklist[index + 1].disc)
        {
            std::swap(tracklist[index], tracklist[index + 1]);
        }
        else
        {
            tracklist[index].disc = tracklist[index + 1].disc;
            focusedRow = index;
        }

        scheduleRebuild(std::move(tracklist), FocusRequest{ .row = focusedRow, .target = FocusTarget::MoveDown });
    }

    // Enter in a title goes on to the next track, and after the last filled one it starts a new one.
    void TracklistEditor::onActivateRequested(const std::size_t index)
    {
        if (index >= m_rows.size())
            return;

        if (index + 1 < m_rows.size())
        {
            m_rows[index + 1]->focus(FocusTarget::Title);
            return;
        }

        if (!m_rows[index]->isBlank())
            onAddClicked();
    }

    void TracklistEditor::refreshTotal()
    {
        // Empty places are skipped by the totals themselves.
        const auto tracklist = allRows();

        const auto total = domain::totalLength(tracklist);
        m_totalLabel.set_visible(total.has_value());

        if (total)
            m_totalLabel.set_text(translate("kpop.tracklist.total", { { "length", total->toText() } }));

        for (const auto& header : m_discHeaders)
        {
            const auto discTotal = domain::totalLength(tracklist, header.disc);
            header.totalLabel->set_visible(discTotal.has_value());

            if (discTotal)
                header.totalLabel->set_text(discTotal->toText());
        }
    }
}
