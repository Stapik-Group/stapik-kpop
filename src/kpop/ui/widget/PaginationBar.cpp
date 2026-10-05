#include "PaginationBar.hpp"

#include "kpop/ui/Translate.hpp"

namespace kpop::ui
{
    namespace
    {
        constexpr int SPACING = 12;
    }

    PaginationBar::PaginationBar() :
        Box(Gtk::Orientation::HORIZONTAL, SPACING)
    {
        set_halign(Gtk::Align::CENTER);

        append(m_previousButton);
        append(m_pageLabel);
        append(m_nextButton);

        m_previousButton.signal_clicked().connect([this] { m_signalPreviousRequested.emit(); });
        m_nextButton.signal_clicked().connect([this] { m_signalNextRequested.emit(); });

        refreshLabels();
        set_visible(false);
    }

    void PaginationBar::setPage(const std::size_t page, const std::size_t pageCount)
    {
        m_page = page;
        m_pageCount = pageCount;

        set_visible(pageCount > 1);
        m_previousButton.set_sensitive(page > 1);
        m_nextButton.set_sensitive(page < pageCount);
        refreshLabels();
    }

    void PaginationBar::refreshLabels()
    {
        m_previousButton.set_label(translate("kpop.page.previous"));
        m_nextButton.set_label(translate("kpop.page.next"));
        m_pageLabel.set_text(translate("kpop.page.label", {
            { "page", std::to_string(m_page) },
            { "pages", std::to_string(m_pageCount) } }));
    }

    sigc::signal<void()>& PaginationBar::signalPreviousRequested()
    {
        return m_signalPreviousRequested;
    }

    sigc::signal<void()>& PaginationBar::signalNextRequested()
    {
        return m_signalNextRequested;
    }
}
