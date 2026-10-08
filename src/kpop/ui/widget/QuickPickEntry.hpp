#pragma once

#include <gtkmm/box.h>
#include <gtkmm/entry.h>

#include <sigc++/signal.h>

#include <span>
#include <string>
#include <string_view>

namespace kpop::ui
{
    // A free text entry with a row of buttons that fill it in with a common value.
    class QuickPickEntry : public Gtk::Box
    {
    public:
        explicit QuickPickEntry(std::span<const std::string_view> suggestions);

        void setText(const std::string& text);
        [[nodiscard]] std::string text() const;

        sigc::signal<void()>& signalChanged();

    private:
        Gtk::Entry m_entry;
        sigc::signal<void()> m_signalChanged;
    };
}
