#include "DetailsForm.hpp"

#include "kpop/ui/Translate.hpp"
#include "kpop/ui/dialog/TracklistDialog.hpp"
#include "kpop/ui/widget/EnumDropDown.hpp"
#include "kpop/ui/widget/MultilineText.hpp"
#include "kpop/ui/widget/PartialDateEntry.hpp"
#include "kpop/ui/widget/QuickPickEntry.hpp"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/window.h>

#include <algorithm>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <utility>

namespace kpop::ui
{
    bool DetailsForm::isValid() const
    {
        return true;
    }

    sigc::signal<void()>& DetailsForm::signalChanged()
    {
        return m_signalChanged;
    }

    void DetailsForm::notifyChanged() const
    {
        m_signalChanged.emit();
    }

    namespace
    {
        using namespace domain;

        void connectChanged(const std::initializer_list<Gtk::Editable*> editables, const sigc::slot<void()>& onChanged)
        {
            for (auto* editable : editables)
                editable->signal_changed().connect(onChanged);
        }

        constexpr int TRACKLIST_ROW_SPACING = 8;
        constexpr int TRACKLIST_BOX_SPACING = 6;
        constexpr int PREVIEW_MAX_HEIGHT = 320;
        constexpr int PREVIEW_MARGIN = 10;
        constexpr int PREVIEW_COLUMN_SPACING = 14;
        constexpr int PREVIEW_ROW_SPACING = 5;
        constexpr int PREVIEW_COLUMNS = 5;
        constexpr auto SUMMARY_SEPARATOR = "  ·  ";
        constexpr auto EMPTY_TITLE_TEXT = "—";

        std::string tracklistSummary(const Tracklist& tracklist)
        {
            if (tracklist.empty())
                return translate("kpop.tracklist.summary.empty");

            auto text = translate("kpop.tracklist.summary", { { "count", std::to_string(tracklist.size()) } });

            if (const auto total = totalLength(tracklist))
                text += SUMMARY_SEPARATOR + translate("kpop.tracklist.total", { { "length", total->toText() } });

            if (const auto gaps = std::ranges::count_if(tracklist, isPlaceholder); gaps > 0)
                text += SUMMARY_SEPARATOR + translate("kpop.tracklist.summary.gaps", { { "count", std::to_string(gaps) } });

            return text;
        }

        class AlbumForm final : public DetailsForm
        {
        public:
            AlbumForm() :
                m_typeDropDown(ALBUM_TYPES),
                m_formatDropDown(ALBUM_FORMATS),
                m_regionDropDown(REGIONS),
                m_regionBox(Gtk::Orientation::VERTICAL, TRACKLIST_BOX_SPACING),
                m_tracklistBox(Gtk::Orientation::VERTICAL, TRACKLIST_BOX_SPACING),
                m_tracklistRow(Gtk::Orientation::HORIZONTAL, TRACKLIST_ROW_SPACING)
            {
                addRow("kpop.field.albumType", m_typeDropDown);
                addRow("kpop.field.albumFormat", m_formatDropDown);
                addRow("kpop.field.edition", m_editionEntry);
                addRow("kpop.field.variant", m_variantEntry);
                addRow("kpop.field.releaseDate", m_releaseDateEntry);
                addRow("kpop.field.label", m_labelEntry);
                // A region from the list, or any text when "other" is chosen.
                m_regionOtherEntry.set_visible(false);
                m_regionBox.append(m_regionDropDown);
                m_regionBox.append(m_regionOtherEntry);
                addRow("kpop.field.region", m_regionBox);
                addRow("kpop.field.catalogNumber", m_catalogNumberEntry);
                addRow("kpop.field.inclusions", m_inclusionsEntry);

                // The tracklist is edited in a window of its own; here only its summary, a short read-only preview
                // and the button are shown.
                m_tracklistSummary.set_hexpand(true);
                m_tracklistSummary.set_xalign(0.0F);
                m_tracklistRow.append(m_tracklistSummary);
                m_tracklistRow.append(m_tracklistButton);
                m_tracklistBox.append(m_tracklistRow);

                m_tracklistPreviewGrid.set_margin(PREVIEW_MARGIN);
                m_tracklistPreviewGrid.set_column_spacing(PREVIEW_COLUMN_SPACING);
                m_tracklistPreviewGrid.set_row_spacing(PREVIEW_ROW_SPACING);
                m_tracklistPreview.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
                m_tracklistPreview.set_propagate_natural_height(true);
                m_tracklistPreview.set_max_content_height(PREVIEW_MAX_HEIGHT);
                m_tracklistPreview.add_css_class("frame");
                m_tracklistPreview.set_child(m_tracklistPreviewGrid);
                m_tracklistBox.append(m_tracklistPreview);

                addRow("kpop.field.tracklist", m_tracklistBox);
                m_tracklistButton.signal_clicked().connect([this] { onTracklistClicked(); });
                refreshTracklist();

                const auto onChanged = [this] { notifyChanged(); };
                m_typeDropDown.property_selected().signal_changed().connect(onChanged);
                m_formatDropDown.property_selected().signal_changed().connect(onChanged);
                m_regionDropDown.property_selected().signal_changed().connect([this]
                {
                    m_regionOtherEntry.set_visible(m_regionDropDown.value() == Region::Other);
                    notifyChanged();
                });
                connectChanged({ &m_editionEntry, &m_variantEntry, &m_releaseDateEntry, &m_labelEntry, &m_regionOtherEntry, &m_catalogNumberEntry, &m_inclusionsEntry }, onChanged);
            }

            void setDetails(const ItemDetails& details) override
            {
                const auto&[type, format, edition, variant, releaseDate, label, region, catalogNumber, inclusions, tracklist] = std::get<AlbumDetails>(details);
                m_typeDropDown.setValue(type);
                m_formatDropDown.setValue(format);
                m_editionEntry.set_text(edition);
                m_variantEntry.set_text(variant);
                m_releaseDateEntry.setValue(releaseDate);
                m_labelEntry.set_text(label);
                const auto knownRegion = regionOf(region);
                m_regionDropDown.setValue(knownRegion);
                m_regionOtherEntry.set_text(knownRegion == Region::Other ? region : std::string());
                m_regionOtherEntry.set_visible(knownRegion == Region::Other);
                m_catalogNumberEntry.set_text(catalogNumber);
                m_inclusionsEntry.set_text(inclusions);
                m_tracklist = tracklist;
                refreshTracklist();
            }

            [[nodiscard]] ItemDetails details() const override
            {
                return AlbumDetails{
                    .type = m_typeDropDown.value(),
                    .format = m_formatDropDown.value(),
                    .edition = trimmedText(m_editionEntry),
                    .variant = trimmedText(m_variantEntry),
                    .releaseDate = m_releaseDateEntry.value(),
                    .label = trimmedText(m_labelEntry),
                    .region = regionText(m_regionDropDown.value(), trimmedText(m_regionOtherEntry)),
                    .catalogNumber = trimmedText(m_catalogNumberEntry),
                    .inclusions = trimmedText(m_inclusionsEntry),
                    .tracklist = m_tracklist };
            }

            // The tracklist is checked by its own window, so only an accepted one ends up here.
            [[nodiscard]] bool isValid() const override
            {
                return m_releaseDateEntry.isValid();
            }

        private:
            void refreshTracklist()
            {
                refreshTracklistSummary();
                refreshTracklistPreview();
            }

            void refreshTracklistSummary()
            {
                m_tracklistSummary.set_text(tracklistSummary(m_tracklist));
                m_tracklistButton.set_label(translate(m_tracklist.empty() ? "kpop.tracklist.button.add" : "kpop.tracklist.button.edit"));
            }

            // Number, title and length only, numbered per disc like in the tracklist window. Everything else is edited there.
            void refreshTracklistPreview()
            {
                while (auto* child = m_tracklistPreviewGrid.get_first_child())
                    m_tracklistPreviewGrid.remove(*child);

                m_tracklistPreview.set_visible(!m_tracklist.empty());

                const bool showDiscHeaders = discCount(m_tracklist) > 1;
                int gridRow = 0;
                int currentDisc = 0;
                int numberOnDisc = 0;

                for (const auto& track : m_tracklist)
                {
                    if (track.disc != currentDisc)
                    {
                        currentDisc = track.disc;
                        numberOnDisc = 0;

                        if (showDiscHeaders)
                        {
                            auto* header = Gtk::make_managed<Gtk::Label>(translate("kpop.tracklist.disc", { { "number", std::to_string(currentDisc) } }));
                            header->set_xalign(0.0F);
                            header->add_css_class("dim-label");
                            m_tracklistPreviewGrid.attach(*header, 0, gridRow++, PREVIEW_COLUMNS, 1);
                        }
                    }

                    ++numberOnDisc;

                    auto* number = Gtk::make_managed<Gtk::Label>(std::to_string(numberOnDisc));
                    number->set_xalign(1.0F);
                    number->add_css_class("dim-label");

                    auto* title = Gtk::make_managed<Gtk::Label>(track.title.empty() ? EMPTY_TITLE_TEXT : track.title);
                    title->set_hexpand(true);
                    title->set_xalign(0.0F);
                    title->set_ellipsize(Pango::EllipsizeMode::END);
                    if (track.title.empty())
                        title->add_css_class("dim-label");

                    auto* length = Gtk::make_managed<Gtk::Label>(track.length ? track.length->toText() : std::string());
                    length->set_xalign(1.0F);
                    length->add_css_class("dim-label");

                    m_tracklistPreviewGrid.attach(*number, 0, gridRow);
                    m_tracklistPreviewGrid.attach(*title, 1, gridRow);
                    m_tracklistPreviewGrid.attach(*length, 2, gridRow);
                    ++gridRow;
                }

                // The preview is as tall as its rows, up to the maximum, and scrolls beyond that. The height is set
                // from the measured content, because the natural height of a scrolled window inside the form's own
                // scroller is not passed on reliably and the last rows ended up hidden.
                int minimum = 0;
                int natural = 0;
                int minimumBaseline = 0;
                int naturalBaseline = 0;
                m_tracklistPreviewGrid.measure(Gtk::Orientation::VERTICAL, -1, minimum, natural, minimumBaseline, naturalBaseline);
                m_tracklistPreview.set_min_content_height(std::min(natural, PREVIEW_MAX_HEIGHT));
            }

            void onTracklistClicked()
            {
                auto* window = dynamic_cast<Gtk::Window*>(get_root());
                if (window == nullptr)
                    return;

                showTracklistDialog(*window, m_tracklist, [this](Tracklist tracklist)
                {
                    m_tracklist = std::move(tracklist);
                    refreshTracklist();
                    notifyChanged();
                });
            }

            EnumDropDown<AlbumType> m_typeDropDown;
            EnumDropDown<AlbumFormat> m_formatDropDown;
            Gtk::Entry m_editionEntry;
            Gtk::Entry m_variantEntry;
            PartialDateEntry m_releaseDateEntry;
            Gtk::Entry m_labelEntry;
            EnumDropDown<Region> m_regionDropDown;
            Gtk::Box m_regionBox;
            Gtk::Entry m_regionOtherEntry;
            Gtk::Entry m_catalogNumberEntry;
            Gtk::Entry m_inclusionsEntry;
            Tracklist m_tracklist;
            Gtk::Box m_tracklistBox;
            Gtk::Box m_tracklistRow;
            Gtk::Label m_tracklistSummary;
            Gtk::Button m_tracklistButton;
            Gtk::ScrolledWindow m_tracklistPreview;
            Gtk::Grid m_tracklistPreviewGrid;
        };

        class PhotocardForm final : public DetailsForm
        {
        public:
            PhotocardForm() :
                m_originDropDown(PHOTOCARD_ORIGINS),
                m_forTradeCheck(translate("kpop.field.forTradeCheck"))
            {
                addRow("kpop.field.member", m_memberEntry);
                addRow("kpop.field.photocardOrigin", m_originDropDown);
                addRow("kpop.field.source", m_sourceEntry);
                addRow("kpop.field.forTrade", m_forTradeCheck);

                const auto onChanged = [this] { notifyChanged(); };
                m_originDropDown.property_selected().signal_changed().connect(onChanged);
                m_forTradeCheck.signal_toggled().connect(onChanged);
                connectChanged({ &m_memberEntry }, onChanged);
                connectChanged({ &m_sourceEntry }, onChanged);
            }

            void setDetails(const ItemDetails& details) override
            {
                const auto&[member, origin, source, forTrade] = std::get<PhotocardDetails>(details);
                m_memberEntry.set_text(member);
                m_originDropDown.setValue(origin);
                m_sourceEntry.set_text(source);
                m_forTradeCheck.set_active(forTrade);
            }

            [[nodiscard]] ItemDetails details() const override
            {
                return PhotocardDetails{
                    .member = trimmedText(m_memberEntry),
                    .origin = m_originDropDown.value(),
                    .source = trimmedText(m_sourceEntry),
                    .forTrade = m_forTradeCheck.get_active() };
            }

        private:
            Gtk::Entry m_memberEntry;
            EnumDropDown<PhotocardOrigin> m_originDropDown;
            Gtk::Entry m_sourceEntry;
            Gtk::CheckButton m_forTradeCheck;
        };

        class MerchandiseForm final : public DetailsForm
        {
        public:
            MerchandiseForm() :
                m_typeDropDown(MERCHANDISE_TYPES),
                m_sizeEntry(APPAREL_SIZE_SUGGESTIONS),
                m_officialCheck(translate("kpop.field.officialCheck"))
            {
                addRow("kpop.field.merchandiseType", m_typeDropDown);
                addRow("kpop.field.size", m_sizeEntry);
                addRow("kpop.field.version", m_versionEntry);
                addRow("kpop.field.official", m_officialCheck);
                refreshSizeVisibility();

                const auto onChanged = [this] { notifyChanged(); };
                m_typeDropDown.property_selected().signal_changed().connect([this, onChanged]
                {
                    refreshSizeVisibility();
                    onChanged();
                });
                m_sizeEntry.signalChanged().connect(onChanged);
                m_officialCheck.signal_toggled().connect(onChanged);
                connectChanged({ &m_versionEntry }, onChanged);
            }

            void setDetails(const ItemDetails& details) override
            {
                const auto&[type, version, official, size] = std::get<MerchandiseDetails>(details);
                m_typeDropDown.setValue(type);
                m_sizeEntry.setText(size);
                m_versionEntry.set_text(version);
                m_officialCheck.set_active(official);
                refreshSizeVisibility();
            }

            [[nodiscard]] ItemDetails details() const override
            {
                const auto type = m_typeDropDown.value();
                return MerchandiseDetails{
                    .type = type,
                    .version = trimmedText(m_versionEntry),
                    .official = m_officialCheck.get_active(),
                    .size = hasSize(type) ? m_sizeEntry.text() : std::string() };
            }

        private:
            void refreshSizeVisibility()
            {
                setRowVisible(m_sizeEntry, hasSize(m_typeDropDown.value()));
            }

            EnumDropDown<MerchandiseType> m_typeDropDown;
            QuickPickEntry m_sizeEntry;
            Gtk::Entry m_versionEntry;
            Gtk::CheckButton m_officialCheck;
        };

        class ClipForm final : public DetailsForm
        {
        public:
            ClipForm() :
                m_typeDropDown(CLIP_TYPES),
                m_watchedCheck(translate("kpop.field.watchedCheck"))
            {
                addRow("kpop.field.clipType", m_typeDropDown);
                addRow("kpop.field.platform", m_platformEntry);
                addRow("kpop.field.url", m_urlEntry);
                addRow("kpop.field.releaseDate", m_releaseDateEntry);
                addRow("kpop.field.albumTitle", m_albumTitleEntry);
                addRow("kpop.field.watched", m_watchedCheck);

                const auto onChanged = [this] { notifyChanged(); };
                m_typeDropDown.property_selected().signal_changed().connect(onChanged);
                m_watchedCheck.signal_toggled().connect(onChanged);
                connectChanged({ &m_platformEntry, &m_urlEntry, &m_releaseDateEntry, &m_albumTitleEntry }, onChanged);
            }

            void setDetails(const ItemDetails& details) override
            {
                const auto&[type, platform, url, releaseDate, albumTitle, watched] = std::get<ClipDetails>(details);
                m_typeDropDown.setValue(type);
                m_platformEntry.set_text(platform);
                m_urlEntry.set_text(url);
                m_releaseDateEntry.setValue(releaseDate);
                m_albumTitleEntry.set_text(albumTitle);
                m_watchedCheck.set_active(watched);
            }

            [[nodiscard]] ItemDetails details() const override
            {
                return ClipDetails{
                    .type = m_typeDropDown.value(),
                    .platform = trimmedText(m_platformEntry),
                    .url = trimmedText(m_urlEntry),
                    .releaseDate = m_releaseDateEntry.value(),
                    .albumTitle = trimmedText(m_albumTitleEntry),
                    .watched = m_watchedCheck.get_active() };
            }

            [[nodiscard]] bool isValid() const override
            {
                return m_releaseDateEntry.isValid();
            }

        private:
            EnumDropDown<ClipType> m_typeDropDown;
            Gtk::Entry m_platformEntry;
            Gtk::Entry m_urlEntry;
            PartialDateEntry m_releaseDateEntry;
            Gtk::Entry m_albumTitleEntry;
            Gtk::CheckButton m_watchedCheck;
        };

        class LyricsForm final : public DetailsForm
        {
        public:
            LyricsForm() :
                m_originalText(160),
                m_romanizedText(120),
                m_translatedText(120)
            {
                addRow("kpop.field.albumTitle", m_albumTitleEntry);
                addRow("kpop.field.writers", m_writersEntry);
                addRow("kpop.field.originalText", m_originalText);
                addRow("kpop.field.romanizedText", m_romanizedText);
                addRow("kpop.field.translatedText", m_translatedText);
                addRow("kpop.field.translationLanguage", m_translationLanguageEntry);

                const auto onChanged = [this] { notifyChanged(); };
                connectChanged({ &m_albumTitleEntry, &m_writersEntry, &m_translationLanguageEntry }, onChanged);
                for (auto* text : { &m_originalText, &m_romanizedText, &m_translatedText })
                    text->signalChanged().connect(onChanged);
            }

            void setDetails(const ItemDetails& details) override
            {
                const auto&[albumTitle, writers, originalText, romanizedText, translatedText, translationLanguage] = std::get<LyricsDetails>(details);
                m_albumTitleEntry.set_text(albumTitle);
                m_writersEntry.set_text(writers);
                m_originalText.setText(originalText);
                m_romanizedText.setText(romanizedText);
                m_translatedText.setText(translatedText);
                m_translationLanguageEntry.set_text(translationLanguage);
            }

            [[nodiscard]] ItemDetails details() const override
            {
                return LyricsDetails{
                    .albumTitle = trimmedText(m_albumTitleEntry),
                    .writers = trimmedText(m_writersEntry),
                    .originalText = m_originalText.text(),
                    .romanizedText = m_romanizedText.text(),
                    .translatedText = m_translatedText.text(),
                    .translationLanguage = trimmedText(m_translationLanguageEntry) };
            }

        private:
            Gtk::Entry m_albumTitleEntry;
            Gtk::Entry m_writersEntry;
            MultilineText m_originalText;
            MultilineText m_romanizedText;
            MultilineText m_translatedText;
            Gtk::Entry m_translationLanguageEntry;
        };

        class EventForm final : public DetailsForm
        {
        public:
            EventForm() :
                m_typeDropDown(EVENT_TYPES)
            {
                addRow("kpop.field.eventType", m_typeDropDown);
                addRow("kpop.field.eventDate", m_dateEntry);
                addRow("kpop.field.venue", m_venueEntry);
                addRow("kpop.field.city", m_cityEntry);
                addRow("kpop.field.seat", m_seatEntry);

                const auto onChanged = [this] { notifyChanged(); };
                m_typeDropDown.property_selected().signal_changed().connect(onChanged);
                connectChanged({ &m_dateEntry, &m_venueEntry, &m_cityEntry, &m_seatEntry }, onChanged);
            }

            void setDetails(const ItemDetails& details) override
            {
                const auto&[type, date, venue, city, seat] = std::get<EventDetails>(details);
                m_typeDropDown.setValue(type);
                m_dateEntry.setValue(date);
                m_venueEntry.set_text(venue);
                m_cityEntry.set_text(city);
                m_seatEntry.set_text(seat);
            }

            [[nodiscard]] ItemDetails details() const override
            {
                return EventDetails{
                    .type = m_typeDropDown.value(),
                    .date = m_dateEntry.value(),
                    .venue = trimmedText(m_venueEntry),
                    .city = trimmedText(m_cityEntry),
                    .seat = trimmedText(m_seatEntry) };
            }

            [[nodiscard]] bool isValid() const override
            {
                return m_dateEntry.isValid();
            }

        private:
            EnumDropDown<EventType> m_typeDropDown;
            PartialDateEntry m_dateEntry;
            Gtk::Entry m_venueEntry;
            Gtk::Entry m_cityEntry;
            Gtk::Entry m_seatEntry;
        };
    }

    DetailsForm* createManagedDetailsForm(const ItemKind kind)
    {
        switch (kind)
        {
            case ItemKind::Album: return Gtk::make_managed<AlbumForm>();
            case ItemKind::Photocard: return Gtk::make_managed<PhotocardForm>();
            case ItemKind::Merchandise: return Gtk::make_managed<MerchandiseForm>();
            case ItemKind::Clip: return Gtk::make_managed<ClipForm>();
            case ItemKind::Lyrics: return Gtk::make_managed<LyricsForm>();
            case ItemKind::Event: return Gtk::make_managed<EventForm>();
        }

        throw std::invalid_argument("Unsupported item kind");
    }
}
