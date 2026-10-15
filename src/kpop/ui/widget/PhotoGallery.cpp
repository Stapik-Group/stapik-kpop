#include "PhotoGallery.hpp"

#include "ImageFileDialog.hpp"
#include "ImageTexture.hpp"

#include "kpop/ui/Translate.hpp"
#include "kpop/ui/dialog/PhotoGalleryDialog.hpp"

#include "stapik/ui/dialog/FileDrop.hpp"

#include <gtkmm/eventcontrollermotion.h>
#include <gtkmm/frame.h>
#include <gtkmm/gestureclick.h>
#include <gtkmm/picture.h>
#include <gtkmm/window.h>

#include <gdk/gdk.h>

#include <utility>

namespace kpop::ui
{
    namespace
    {
        constexpr int GALLERY_SPACING = 6;
        constexpr int SUMMARY_SPACING = 8;
        constexpr int STRIP_SPACING = 8;
        constexpr int THUMBNAIL_WIDTH = 128;
        constexpr int THUMBNAIL_HEIGHT = 96;
        constexpr int STRIP_HEIGHT = THUMBNAIL_HEIGHT + 24;
    }

    PhotoGallery::PhotoGallery(image::ImageLibrary& library) :
        Gtk::Box(Gtk::Orientation::VERTICAL, GALLERY_SPACING),
        m_library(library),
        m_summaryRow(Gtk::Orientation::HORIZONTAL, SUMMARY_SPACING),
        m_strip(Gtk::Orientation::HORIZONTAL, STRIP_SPACING)
    {
        m_summaryLabel.set_hexpand(true);
        m_summaryLabel.set_xalign(0.0F);
        m_summaryLabel.set_wrap(true);
        m_summaryRow.append(m_summaryLabel);
        m_summaryRow.append(m_openButton);

        m_scroller.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::NEVER);
        m_scroller.set_min_content_height(STRIP_HEIGHT);
        m_scroller.set_child(m_strip);

        // The hint only appears while the pointer is over the strip and never takes the clicks itself.
        m_openHint.set_text(translate("kpop.photos.open"));
        m_openHint.add_css_class("osd");
        m_openHint.set_halign(Gtk::Align::CENTER);
        m_openHint.set_valign(Gtk::Align::CENTER);
        m_openHint.set_can_target(false);
        m_openHint.set_visible(false);

        m_stripOverlay.set_child(m_scroller);
        m_stripOverlay.add_overlay(m_openHint);
        m_stripOverlay.set_cursor("pointer");

        const auto motion = Gtk::EventControllerMotion::create();
        motion->signal_enter().connect([this](const double, const double) { m_openHint.set_visible(true); });
        motion->signal_leave().connect([this] { m_openHint.set_visible(false); });
        m_stripOverlay.add_controller(motion);

        const auto click = Gtk::GestureClick::create();
        click->set_button(GDK_BUTTON_PRIMARY);
        click->signal_released().connect([this](const int, const double, const double) { openGallery(); });
        m_stripOverlay.add_controller(click);

        m_openButton.signal_clicked().connect([this] { openGallery(); });

        stapik::ui::acceptFileDrop(
            *this,
            [this](const std::vector<std::filesystem::path>& files) { onFilesDropped(files); },
            { .extensions = imageFileExtensions(), .allowMultiple = true });

        append(m_summaryRow);
        append(m_stripOverlay);

        rebuild();
    }

    void PhotoGallery::setPhotos(const std::vector<domain::ItemPhoto>& photos)
    {
        m_photos = photos;
        rebuild();
    }

    const std::vector<domain::ItemPhoto>& PhotoGallery::photos() const
    {
        return m_photos;
    }

    sigc::signal<void()>& PhotoGallery::signalChanged()
    {
        return m_signalChanged;
    }

    void PhotoGallery::openGallery()
    {
        auto* window = dynamic_cast<Gtk::Window*>(get_root());
        if (window == nullptr)
            return;

        showPhotoGalleryDialog(*window, m_library, m_photos, [this](std::vector<domain::ItemPhoto> photos)
        {
            m_photos = std::move(photos);
            rebuild();
            m_signalChanged.emit();
        });
    }

    void PhotoGallery::onFilesDropped(const std::vector<std::filesystem::path>& files)
    {
        std::string failure;
        if (importPhotos(m_library, files, m_photos, failure))
        {
            rebuild();
            m_signalChanged.emit();
        }

        // After the rebuild, which clears the status line.
        if (!failure.empty())
            showImportError(failure);
    }

    void PhotoGallery::rebuild()
    {
        while (auto* child = m_strip.get_first_child())
            m_strip.remove(*child);

        for (std::size_t index = 0; index < m_photos.size(); ++index)
            m_strip.append(*createThumbnail(index));

        m_stripOverlay.set_visible(!m_photos.empty());

        m_summaryLabel.remove_css_class("error");
        m_summaryLabel.set_text(m_photos.empty()
            ? translate("kpop.photos.none")
            : translate("kpop.photos.summary", { { "count", std::to_string(m_photos.size()) } }));
        m_openButton.set_label(translate(m_photos.empty() ? "kpop.photos.button.add" : "kpop.photos.button.edit"));
    }

    void PhotoGallery::showImportError(const std::string& reason)
    {
        m_summaryLabel.add_css_class("error");
        m_summaryLabel.set_text(translate("kpop.image.importFailed", { { "reason", reason } }));
    }

    Gtk::Widget* PhotoGallery::createThumbnail(const std::size_t index)
    {
        const auto path = m_library.pathOf(m_photos[index].image);

        // Decoded to fit the slot and shown pixel for pixel, so the thumbnail is sharp whatever its proportions.
        const auto texture = path ? loadTexture(*path, THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT) : Glib::RefPtr<Gdk::Texture>();

        auto* frame = Gtk::make_managed<Gtk::Frame>();
        frame->set_halign(Gtk::Align::CENTER);
        frame->set_valign(Gtk::Align::CENTER);

        if (texture)
        {
            frame->set_child(*makeExactPicture(texture));
        }
        else
        {
            auto* missing = Gtk::make_managed<Gtk::Label>("?");
            missing->set_size_request(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
            missing->set_tooltip_text(translate("kpop.image.missing"));
            frame->set_child(*missing);
        }

        // The caption is what tells the photos apart, so it is shown when the pointer rests on a thumbnail.
        if (!m_photos[index].caption.empty())
            frame->set_tooltip_text(m_photos[index].caption);

        auto* slot = Gtk::make_managed<Gtk::Box>();
        slot->set_size_request(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
        slot->set_valign(Gtk::Align::START);
        frame->set_hexpand(true);
        frame->set_vexpand(true);
        slot->append(*frame);
        return slot;
    }
}
