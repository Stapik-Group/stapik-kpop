#pragma once

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>

#include <sigc++/signal.h>

#include <cstddef>

namespace kpop::ui
{
    class PaginationBar : public Gtk::Box
    {
    public:
        PaginationBar();

        void setPage(std::size_t page, std::size_t pageCount);
        void refreshLabels();

        sigc::signal<void()>& signalPreviousRequested();
        sigc::signal<void()>& signalNextRequested();

    private:
        Gtk::Button m_previousButton;
        Gtk::Label m_pageLabel;
        Gtk::Button m_nextButton;
        std::size_t m_page = 1;
        std::size_t m_pageCount = 1;
        sigc::signal<void()> m_signalPreviousRequested;
        sigc::signal<void()> m_signalNextRequested;
    };
}
