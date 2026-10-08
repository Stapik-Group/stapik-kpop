#include "FormGrid.hpp"

#include "kpop/ui/Translate.hpp"
#include "kpop/ui/widget/MultilineText.hpp"

#include <gtkmm/label.h>

namespace kpop::ui
{
    namespace
    {
        constexpr int ROW_SPACING = 6;
        constexpr int COLUMN_SPACING = 12;
        constexpr int LABEL_COLUMN = 0;
        constexpr int FIELD_COLUMN = 1;
        constexpr int FULL_WIDTH_COLUMNS = 2;
    }

    FormGrid::FormGrid()
    {
        set_row_spacing(ROW_SPACING);
        set_column_spacing(COLUMN_SPACING);
    }

    void FormGrid::addRow(const std::string_view labelKey, Widget& field)
    {
        auto* label = Gtk::make_managed<Gtk::Label>(translate(labelKey));
        label->set_halign(Gtk::Align::START);
        label->set_xalign(0.0F);

        const bool isTallField = dynamic_cast<MultilineText*>(&field) != nullptr;
        label->set_valign(isTallField ? Gtk::Align::START : Gtk::Align::CENTER);

        field.set_hexpand(true);

        if (m_labelSizeGroup)
            m_labelSizeGroup->add_widget(*label);

        m_labels.push_back(label);
        m_labelOfField[&field] = label;
        attach(*label, LABEL_COLUMN, m_nextRow);
        attach(field, FIELD_COLUMN, m_nextRow);
        ++m_nextRow;
    }

    void FormGrid::addFullWidthRow(const std::string_view labelKey, Widget& field)
    {
        auto* label = Gtk::make_managed<Gtk::Label>(translate(labelKey));
        label->set_halign(Gtk::Align::START);
        label->set_xalign(0.0F);

        field.set_hexpand(true);

        attach(*label, LABEL_COLUMN, m_nextRow, FULL_WIDTH_COLUMNS, 1);
        attach(field, LABEL_COLUMN, m_nextRow + 1, FULL_WIDTH_COLUMNS, 1);
        m_nextRow += 2;
    }

    void FormGrid::setRowVisible(Widget& field, const bool visible)
    {
        field.set_visible(visible);

        if (const auto label = m_labelOfField.find(&field); label != m_labelOfField.end())
            label->second->set_visible(visible);
    }

    void FormGrid::shareLabelWidth(const Glib::RefPtr<Gtk::SizeGroup>& sizeGroup)
    {
        m_labelSizeGroup = sizeGroup;

        for (auto* label : m_labels)
            m_labelSizeGroup->add_widget(*label);
    }
}
