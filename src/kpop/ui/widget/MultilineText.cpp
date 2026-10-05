#include "MultilineText.hpp"

#include "stapik/text/Trim.hpp"

namespace kpop::ui
{
    MultilineText::MultilineText(const int minimumHeight)
    {
        set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
        set_min_content_height(minimumHeight);
        set_has_frame(true);
        set_child(m_textView);
        add_css_class("kpop-multiline");

        m_textView.set_wrap_mode(Gtk::WrapMode::WORD_CHAR);
        m_textView.get_buffer()->signal_changed().connect([this] { m_signalChanged.emit(); });
    }

    void MultilineText::setText(const std::string& text)
    {
        m_textView.get_buffer()->set_text(text);
    }

    std::string MultilineText::text() const
    {
        return stapik::text::trim(m_textView.get_buffer()->get_text().raw());
    }

    sigc::signal<void()>& MultilineText::signalChanged()
    {
        return m_signalChanged;
    }
}
