#include "DetailsForm.hpp"

#include "kpop/ui/Translate.hpp"
#include "kpop/ui/widget/EnumDropDown.hpp"
#include "kpop/ui/widget/MultilineText.hpp"
#include "kpop/ui/widget/PartialDateEntry.hpp"

#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>

#include <initializer_list>
#include <stdexcept>

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

        class AlbumForm final : public DetailsForm
        {
        public:
            AlbumForm() :
                m_typeDropDown(ALBUM_TYPES),
                m_formatDropDown(ALBUM_FORMATS)
            {
                addRow("kpop.field.albumType", m_typeDropDown);
                addRow("kpop.field.albumFormat", m_formatDropDown);
                addRow("kpop.field.edition", m_editionEntry);
                addRow("kpop.field.releaseDate", m_releaseDateEntry);
                addRow("kpop.field.label", m_labelEntry);
                addRow("kpop.field.region", m_regionEntry);
                addRow("kpop.field.catalogNumber", m_catalogNumberEntry);
                addRow("kpop.field.inclusions", m_inclusionsEntry);

                const auto onChanged = [this] { notifyChanged(); };
                m_typeDropDown.property_selected().signal_changed().connect(onChanged);
                m_formatDropDown.property_selected().signal_changed().connect(onChanged);
                connectChanged({ &m_editionEntry, &m_releaseDateEntry, &m_labelEntry, &m_regionEntry, &m_catalogNumberEntry, &m_inclusionsEntry }, onChanged);
            }

            void setDetails(const ItemDetails& details) override
            {
                const auto&[type, format, edition, releaseDate, label, region, catalogNumber, inclusions] = std::get<AlbumDetails>(details);
                m_typeDropDown.setValue(type);
                m_formatDropDown.setValue(format);
                m_editionEntry.set_text(edition);
                m_releaseDateEntry.setValue(releaseDate);
                m_labelEntry.set_text(label);
                m_regionEntry.set_text(region);
                m_catalogNumberEntry.set_text(catalogNumber);
                m_inclusionsEntry.set_text(inclusions);
            }

            [[nodiscard]] ItemDetails details() const override
            {
                return AlbumDetails{
                    .type = m_typeDropDown.value(),
                    .format = m_formatDropDown.value(),
                    .edition = trimmedText(m_editionEntry),
                    .releaseDate = m_releaseDateEntry.value(),
                    .label = trimmedText(m_labelEntry),
                    .region = trimmedText(m_regionEntry),
                    .catalogNumber = trimmedText(m_catalogNumberEntry),
                    .inclusions = trimmedText(m_inclusionsEntry) };
            }

            [[nodiscard]] bool isValid() const override
            {
                return m_releaseDateEntry.isValid();
            }

        private:
            EnumDropDown<AlbumType> m_typeDropDown;
            EnumDropDown<AlbumFormat> m_formatDropDown;
            Gtk::Entry m_editionEntry;
            PartialDateEntry m_releaseDateEntry;
            Gtk::Entry m_labelEntry;
            Gtk::Entry m_regionEntry;
            Gtk::Entry m_catalogNumberEntry;
            Gtk::Entry m_inclusionsEntry;
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
                m_officialCheck(translate("kpop.field.officialCheck"))
            {
                addRow("kpop.field.merchandiseType", m_typeDropDown);
                addRow("kpop.field.version", m_versionEntry);
                addRow("kpop.field.official", m_officialCheck);

                const auto onChanged = [this] { notifyChanged(); };
                m_typeDropDown.property_selected().signal_changed().connect(onChanged);
                m_officialCheck.signal_toggled().connect(onChanged);
                connectChanged({ &m_versionEntry }, onChanged);
            }

            void setDetails(const ItemDetails& details) override
            {
                const auto&[type, version, official] = std::get<MerchandiseDetails>(details);
                m_typeDropDown.setValue(type);
                m_versionEntry.set_text(version);
                m_officialCheck.set_active(official);
            }

            [[nodiscard]] ItemDetails details() const override
            {
                return MerchandiseDetails{
                    .type = m_typeDropDown.value(),
                    .version = trimmedText(m_versionEntry),
                    .official = m_officialCheck.get_active() };
            }

        private:
            EnumDropDown<MerchandiseType> m_typeDropDown;
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
