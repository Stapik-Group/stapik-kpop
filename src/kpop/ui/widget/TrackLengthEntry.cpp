#include "TrackLengthEntry.hpp"

#include "kpop/ui/Translate.hpp"

namespace kpop::ui
{
    namespace
    {
        constexpr int WIDTH_CHARS = 6;
        constexpr int MAX_WIDTH_CHARS = 8;
    }

    TrackLengthEntry::TrackLengthEntry()
    {
        set_placeholder_text(translate("kpop.tracklist.lengthPlaceholder"));
        set_width_chars(WIDTH_CHARS);
        set_max_width_chars(MAX_WIDTH_CHARS);
        signal_changed().connect([this] { refreshValidityStyle(); });
    }

    void TrackLengthEntry::setValue(const std::optional<domain::TrackLength>& length)
    {
        set_text(length ? length->toText() : std::string());
    }

    std::optional<domain::TrackLength> TrackLengthEntry::value() const
    {
        return domain::TrackLength::parse(trimmedText(*this));
    }

    bool TrackLengthEntry::isValid() const
    {
        return isEmpty() || value().has_value();
    }

    bool TrackLengthEntry::isEmpty() const
    {
        return trimmedText(*this).empty();
    }

    void TrackLengthEntry::refreshValidityStyle()
    {
        if (isValid())
            remove_css_class("kpop-invalid");
        else
            add_css_class("kpop-invalid");
    }
}
