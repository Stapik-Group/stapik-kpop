#pragma once

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>

#include <sigc++/signal.h>

#include <string>

namespace kpop::ui
{
    class MultilineText : public Gtk::ScrolledWindow
    {
    public:
        explicit MultilineText(int minimumHeight = 80);

        void setText(const std::string& text);

        [[nodiscard]] std::string text() const;

        sigc::signal<void()>& signalChanged();

    private:
        Gtk::TextView m_textView;
        sigc::signal<void()> m_signalChanged;
    };
}
