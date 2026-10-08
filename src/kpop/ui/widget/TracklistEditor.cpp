#include "TracklistEditor.hpp"

#include "TrackLengthEntry.hpp"

#include "kpop/ui/Translate.hpp"

#include <gtkmm/entry.h>
#include <gtkmm/togglebutton.h>

#include <glibmm/main.h>

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

        constexpr int NUMBER_COLUMN = 0;
        constexpr int TITLE_COLUMN = 1;
        constexpr int WRITERS_COLUMN = 2;
        constexpr int LENGTH_COLUMN = 3;
        constexpr int TITLE_TRACK_COLUMN = 4;
        constexpr int MOVE_UP_COLUMN = 5;
        constexpr int MOVE_DOWN_COLUMN = 6;
        constexpr int REMOVE_COLUMN = 7;

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
        Row(Gtk::Grid& grid, const std::size_t index, const std::size_t count) :
            m_numberLabel(Gtk::make_managed<Gtk::Label>(std::to_string(index + 1))),
            m_titleEntry(Gtk::make_managed<Gtk::Entry>()),
            m_writersEntry(Gtk::make_managed<Gtk::Entry>()),
            m_lengthEntry(Gtk::make_managed<TrackLengthEntry>()),
            m_titleTrackToggle(Gtk::make_managed<Gtk::ToggleButton>(TITLE_TRACK_SYMBOL)),
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
            m_moveUpButton->set_sensitive(index > 0);
            m_moveDownButton->set_sensitive(index + 1 < count);

            const int gridRow = FIRST_TRACK_GRID_ROW + static_cast<int>(index);
            grid.attach(*m_numberLabel, NUMBER_COLUMN, gridRow);
            grid.attach(*m_titleEntry, TITLE_COLUMN, gridRow);
            grid.attach(*m_writersEntry, WRITERS_COLUMN, gridRow);
            grid.attach(*m_lengthEntry, LENGTH_COLUMN, gridRow);
            grid.attach(*m_titleTrackToggle, TITLE_TRACK_COLUMN, gridRow);
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
        }

        [[nodiscard]] domain::Track value() const
        {
            domain::Track track;
            track.title = trimmedText(*m_titleEntry);
            track.writers = trimmedText(*m_writersEntry);
            track.length = m_lengthEntry->value();
            track.titleTrack = m_titleTrackToggle->get_active();
            return track;
        }

        [[nodiscard]] bool isBlank() const
        {
            return trimmedText(*m_titleEntry).empty()
                && trimmedText(*m_writersEntry).empty()
                && m_lengthEntry->isEmpty()
                && !m_titleTrackToggle->get_active();
        }

        // A row is either completely empty (and ignored) or has a title and a readable length.
        [[nodiscard]] bool isValid() const
        {
            return isBlank() || (!trimmedText(*m_titleEntry).empty() && m_lengthEntry->isValid());
        }

        void focusTitle() const
        {
            m_titleEntry->grab_focus();
        }

        sigc::signal<void()>& signalChanged() { return m_signalChanged; }
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

        Gtk::Label* m_numberLabel;
        Gtk::Entry* m_titleEntry;
        Gtk::Entry* m_writersEntry;
        TrackLengthEntry* m_lengthEntry;
        Gtk::ToggleButton* m_titleTrackToggle;
        Gtk::Button* m_moveUpButton;
        Gtk::Button* m_moveDownButton;
        Gtk::Button* m_removeButton;

        sigc::signal<void()> m_signalChanged;
        sigc::signal<void()> m_signalMoveUp;
        sigc::signal<void()> m_signalMoveDown;
        sigc::signal<void()> m_signalRemove;
    };

    TracklistEditor::TracklistEditor() :
        Box(Gtk::Orientation::VERTICAL, EDITOR_SPACING),
        m_addButton(translate("kpop.tracklist.add"))
    {
        add_css_class("kpop-tracklist");

        m_grid.set_column_spacing(GRID_COLUMN_SPACING);
        m_grid.set_row_spacing(GRID_ROW_SPACING);

        m_addButton.set_halign(Gtk::Align::START);
        m_addButton.signal_clicked().connect([this] { onAddClicked(); });

        m_totalLabel.set_hexpand(true);
        m_totalLabel.set_halign(Gtk::Align::END);

        auto* footer = Gtk::make_managed<Box>(Gtk::Orientation::HORIZONTAL, EDITOR_SPACING);
        footer->append(m_addButton);
        footer->append(m_totalLabel);

        append(m_grid);
        append(*footer);

        rebuild({}, std::nullopt);
    }

    TracklistEditor::~TracklistEditor()
    {
        m_pendingRebuild.disconnect();
    }

    void TracklistEditor::setValue(const domain::Tracklist& tracklist)
    {
        m_pendingRebuild.disconnect();
        rebuild(tracklist, std::nullopt);
    }

    domain::Tracklist TracklistEditor::value() const
    {
        domain::Tracklist tracklist;
        for (const auto& row : m_rows)
        {
            if (!row->isBlank())
                tracklist.push_back(row->value());
        }

        return tracklist;
    }

    bool TracklistEditor::isValid() const
    {
        for (const auto& row : m_rows)
        {
            if (!row->isValid())
                return false;
        }

        return true;
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

    void TracklistEditor::rebuild(const domain::Tracklist& tracklist, const std::optional<std::size_t> focusedRow)
    {
        m_rows.clear();
        while (auto* child = m_grid.get_first_child())
            m_grid.remove(*child);

        m_grid.set_visible(!tracklist.empty());
        if (!tracklist.empty())
            addHeader();

        for (std::size_t index = 0; index < tracklist.size(); ++index)
        {
            auto row = std::make_unique<Row>(m_grid, index, tracklist.size());
            row->setValue(tracklist[index]);

            row->signalChanged().connect([this]
            {
                refreshTotal();
                m_signalChanged.emit();
            });
            row->signalMoveUp().connect([this, index] { onMoveUpRequested(index); });
            row->signalMoveDown().connect([this, index] { onMoveDownRequested(index); });
            row->signalRemove().connect([this, index] { onRemoveRequested(index); });

            m_rows.push_back(std::move(row));
        }

        refreshTotal();

        if (focusedRow && *focusedRow < m_rows.size())
            m_rows[*focusedRow]->focusTitle();
    }

    // Rows are destroyed by a rebuild, so it must never run inside a signal emitted by one of them.
    void TracklistEditor::scheduleRebuild(domain::Tracklist tracklist, const std::optional<std::size_t> focusedRow)
    {
        m_pendingTracklist = std::move(tracklist);
        m_pendingFocusedRow = focusedRow;

        if (m_pendingRebuild.connected())
            return;

        m_pendingRebuild = Glib::signal_idle().connect([this]
        {
            rebuild(m_pendingTracklist, m_pendingFocusedRow);
            m_signalChanged.emit();
            return false;
        });
    }

    void TracklistEditor::addHeader()
    {
        auto* titleTrackHeader = createHeaderLabel(TITLE_TRACK_SYMBOL, Gtk::Align::CENTER);
        titleTrackHeader->set_tooltip_text(translate("kpop.tracklist.titleTrackTooltip"));

        m_grid.attach(*createHeaderLabel(translate("kpop.tracklist.number"), Gtk::Align::END), NUMBER_COLUMN, HEADER_GRID_ROW);
        m_grid.attach(*createHeaderLabel(translate("kpop.tracklist.title"), Gtk::Align::START), TITLE_COLUMN, HEADER_GRID_ROW);
        m_grid.attach(*createHeaderLabel(translate("kpop.tracklist.writers"), Gtk::Align::START), WRITERS_COLUMN, HEADER_GRID_ROW);
        m_grid.attach(*createHeaderLabel(translate("kpop.tracklist.length"), Gtk::Align::START), LENGTH_COLUMN, HEADER_GRID_ROW);
        m_grid.attach(*titleTrackHeader, TITLE_TRACK_COLUMN, HEADER_GRID_ROW);
    }

    void TracklistEditor::onAddClicked()
    {
        auto tracklist = allRows();
        tracklist.emplace_back();
        const auto newRow = tracklist.size() - 1;
        scheduleRebuild(std::move(tracklist), newRow);
    }

    void TracklistEditor::onRemoveRequested(const std::size_t index)
    {
        auto tracklist = allRows();
        if (index >= tracklist.size())
            return;

        tracklist.erase(tracklist.begin() + static_cast<std::ptrdiff_t>(index));
        scheduleRebuild(std::move(tracklist), std::nullopt);
    }

    void TracklistEditor::onMoveUpRequested(const std::size_t index)
    {
        auto tracklist = allRows();
        if (index == 0 || index >= tracklist.size())
            return;

        std::swap(tracklist[index - 1], tracklist[index]);
        scheduleRebuild(std::move(tracklist), index - 1);
    }

    void TracklistEditor::onMoveDownRequested(const std::size_t index)
    {
        auto tracklist = allRows();
        if (index + 1 >= tracklist.size())
            return;

        std::swap(tracklist[index], tracklist[index + 1]);
        scheduleRebuild(std::move(tracklist), index + 1);
    }

    void TracklistEditor::refreshTotal()
    {
        const auto total = domain::totalLength(value());
        m_totalLabel.set_visible(total.has_value());

        if (total)
            m_totalLabel.set_text(translate("kpop.tracklist.total", { { "length", total->toText() } }));
    }
}
