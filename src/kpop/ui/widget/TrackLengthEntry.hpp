#pragma once

#include "kpop/domain/TrackLength.hpp"

#include <gtkmm/entry.h>

#include <optional>

namespace kpop::ui
{
    class TrackLengthEntry : public Gtk::Entry
    {
    public:
        TrackLengthEntry();

        void setValue(const std::optional<domain::TrackLength>& length);
        [[nodiscard]] std::optional<domain::TrackLength> value() const;

        // An empty entry is valid and means "length unknown".
        [[nodiscard]] bool isValid() const;
        [[nodiscard]] bool isEmpty() const;

    private:
        void refreshValidityStyle();
    };
}
