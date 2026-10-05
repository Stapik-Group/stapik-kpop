#pragma once

#include "kpop/app/CollectionController.hpp"

#include "stapik/ui/dialog/StapikDialog.hpp"

#include <gtkmm/button.h>
#include <gtkmm/listbox.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/window.h>

#include <sigc++/connection.h>

#include <string>
#include <vector>

namespace kpop::ui
{
    class ArtistsDialog : public StapikDialog
    {
    public:
        ArtistsDialog(Gtk::Window& parent, app::CollectionController& controller);
        ~ArtistsDialog() override;

        ArtistsDialog(const ArtistsDialog&) = delete;
        ArtistsDialog& operator=(const ArtistsDialog&) = delete;

    private:
        void rebuildList();
        void updateButtonSensitivity();
        [[nodiscard]] const domain::Artist* selectedArtist() const;
        void onAddClicked();
        void onEditClicked();
        void onRemoveClicked();

        app::CollectionController& m_controller;
        Gtk::ScrolledWindow m_scroller;
        Gtk::ListBox m_listBox;
        Gtk::Button m_addButton;
        Gtk::Button m_editButton;
        Gtk::Button m_removeButton;
        std::vector<std::string> m_artistIds;
        sigc::connection m_documentConnection;
    };
}
