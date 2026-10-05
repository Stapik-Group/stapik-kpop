#include "ArtistsDialog.hpp"

#include "ArtistEditDialog.hpp"

#include "kpop/ui/Translate.hpp"

#include "stapik/ui/dialog/ConfirmDialog.hpp"
#include "stapik/ui/dialog/DialogUtils.hpp"

#include <gtkmm/box.h>
#include <gtkmm/label.h>

namespace kpop::ui
{
    namespace
    {
        constexpr int LIST_MIN_HEIGHT = 260;
        constexpr int LIST_MIN_WIDTH = 420;
        constexpr int BUTTON_SPACING = 8;
        constexpr int ROW_MARGIN = 6;
    }

    ArtistsDialog::ArtistsDialog(Window& parent, app::CollectionController& controller) :
        StapikDialog(parent, translate("kpop.dialog.artists.title")),
        m_controller(controller),
        m_addButton(translate("kpop.button.add")),
        m_editButton(translate("kpop.button.edit")),
        m_removeButton(translate("kpop.button.remove"))
    {
        m_scroller.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
        m_scroller.set_min_content_height(LIST_MIN_HEIGHT);
        m_scroller.set_min_content_width(LIST_MIN_WIDTH);
        m_scroller.set_has_frame(true);
        m_scroller.set_vexpand(true);
        m_scroller.set_child(m_listBox);
        contentBox().append(m_scroller);

        auto* buttons = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, BUTTON_SPACING);
        buttons->append(m_addButton);
        buttons->append(m_editButton);
        buttons->append(m_removeButton);
        contentBox().append(*buttons);

        add_button(translate("kpop.button.close"), Gtk::ResponseType::CLOSE);

        m_listBox.set_selection_mode(Gtk::SelectionMode::SINGLE);
        m_listBox.signal_selected_rows_changed().connect([this] { updateButtonSensitivity(); });
        m_listBox.signal_row_activated().connect([this](Gtk::ListBoxRow*) { onEditClicked(); });

        m_addButton.signal_clicked().connect([this] { onAddClicked(); });
        m_editButton.signal_clicked().connect([this] { onEditClicked(); });
        m_removeButton.signal_clicked().connect([this] { onRemoveClicked(); });

        m_documentConnection = m_controller.signalDocumentChanged().connect([this] { rebuildList(); });

        rebuildList();
    }

    ArtistsDialog::~ArtistsDialog()
    {
        m_documentConnection.disconnect();
    }

    void ArtistsDialog::rebuildList()
    {
        while (auto* row = m_listBox.get_row_at_index(0))
            m_listBox.remove(*row);

        m_artistIds.clear();

        for (const auto& document = m_controller.document(); const auto& artist : document.artists())
        {
            const auto entryCount = document.countItemsOfArtist(artist.id);
            const auto text = artist.name + "  ·  " + translate(domain::ARTIST_TYPES.nameKey(artist.type)) + "  (" + std::to_string(entryCount) + ")";

            auto* label = Gtk::make_managed<Gtk::Label>(text);
            label->set_halign(Gtk::Align::START);
            label->set_xalign(0.0F);
            label->set_margin(ROW_MARGIN);

            m_listBox.append(*label);
            m_artistIds.push_back(artist.id);
        }

        updateButtonSensitivity();
    }

    void ArtistsDialog::updateButtonSensitivity()
    {
        const bool hasSelection = m_listBox.get_selected_row() != nullptr;
        m_editButton.set_sensitive(hasSelection);
        m_removeButton.set_sensitive(hasSelection);
    }

    const domain::Artist* ArtistsDialog::selectedArtist() const
    {
        const auto* row = m_listBox.get_selected_row();
        if (row == nullptr)
            return nullptr;

        const auto index = static_cast<std::size_t>(row->get_index());
        return index < m_artistIds.size() ? m_controller.document().findArtist(m_artistIds[index]) : nullptr;
    }

    void ArtistsDialog::onAddClicked()
    {
        showArtistEditDialog(*this, std::nullopt, [this](domain::Artist artist)
        {
            m_controller.addArtist(std::move(artist));
        });
    }

    void ArtistsDialog::onEditClicked()
    {
        const auto* artist = selectedArtist();
        if (artist == nullptr)
            return;

        showArtistEditDialog(*this, *artist, [this](const domain::Artist& edited)
        {
            m_controller.updateArtist(edited);
        });
    }

    void ArtistsDialog::onRemoveClicked()
    {
        const auto* artist = selectedArtist();
        if (artist == nullptr)
            return;

        if (const auto entryCount = m_controller.document().countItemsOfArtist(artist->id); entryCount > 0)
        {
            showMessageDialog(
                *this,
                translate("kpop.dialog.artistInUse.header"),
                translate("kpop.dialog.artistInUse.text", { { "name", artist->name }, { "count", std::to_string(entryCount) } }),
                Gtk::MessageType::WARNING);
            return;
        }

        const auto artistId = artist->id;
        showConfirmDialog(
            *this,
            ConfirmDialogOptions{
                .title = translate("kpop.dialog.deleteArtist.title"),
                .message = translate("kpop.dialog.deleteArtist.message", { { "name", artist->name } }),
                .confirmLabel = translate("kpop.button.remove"),
                .destructive = true },
            [this, artistId]
            {
                static_cast<void>(m_controller.removeArtist(artistId));
            });
    }
}
