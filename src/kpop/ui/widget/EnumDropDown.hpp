#pragma once

#include "kpop/ui/Translate.hpp"

#include <glibmm/ustring.h>
#include <gtkmm/dropdown.h>
#include <gtkmm/stringlist.h>

#include <algorithm>
#include <vector>

namespace kpop::ui
{
    template<typename EnumType>
    class EnumDropDown : public Gtk::DropDown
    {
    public:
        template<typename Catalog>
        explicit EnumDropDown(const Catalog& catalog)
        {
            std::vector<Glib::ustring> labels;
            for (const auto& catalogEntry : catalog.entries())
            {
                m_values.push_back(catalogEntry.value);
                labels.emplace_back(translate(catalog.nameKey(catalogEntry.value)));
            }

            set_model(Gtk::StringList::create(labels));
        }

        [[nodiscard]] EnumType value() const
        {
            return m_values.at(get_selected());
        }

        void setValue(const EnumType value)
        {
            const auto position = std::ranges::find(m_values, value);
            if (position != m_values.end())
                set_selected(static_cast<guint>(position - m_values.begin()));
        }

    private:
        std::vector<EnumType> m_values;
    };
}
