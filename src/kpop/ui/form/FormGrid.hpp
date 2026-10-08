#pragma once

#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/sizegroup.h>
#include <gtkmm/widget.h>

#include <map>
#include <string_view>
#include <vector>

namespace kpop::ui
{
    class FormGrid : public Gtk::Grid
    {
    public:
        FormGrid();

        void addRow(std::string_view labelKey, Gtk::Widget& field);
        void addFullWidthRow(std::string_view labelKey, Gtk::Widget& field);
        void setRowVisible(Gtk::Widget& field, bool visible);
        void shareLabelWidth(const Glib::RefPtr<Gtk::SizeGroup>& sizeGroup);

    private:
        int m_nextRow = 0;
        Glib::RefPtr<Gtk::SizeGroup> m_labelSizeGroup;
        std::vector<Gtk::Label*> m_labels;
        std::map<Gtk::Widget*, Gtk::Label*> m_labelOfField;
    };
}
