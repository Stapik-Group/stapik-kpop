#include "PartialDateEntry.hpp"

#include "kpop/ui/Translate.hpp"

namespace kpop::ui
{
    PartialDateEntry::PartialDateEntry()
    {
        set_placeholder_text(translate("kpop.date.placeholder"));
        set_max_width_chars(12);
        signal_changed().connect([this] { refreshValidityStyle(); });
    }

    void PartialDateEntry::setValue(const std::optional<domain::PartialDate>& date)
    {
        set_text(date ? date->toKey() : std::string());
    }

    std::optional<domain::PartialDate> PartialDateEntry::value() const
    {
        return domain::PartialDate::parse(trimmedText(*this));
    }

    bool PartialDateEntry::isValid() const
    {
        return trimmedText(*this).empty() || value().has_value();
    }

    void PartialDateEntry::refreshValidityStyle()
    {
        if (isValid())
            remove_css_class("kpop-invalid");
        else
            add_css_class("kpop-invalid");
    }
}
