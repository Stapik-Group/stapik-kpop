#include "ItemListView.hpp"

#include "kpop/ui/Translate.hpp"
#include "kpop/ui/widget/ImageTexture.hpp"

#include "stapik/domain/Category.hpp"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/picture.h>

#include <algorithm>

namespace kpop::ui
{
    namespace
    {
        constexpr int ROW_SPACING = 10;
        constexpr int ROW_MARGIN = 6;
        constexpr int SWATCH_WIDTH = 6;
        constexpr int PLACEHOLDER_MARGIN = 24;
        constexpr int THUMBNAIL_SIZE = 48;
        constexpr int THUMBNAIL_DECODE_SIZE = THUMBNAIL_SIZE * 2;

        Gtk::Picture* makeThumbnail(const std::filesystem::path& imagePath)
        {
            auto* thumbnail = Gtk::make_managed<Gtk::Picture>();
            thumbnail->set_size_request(THUMBNAIL_SIZE, THUMBNAIL_SIZE);
            thumbnail->set_content_fit(Gtk::ContentFit::CONTAIN);
            thumbnail->set_valign(Gtk::Align::CENTER);

            if (!imagePath.empty())
            {
                const Glib::RefPtr<Gdk::Paintable> texture = loadTexture(imagePath, THUMBNAIL_DECODE_SIZE);
                thumbnail->set_paintable(texture);
            }

            return thumbnail;
        }

        Gtk::Label* makeLabel(const std::string& text, const char* cssClass)
        {
            auto* label = Gtk::make_managed<Gtk::Label>(text);
            label->add_css_class(cssClass);
            return label;
        }

        const char* statusCssClass(const domain::ItemStatus status)
        {
            switch (status)
            {
                case domain::ItemStatus::Owned: return "kpop-status-owned";
                case domain::ItemStatus::Ordered: return "kpop-status-ordered";
                case domain::ItemStatus::Wishlist: return "kpop-status-wishlist";
                case domain::ItemStatus::Sold: return "kpop-status-sold";
            }

            return "kpop-status-owned";
        }
    }

    ItemListView::ItemListView()
    {
        set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
        set_vexpand(true);
        set_child(m_listBox);

        m_listBox.add_css_class("kpop-list");
        m_listBox.set_selection_mode(Gtk::SelectionMode::SINGLE);

        m_placeholderLabel.set_margin(PLACEHOLDER_MARGIN);
        m_listBox.set_placeholder(m_placeholderLabel);
        refreshPlaceholder();

        m_listBox.signal_row_activated().connect([this](const Gtk::ListBoxRow* row)
        {
            if (row == nullptr)
                return;

            if (const auto index = static_cast<std::size_t>(row->get_index()); index < m_rowIds.size())
                m_signalEditRequested.emit(m_rowIds[index]);
        });
    }

    void ItemListView::setRows(const std::vector<ItemRow>& rows)
    {
        clear();

        // Room for the images is only reserved when there is something to show, so the titles stay aligned.
        const bool showThumbnails = std::ranges::any_of(rows, [](const ItemRow& row) { return !row.imagePath.empty(); });

        for (const auto& row : rows)
            appendRow(row, showThumbnails);
    }

    void ItemListView::refreshPlaceholder()
    {
        m_placeholderLabel.set_text(translate("kpop.list.empty"));
    }

    sigc::signal<void(const std::string&)>& ItemListView::signalEditRequested()
    {
        return m_signalEditRequested;
    }

    sigc::signal<void(const std::string&)>& ItemListView::signalDeleteRequested()
    {
        return m_signalDeleteRequested;
    }

    void ItemListView::clear()
    {
        while (auto* row = m_listBox.get_row_at_index(0))
            m_listBox.remove(*row);

        m_rowIds.clear();
    }

    void ItemListView::appendRow(const ItemRow& row, const bool showThumbnail)
    {
        auto* box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, ROW_SPACING);
        box->add_css_class("kpop-row");
        box->set_margin(ROW_MARGIN);

        auto* swatch = Gtk::make_managed<Gtk::Label>();
        swatch->set_size_request(SWATCH_WIDTH, -1);
        swatch->add_css_class("kpop-kind-swatch");
        swatch->add_css_class(stapik::domain::categoryColorCssClass(domain::colorOf(row.kind)));
        box->append(*swatch);

        if (showThumbnail)
            box->append(*makeThumbnail(row.imagePath));

        auto* texts = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
        texts->set_hexpand(true);
        texts->set_valign(Gtk::Align::CENTER);

        auto* title = makeLabel(row.title, "kpop-row-title");
        title->set_halign(Gtk::Align::START);
        title->set_xalign(0.0F);
        title->set_ellipsize(Pango::EllipsizeMode::END);
        texts->append(*title);

        if (!row.subtitle.empty())
        {
            auto* subtitle = makeLabel(row.subtitle, "kpop-row-subtitle");
            subtitle->set_halign(Gtk::Align::START);
            subtitle->set_xalign(0.0F);
            subtitle->set_ellipsize(Pango::EllipsizeMode::END);
            texts->append(*subtitle);
        }

        box->append(*texts);

        for (const auto* extraText : { &row.quantityText, &row.priceText })
        {
            if (!extraText->empty())
                box->append(*makeLabel(*extraText, "kpop-row-extra"));
        }

        auto* status = makeLabel(row.statusText, "kpop-badge");
        status->add_css_class(statusCssClass(row.status));
        status->set_valign(Gtk::Align::CENTER);
        box->append(*status);

        auto* editButton = Gtk::make_managed<Gtk::Button>(translate("kpop.button.edit"));
        editButton->set_valign(Gtk::Align::CENTER);
        editButton->signal_clicked().connect([this, itemId = row.id] { m_signalEditRequested.emit(itemId); });
        box->append(*editButton);

        auto* deleteButton = Gtk::make_managed<Gtk::Button>(translate("kpop.button.remove"));
        deleteButton->set_valign(Gtk::Align::CENTER);
        deleteButton->signal_clicked().connect([this, itemId = row.id] { m_signalDeleteRequested.emit(itemId); });
        box->append(*deleteButton);

        m_listBox.append(*box);
        m_rowIds.push_back(row.id);
    }
}
