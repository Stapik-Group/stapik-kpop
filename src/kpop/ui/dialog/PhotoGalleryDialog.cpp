#include "PhotoGalleryDialog.hpp"

#include "kpop/ui/Translate.hpp"
#include "kpop/ui/widget/ImageFileDialog.hpp"
#include "kpop/ui/widget/ImageTexture.hpp"

#include "stapik/ui/dialog/DialogUtils.hpp"
#include "stapik/ui/dialog/FileDrop.hpp"

#include <gtkmm/entry.h>
#include <gtkmm/frame.h>
#include <gtkmm/picture.h>

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <utility>

namespace kpop::ui
{
    namespace
    {
        constexpr int DIALOG_WIDTH = 760;
        constexpr int DIALOG_HEIGHT = 660;
        constexpr int LIST_MARGIN = 6;
        constexpr int LIST_SPACING = 12;
        constexpr int ROW_SPACING = 14;
        constexpr int ROW_MARGIN = 10;
        constexpr int CAPTION_SPACING = 4;
        constexpr int BUTTONS_SPACING = 4;
        constexpr int PHOTO_WIDTH = 280;
        constexpr int PHOTO_HEIGHT = 200;

        std::string trimmed(const std::string& text)
        {
            const auto first = text.find_first_not_of(" \t\r\n");
            if (first == std::string::npos)
                return {};

            const auto last = text.find_last_not_of(" \t\r\n");
            return text.substr(first, last - first + 1);
        }
    }

    bool importPhotos(
        image::ImageLibrary& library,
        const std::vector<std::filesystem::path>& files,
        std::vector<domain::ItemPhoto>& photos,
        std::string& failure)
    {
        bool imported = false;

        for (const auto& file : files)
        {
            std::optional<domain::ImageId> id;

            try
            {
                id = library.importFile(file);
            }
            catch (const std::runtime_error& error)
            {
                failure = error.what();
                continue;
            }

            imported = true;

            // The same picture is the same image, so it is not added a second time.
            const auto sameImage = [&id](const domain::ItemPhoto& photo) { return photo.image == *id; };
            if (std::ranges::none_of(photos, sameImage))
                photos.push_back({ .image = *id, .caption = {} });
        }

        return imported;
    }

    PhotoGalleryDialog::PhotoGalleryDialog(
        Gtk::Window& parent,
        image::ImageLibrary& library,
        const std::vector<domain::ItemPhoto>& photos,
        std::function<void(std::vector<domain::ItemPhoto>)> onAccept) :
        StapikDialog(parent, translate("kpop.dialog.photos.title")),
        m_library(library),
        m_photos(photos),
        m_list(Gtk::Orientation::VERTICAL, LIST_SPACING),
        m_addButton(translate("kpop.photos.add"))
    {
        set_default_size(DIALOG_WIDTH, DIALOG_HEIGHT);

        m_list.set_margin(LIST_MARGIN);
        m_scroller.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
        m_scroller.set_hexpand(true);
        m_scroller.set_vexpand(true);
        m_scroller.set_child(m_list);

        m_statusLabel.set_xalign(0.0F);
        m_statusLabel.set_wrap(true);

        m_addButton.set_halign(Gtk::Align::START);
        m_addButton.signal_clicked().connect([this] { onAddClicked(); });

        contentBox().append(m_scroller);
        contentBox().append(m_statusLabel);
        contentBox().append(m_addButton);

        stapik::ui::acceptFileDrop(
            contentBox(),
            [this](const std::vector<std::filesystem::path>& files) { onFilesChosen(files); },
            { .extensions = imageFileExtensions(), .allowMultiple = true });

        addCancelButton();
        addOkButton();

        signal_response().connect([this, onAccept = std::move(onAccept)](const int response)
        {
            if (response != static_cast<int>(Gtk::ResponseType::OK) || !onAccept)
                return;

            for (auto& photo : m_photos)
                photo.caption = trimmed(photo.caption);

            onAccept(m_photos);
        });

        rebuild();
    }

    void PhotoGalleryDialog::onAddClicked()
    {
        chooseImageFile(
            *this,
            translate("kpop.photos.dialogTitle"),
            [this](const std::filesystem::path& file) { onFilesChosen({ file }); },
            [this](const std::string& reason) { showImportError(reason); });
    }

    void PhotoGalleryDialog::onFilesChosen(const std::vector<std::filesystem::path>& files)
    {
        std::string failure;
        const bool imported = importPhotos(m_library, files, m_photos, failure);

        if (imported)
            rebuild();

        // After the rebuild, which clears the status line.
        if (!failure.empty())
            showImportError(failure);
    }

    void PhotoGalleryDialog::movePhoto(const std::size_t index, const int offset)
    {
        const auto target = static_cast<std::ptrdiff_t>(index) + offset;
        if (index >= m_photos.size() || target < 0 || target >= static_cast<std::ptrdiff_t>(m_photos.size()))
            return;

        std::swap(m_photos[index], m_photos[static_cast<std::size_t>(target)]);
        rebuild();
    }

    void PhotoGalleryDialog::removePhoto(const std::size_t index)
    {
        if (index >= m_photos.size())
            return;

        m_photos.erase(m_photos.begin() + static_cast<std::ptrdiff_t>(index));
        rebuild();
    }

    void PhotoGalleryDialog::rebuild()
    {
        while (auto* child = m_list.get_first_child())
            m_list.remove(*child);

        for (std::size_t index = 0; index < m_photos.size(); ++index)
            m_list.append(*createRow(index));

        m_statusLabel.remove_css_class("error");
        m_statusLabel.set_text(m_photos.empty() ? translate("kpop.photos.emptyHint") : translate("kpop.photos.dropHint"));
        m_statusLabel.add_css_class("dim-label");
    }

    void PhotoGalleryDialog::showImportError(const std::string& reason)
    {
        m_statusLabel.remove_css_class("dim-label");
        m_statusLabel.add_css_class("error");
        m_statusLabel.set_text(translate("kpop.image.importFailed", { { "reason", reason } }));
    }

    // One photo with the field for its caption on the right and the buttons that move or remove it.
    Gtk::Widget* PhotoGalleryDialog::createRow(const std::size_t index)
    {
        const auto path = m_library.pathOf(m_photos[index].image);

        // Decoded to fit the slot and shown pixel for pixel, so the photo is sharp whatever its proportions.
        const auto texture = path ? loadTexture(*path, PHOTO_WIDTH, PHOTO_HEIGHT) : Glib::RefPtr<Gdk::Texture>();

        auto* photoFrame = Gtk::make_managed<Gtk::Frame>();
        photoFrame->set_halign(Gtk::Align::CENTER);
        photoFrame->set_valign(Gtk::Align::CENTER);

        if (texture)
        {
            photoFrame->set_child(*makeExactPicture(texture));
        }
        else
        {
            auto* missing = Gtk::make_managed<Gtk::Label>("?");
            missing->set_size_request(PHOTO_WIDTH, PHOTO_HEIGHT);
            missing->set_tooltip_text(translate("kpop.image.missing"));
            photoFrame->set_child(*missing);
        }

        // Every photo gets the same slot, so the captions line up whether the photo is wide or tall.
        auto* photoSlot = Gtk::make_managed<Gtk::Box>();
        photoSlot->set_size_request(PHOTO_WIDTH, PHOTO_HEIGHT);
        photoSlot->set_valign(Gtk::Align::CENTER);
        photoSlot->append(*photoFrame);
        photoFrame->set_hexpand(true);
        photoFrame->set_vexpand(true);

        auto* captionLabel = Gtk::make_managed<Gtk::Label>(translate("kpop.photos.caption"));
        captionLabel->set_xalign(0.0F);
        captionLabel->add_css_class("dim-label");

        auto* captionEntry = Gtk::make_managed<Gtk::Entry>();
        captionEntry->set_hexpand(true);
        captionEntry->set_placeholder_text(translate("kpop.photos.captionPlaceholder"));
        captionEntry->set_text(m_photos[index].caption);

        // The rows are only rebuilt when a photo is moved, added or removed, so typing never loses the focus.
        captionEntry->signal_changed().connect([this, index, captionEntry]
        {
            if (index < m_photos.size())
                m_photos[index].caption = captionEntry->get_text();
        });

        auto* captionBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, CAPTION_SPACING);
        captionBox->set_hexpand(true);
        captionBox->set_valign(Gtk::Align::CENTER);
        captionBox->append(*captionLabel);
        captionBox->append(*captionEntry);

        auto* upButton = Gtk::make_managed<Gtk::Button>("▲");
        upButton->set_tooltip_text(translate("kpop.photos.moveUp"));
        upButton->set_sensitive(index > 0);
        upButton->signal_clicked().connect([this, index] { movePhoto(index, -1); });

        auto* downButton = Gtk::make_managed<Gtk::Button>("▼");
        downButton->set_tooltip_text(translate("kpop.photos.moveDown"));
        downButton->set_sensitive(index + 1 < m_photos.size());
        downButton->signal_clicked().connect([this, index] { movePhoto(index, 1); });

        auto* removeButton = Gtk::make_managed<Gtk::Button>("✕");
        removeButton->set_tooltip_text(translate("kpop.photos.removeTooltip"));
        removeButton->signal_clicked().connect([this, index] { removePhoto(index); });

        auto* buttons = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, BUTTONS_SPACING);
        buttons->set_valign(Gtk::Align::CENTER);
        buttons->append(*upButton);
        buttons->append(*downButton);
        buttons->append(*removeButton);

        auto* row = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, ROW_SPACING);
        row->set_margin(ROW_MARGIN);
        row->append(*photoSlot);
        row->append(*captionBox);
        row->append(*buttons);

        auto* card = Gtk::make_managed<Gtk::Frame>();
        card->set_child(*row);
        return card;
    }

    void showPhotoGalleryDialog(
        Gtk::Window& parent,
        image::ImageLibrary& library,
        const std::vector<domain::ItemPhoto>& photos,
        std::function<void(std::vector<domain::ItemPhoto>)> onAccept)
    {
        showAutoDeletingDialog<PhotoGalleryDialog>(parent, library, photos, std::move(onAccept));
    }
}
