#pragma once

#include "kpop/app/CollectionController.hpp"
#include "kpop/ui/form/DetailsForm.hpp"
#include "kpop/ui/widget/EnumDropDown.hpp"
#include "kpop/ui/widget/MoneyEntry.hpp"
#include "kpop/ui/widget/MultilineText.hpp"
#include "kpop/ui/widget/PartialDateEntry.hpp"

#include "stapik/ui/dialog/StapikDialog.hpp"

#include <gtkmm/button.h>
#include <gtkmm/dropdown.h>
#include <gtkmm/entry.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/stack.h>
#include <gtkmm/window.h>

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace kpop::ui
{
    struct ItemDialogOptions
    {
        std::optional<domain::CollectionItem> existing;
        domain::ItemKind defaultKind = domain::ItemKind::Album;
        std::string defaultArtistId;
    };

    class ItemDialog : public StapikDialog
    {
    public:
        ItemDialog(Gtk::Window& parent, app::CollectionController& controller, const ItemDialogOptions& options, std::function<void(domain::CollectionItem)> onAccept);

    private:
        void buildLayout();
        void populate(const domain::CollectionItem& item);
        void refreshArtists(const std::string& artistIdToSelect);
        void showFormOfSelectedKind();
        void onNewArtistClicked();
        void updateConfirmSensitivity();
        [[nodiscard]] bool isInputValid() const;
        [[nodiscard]] domain::CollectionItem collect();

        app::CollectionController& m_controller;
        std::string m_existingId;
        std::vector<std::string> m_artistIds;

        FormGrid m_topGrid;
        FormGrid m_bottomGrid;
        EnumDropDown<domain::ItemKind> m_kindDropDown;
        Gtk::Entry m_titleEntry;
        Gtk::DropDown m_artistDropDown;
        Gtk::Button m_newArtistButton;
        Gtk::Stack m_detailsStack;
        std::vector<DetailsForm*> m_detailsForms;
        EnumDropDown<domain::ItemStatus> m_statusDropDown;
        EnumDropDown<domain::ItemCondition> m_conditionDropDown;
        Gtk::SpinButton m_quantitySpin;
        MoneyEntry m_priceEntry;
        PartialDateEntry m_acquiredOnEntry;
        Gtk::Entry m_acquiredFromEntry;
        MultilineText m_notesText;
    };

    void showItemDialog(Gtk::Window& parent, app::CollectionController& controller, const ItemDialogOptions& options, std::function<void(domain::CollectionItem)> onAccept);
}
