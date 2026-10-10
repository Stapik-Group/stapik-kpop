#include "PhotoGallery.hpp"

#include "ImageFileDialog.hpp"
#include "ImageTexture.hpp"

#include "kpop/ui/Translate.hpp"

#include <gtkmm/frame.h>
#include <gtkmm/overlay.h>
#include <gtkmm/picture.h>
#include <gtkmm/window.h>

#include <algorithm>
#include <optional>
#include <stdexcept>

namespace kpop::ui
{
    namespace
    {
        constexpr int GALLERY_SPACING = 6;
        constexpr int STRIP_SPACING = 8;
        constexpr int THUMBNAIL_SIZE = 120;
        constexpr int REMOVE_BUTTON_MARGIN = 4;
        constexpr int STRIP_HEIGHT = THUMBNAIL_SIZE + 24;
    }

    PhotoGallery::PhotoGallery(image::ImageLibrary& library) :
        Gtk::Box(Gtk::Orientation::VERTICAL, GALLERY_SPACING),
        m_library(library),
        m_strip(Gtk::Orientation::HORIZONTAL, STRIP_SPACING),
        m_addButton(translate("kpop.photos.add"))
    {
        m_scroller.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::NEVER);
        m_scroller.set_min_content_height(STRIP_HEIGHT);
        m_scroller.set_child(m_strip);

        m_statusLabel.set_xalign(0.0F);
        m_statusLabel.set_wrap(true);

        m_addButton.set_halign(Gtk::Align::START);
        m_addButton.signal_clicked().connect([this] { onAddClicked(); });

        append(m_scroller);
        append(m_statusLabel);
        append(m_addButton);

        rebuild();
    }

    void PhotoGallery::setPhotos(const std::vector<domain::ImageId>& photos)
    {
        m_photos = photos;
        rebuild();
    }

    const std::vector<domain::ImageId>& PhotoGallery::photos() const
    {
        return m_photos;
    }

    sigc::signal<void()>& PhotoGallery::signalChanged()
    {
        return m_signalChanged;
    }

    void PhotoGallery::onAddClicked()
    {
        auto* window = dynamic_cast<Gtk::Window*>(get_root());
        if (window == nullptr)
            return;

        chooseImageFile(
            *window,
            translate("kpop.photos.dialogTitle"),
            [this](const std::filesystem::path& file) { onFileChosen(file); },
            [this](const std::string& reason) { showImportError(reason); });
    }

    void PhotoGallery::onFileChosen(const std::filesystem::path& file)
    {
        std::optional<domain::ImageId> id;

        try
        {
            id = m_library.importFile(file);
        }
        catch (const std::runtime_error& error)
        {
            showImportError(error.what());
            return;
        }

        // The same picture is the same image, so it is not added a second time.
        if (std::ranges::find(m_photos, *id) == m_photos.end())
            m_photos.push_back(*id);

        rebuild();
        m_signalChanged.emit();
    }

    void PhotoGallery::onRemoveRequested(const std::size_t index)
    {
        if (index >= m_photos.size())
            return;

        m_photos.erase(m_photos.begin() + static_cast<std::ptrdiff_t>(index));
        rebuild();
        m_signalChanged.emit();
    }

    void PhotoGallery::rebuild()
    {
        while (auto* child = m_strip.get_first_child())
            m_strip.remove(*child);

        for (std::size_t index = 0; index < m_photos.size(); ++index)
            m_strip.append(*createThumbnail(index));

        m_scroller.set_visible(!m_photos.empty());

        m_statusLabel.remove_css_class("error");
        m_statusLabel.set_text(m_photos.empty() ? translate("kpop.photos.none") : std::string());
        m_statusLabel.set_visible(m_photos.empty());
    }

    void PhotoGallery::showImportError(const std::string& reason)
    {
        m_statusLabel.add_css_class("error");
        m_statusLabel.set_text(translate("kpop.image.importFailed", { { "reason", reason } }));
        m_statusLabel.set_visible(true);
    }

    Gtk::Widget* PhotoGallery::createThumbnail(const std::size_t index)
    {
        const auto path = m_library.pathOf(m_photos[index]);

        // Decoded at the displayed size, which is also the natural size of the picture.
        const Glib::RefPtr<Gdk::Paintable> texture = path ? loadTexture(*path, THUMBNAIL_SIZE) : Glib::RefPtr<Gdk::Texture>();

        auto* frame = Gtk::make_managed<Gtk::Frame>();
        frame->set_valign(Gtk::Align::START);

        if (texture)
        {
            auto* picture = Gtk::make_managed<Gtk::Picture>();
            picture->set_size_request(THUMBNAIL_SIZE, THUMBNAIL_SIZE);
            picture->set_content_fit(Gtk::ContentFit::CONTAIN);
            picture->set_paintable(texture);
            frame->set_child(*picture);
        }
        else
        {
            auto* missing = Gtk::make_managed<Gtk::Label>("?");
            missing->set_size_request(THUMBNAIL_SIZE, THUMBNAIL_SIZE);
            missing->set_tooltip_text(translate("kpop.image.missing"));
            frame->set_child(*missing);
        }

        auto* removeButton = Gtk::make_managed<Gtk::Button>("✕");
        removeButton->add_css_class("osd");
        removeButton->add_css_class("circular");
        removeButton->set_halign(Gtk::Align::END);
        removeButton->set_valign(Gtk::Align::START);
        removeButton->set_margin(REMOVE_BUTTON_MARGIN);
        removeButton->set_tooltip_text(translate("kpop.photos.removeTooltip"));
        removeButton->signal_clicked().connect([this, index] { onRemoveRequested(index); });

        auto* overlay = Gtk::make_managed<Gtk::Overlay>();
        overlay->set_child(*frame);
        overlay->add_overlay(*removeButton);
        return overlay;
    }
}
