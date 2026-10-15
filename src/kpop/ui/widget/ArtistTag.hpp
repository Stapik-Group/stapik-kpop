#pragma once

#include "stapik/domain/CategoryColor.hpp"

#include <glibmm/ustring.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>

#include <optional>
#include <string>

namespace kpop::ui
{
    // A small tag with the name of the artist and a dot in the colour of the artist, shared by the list and the grid
    // so both show the artist the same way. When fillWidth is set the tag takes the width it is given and cuts a long
    // name with an ellipsis instead of widening its parent.
    inline Gtk::Box* makeArtistTag(const std::string& name, const std::optional<stapik::domain::CategoryColor>& color, const bool fillWidth)
    {
        constexpr int TAG_SPACING = 5;
        constexpr int DOT_SIZE = 8;

        auto* tag = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, TAG_SPACING);
        tag->add_css_class("kpop-artist-tag");
        tag->set_valign(Gtk::Align::CENTER);
        tag->set_tooltip_text(name);

        if (color)
        {
            auto* dot = Gtk::make_managed<Gtk::Label>();
            dot->set_size_request(DOT_SIZE, DOT_SIZE);
            dot->set_valign(Gtk::Align::CENTER);
            dot->add_css_class("kpop-artist-dot");
            dot->add_css_class(stapik::domain::categoryColorCssClass(*color));
            tag->append(*dot);
        }

        auto* label = Gtk::make_managed<Gtk::Label>(name);
        label->set_ellipsize(Pango::EllipsizeMode::END);
        if (fillWidth)
        {
            label->set_max_width_chars(1);
            label->set_hexpand(true);
            label->set_xalign(0.5F);
            tag->set_halign(Gtk::Align::FILL);
        }
        else
        {
            label->set_xalign(0.0F);
            tag->set_halign(Gtk::Align::START);
        }
        tag->append(*label);

        return tag;
    }
}
