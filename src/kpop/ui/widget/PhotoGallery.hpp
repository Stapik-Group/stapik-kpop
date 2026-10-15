#pragma once

#include "kpop/domain/CollectionItem.hpp"
#include "kpop/image/ImageLibrary.hpp"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/overlay.h>
#include <gtkmm/scrolledwindow.h>

#include <sigc++/signal.h>

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace kpop::ui
{
    // The additional photos of an entry: a read-only strip of thumbnails and a summary with a button. Clicking the
    // strip or the button opens the gallery window, where the photos and their captions are edited. Image files dropped
    // on the widget are added directly.
    class PhotoGallery : public Gtk::Box
    {
    public:
        explicit PhotoGallery(image::ImageLibrary& library);

        void setPhotos(const std::vector<domain::ItemPhoto>& photos);
        [[nodiscard]] const std::vector<domain::ItemPhoto>& photos() const;

        sigc::signal<void()>& signalChanged();

    private:
        void openGallery();
        void onFilesDropped(const std::vector<std::filesystem::path>& files);
        void rebuild();
        void showImportError(const std::string& reason);
        [[nodiscard]] Gtk::Widget* createThumbnail(std::size_t index);

        image::ImageLibrary& m_library;
        std::vector<domain::ItemPhoto> m_photos;

        Gtk::Box m_summaryRow;
        Gtk::Label m_summaryLabel;
        Gtk::Button m_openButton;
        Gtk::Overlay m_stripOverlay;
        Gtk::ScrolledWindow m_scroller;
        Gtk::Box m_strip;
        Gtk::Label m_openHint;
        sigc::signal<void()> m_signalChanged;
    };
}
