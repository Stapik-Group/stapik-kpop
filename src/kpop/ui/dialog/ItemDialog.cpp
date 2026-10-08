#include "ItemDialog.hpp"

#include "ArtistEditDialog.hpp"

#include "kpop/ui/Translate.hpp"

#include "stapik/ui/dialog/DialogUtils.hpp"

#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stringlist.h>

#include <glibmm/ustring.h>

#include <utility>

namespace kpop::ui
{
    namespace
    {
        constexpr int DIALOG_WIDTH = 760;
        constexpr int DIALOG_HEIGHT = 720;
        constexpr int SECTION_SPACING = 12;
        constexpr int NOTES_MIN_HEIGHT = 70;
        constexpr int ARTIST_ROW_SPACING = 6;
        constexpr double MIN_QUANTITY = 1.0;
        constexpr double MAX_QUANTITY = 9999.0;
        constexpr double QUANTITY_STEP = 1.0;
        constexpr double QUANTITY_PAGE_STEP = 10.0;
    }

    ItemDialog::ItemDialog(
        Window& parent,
        app::CollectionController& controller,
        const ItemDialogOptions& options,
        std::function<void(domain::CollectionItem)> onAccept) :
        StapikDialog(parent, translate(options.existing ? "kpop.dialog.editItem.title" : "kpop.dialog.addItem.title")),
        m_controller(controller),
        m_existingId(options.existing ? options.existing->id : std::string()),
        m_kindDropDown(domain::ITEM_KINDS),
        m_newArtistButton("+"),
        m_statusDropDown(domain::ITEM_STATUSES),
        m_conditionDropDown(domain::ITEM_CONDITIONS),
        m_notesText(NOTES_MIN_HEIGHT)
    {
        buildLayout();

        addCancelButton();
        addOkButton();

        refreshArtists(options.existing ? options.existing->artistId : options.defaultArtistId);

        if (options.existing)
        {
            populate(*options.existing);
            m_kindDropDown.set_sensitive(false);
        }
        else
        {
            m_kindDropDown.setValue(options.defaultKind);
        }

        showFormOfSelectedKind();

        signal_response().connect([this, onAccept = std::move(onAccept)](const int response)
        {
            if (response == static_cast<int>(Gtk::ResponseType::OK) && onAccept)
                onAccept(collect());
        });

        updateConfirmSensitivity();
    }

    void ItemDialog::buildLayout()
    {
        set_default_size(DIALOG_WIDTH, DIALOG_HEIGHT);

        auto* artistRow = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, ARTIST_ROW_SPACING);
        m_artistDropDown.set_hexpand(true);
        m_newArtistButton.set_tooltip_text(translate("kpop.artist.newTooltip"));
        artistRow->append(m_artistDropDown);
        artistRow->append(m_newArtistButton);

        const auto labelSizeGroup = Gtk::SizeGroup::create(Gtk::SizeGroup::Mode::HORIZONTAL);
        m_topGrid.shareLabelWidth(labelSizeGroup);
        m_bottomGrid.shareLabelWidth(labelSizeGroup);

        m_topGrid.addRow("kpop.field.kind", m_kindDropDown);
        m_topGrid.addRow("kpop.field.title", m_titleEntry);
        m_topGrid.addRow("kpop.field.artist", *artistRow);

        for (const auto&[value, id] : domain::ITEM_KINDS.entries())
        {
            auto* form = createManagedDetailsForm(value);
            form->shareLabelWidth(labelSizeGroup);
            form->setDetails(domain::defaultDetails(value));
            form->signalChanged().connect([this] { updateConfirmSensitivity(); });

            m_detailsStack.add(*form, std::string(id));
            m_detailsForms.push_back(form);
        }
        m_detailsStack.set_vhomogeneous(false);

        m_quantitySpin.set_adjustment(Gtk::Adjustment::create(MIN_QUANTITY, MIN_QUANTITY, MAX_QUANTITY, QUANTITY_STEP, QUANTITY_PAGE_STEP));
        m_quantitySpin.set_numeric(true);
        m_quantitySpin.set_halign(Gtk::Align::START);

        m_bottomGrid.addRow("kpop.field.status", m_statusDropDown);
        m_bottomGrid.addRow("kpop.field.condition", m_conditionDropDown);
        m_bottomGrid.addRow("kpop.field.quantity", m_quantitySpin);
        m_bottomGrid.addRow("kpop.field.price", m_priceEntry);
        m_bottomGrid.addRow("kpop.field.acquiredOn", m_acquiredOnEntry);
        m_bottomGrid.addRow("kpop.field.acquiredFrom", m_acquiredFromEntry);
        m_bottomGrid.addRow("kpop.field.notes", m_notesText);

        auto* sections = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, SECTION_SPACING);
        sections->append(m_topGrid);
        sections->append(m_detailsStack);
        sections->append(m_bottomGrid);

        auto* scroller = Gtk::make_managed<Gtk::ScrolledWindow>();
        scroller->set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
        scroller->set_vexpand(true);
        scroller->set_propagate_natural_height(true);
        scroller->set_child(*sections);
        contentBox().append(*scroller);

        m_titleEntry.signal_changed().connect([this] { updateConfirmSensitivity(); });
        m_priceEntry.signalChanged().connect([this] { updateConfirmSensitivity(); });
        m_acquiredOnEntry.signal_changed().connect([this] { updateConfirmSensitivity(); });
        m_kindDropDown.property_selected().signal_changed().connect([this]
        {
            showFormOfSelectedKind();
            updateConfirmSensitivity();
        });
        m_newArtistButton.signal_clicked().connect([this] { onNewArtistClicked(); });
    }

    void ItemDialog::populate(const domain::CollectionItem& item)
    {
        m_kindDropDown.setValue(item.kind());
        m_titleEntry.set_text(item.title);
        m_statusDropDown.setValue(item.status);
        m_conditionDropDown.setValue(item.condition);
        m_quantitySpin.set_value(static_cast<double>(item.quantity));
        m_priceEntry.setValue(item.price);
        m_acquiredOnEntry.setValue(item.acquiredOn);
        m_acquiredFromEntry.set_text(item.acquiredFrom);
        m_notesText.setText(item.notes);

        m_detailsForms.at(static_cast<std::size_t>(item.kind()))->setDetails(item.details);
    }

    void ItemDialog::refreshArtists(const std::string& artistIdToSelect)
    {
        std::vector<Glib::ustring> labels{ translate("kpop.artist.none") };
        m_artistIds.assign(1, std::string());

        guint selectedPosition = 0;
        for (const auto& artist : m_controller.document().artists())
        {
            if (artist.id == artistIdToSelect)
                selectedPosition = static_cast<guint>(labels.size());

            labels.emplace_back(artist.name);
            m_artistIds.push_back(artist.id);
        }

        m_artistDropDown.set_model(Gtk::StringList::create(labels));
        m_artistDropDown.set_selected(selectedPosition);
    }

    void ItemDialog::showFormOfSelectedKind()
    {
        m_detailsStack.set_visible_child(std::string(domain::ITEM_KINDS.idOf(m_kindDropDown.value())));
    }

    void ItemDialog::onNewArtistClicked()
    {
        showArtistEditDialog(*this, std::nullopt, [this](domain::Artist artist)
        {
            const auto artistId = m_controller.addArtist(std::move(artist));
            refreshArtists(artistId);
        });
    }

    void ItemDialog::updateConfirmSensitivity()
    {
        set_response_sensitive(Gtk::ResponseType::OK, isInputValid());
    }

    bool ItemDialog::isInputValid() const
    {
        const auto* form = m_detailsForms.at(static_cast<std::size_t>(m_kindDropDown.value()));

        return !trimmedText(m_titleEntry).empty()
            && m_priceEntry.isValid()
            && m_acquiredOnEntry.isValid()
            && form->isValid();
    }

    domain::CollectionItem ItemDialog::collect()
    {
        m_quantitySpin.update();

        const auto artistPosition = static_cast<std::size_t>(m_artistDropDown.get_selected());

        domain::CollectionItem item;
        item.id = m_existingId;
        item.title = trimmedText(m_titleEntry);
        item.artistId = artistPosition < m_artistIds.size() ? m_artistIds[artistPosition] : std::string();
        item.status = m_statusDropDown.value();
        item.condition = m_conditionDropDown.value();
        item.quantity = m_quantitySpin.get_value_as_int();
        item.price = m_priceEntry.value();
        item.acquiredOn = m_acquiredOnEntry.value();
        item.acquiredFrom = trimmedText(m_acquiredFromEntry);
        item.notes = m_notesText.text();
        item.details = m_detailsForms.at(static_cast<std::size_t>(m_kindDropDown.value()))->details();
        return item;
    }

    void showItemDialog(
        Gtk::Window& parent,
        app::CollectionController& controller,
        const ItemDialogOptions& options,
        std::function<void(domain::CollectionItem)> onAccept)
    {
        showAutoDeletingDialog<ItemDialog>(parent, controller, options, std::move(onAccept));
    }
}
