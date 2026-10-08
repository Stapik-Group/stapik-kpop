#include "QuickPickEntry.hpp"

#include "kpop/ui/Translate.hpp"

#include <gtkmm/button.h>

namespace kpop::ui
{
    namespace
    {
        constexpr int SPACING = 4;
        constexpr int ENTRY_WIDTH_CHARS = 8;
    }

    QuickPickEntry::QuickPickEntry(const std::span<const std::string_view> suggestions) :
        Gtk::Box(Gtk::Orientation::HORIZONTAL, SPACING)
    {
        add_css_class("kpop-quickpick");

        m_entry.set_hexpand(true);
        m_entry.set_width_chars(ENTRY_WIDTH_CHARS);
        append(m_entry);

        for (const auto suggestion : suggestions)
        {
            auto* button = Gtk::make_managed<Gtk::Button>(std::string(suggestion));
            button->add_css_class("flat");
            button->signal_clicked().connect([this, value = std::string(suggestion)] { m_entry.set_text(value); });
            append(*button);
        }

        m_entry.signal_changed().connect([this] { m_signalChanged.emit(); });
    }

    void QuickPickEntry::setText(const std::string& text)
    {
        m_entry.set_text(text);
    }

    std::string QuickPickEntry::text() const
    {
        return trimmedText(m_entry);
    }

    sigc::signal<void()>& QuickPickEntry::signalChanged()
    {
        return m_signalChanged;
    }
}
