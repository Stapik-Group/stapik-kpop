#pragma once

#include "kpop/domain/Artist.hpp"
#include "kpop/ui/form/FormGrid.hpp"
#include "kpop/ui/widget/EnumDropDown.hpp"
#include "kpop/ui/widget/MultilineText.hpp"

#include "stapik/ui/dialog/StapikDialog.hpp"

#include <gtkmm/entry.h>
#include <gtkmm/window.h>

#include <functional>
#include <optional>

namespace kpop::ui
{
    class ArtistEditDialog : public StapikDialog
    {
    public:
        ArtistEditDialog(Gtk::Window& parent, const std::optional<domain::Artist>& existing, std::function<void(domain::Artist)> onAccept);

    private:
        [[nodiscard]] domain::Artist collect() const;
        void updateConfirmSensitivity();

        std::string m_existingId;
        FormGrid m_grid;
        Gtk::Entry m_nameEntry;
        EnumDropDown<domain::ArtistType> m_typeDropDown;
        MultilineText m_membersText;
    };

    void showArtistEditDialog(Gtk::Window& parent, const std::optional<domain::Artist>& existing, std::function<void(domain::Artist)> onAccept);
}
