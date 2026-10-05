#pragma once

#include "kpop/domain/PartialDate.hpp"

#include <gtkmm/entry.h>

#include <optional>

namespace kpop::ui
{
    class PartialDateEntry : public Gtk::Entry
    {
    public:
        PartialDateEntry();

        void setValue(const std::optional<domain::PartialDate>& date);
        [[nodiscard]] std::optional<domain::PartialDate> value() const;
        [[nodiscard]] bool isValid() const;

    private:
        void refreshValidityStyle();
    };
}
