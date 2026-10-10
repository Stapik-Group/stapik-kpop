#pragma once

#include "kpop/domain/ImageId.hpp"
#include "kpop/image/ImageLibrary.hpp"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>

#include <sigc++/signal.h>

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace kpop::ui
{
    // The additional photos of an entry as a strip of thumbnails, each with a button to remove it.
    class PhotoGallery : public Gtk::Box
    {
    public:
        explicit PhotoGallery(image::ImageLibrary& library);

        void setPhotos(const std::vector<domain::ImageId>& photos);
        [[nodiscard]] const std::vector<domain::ImageId>& photos() const;

        sigc::signal<void()>& signalChanged();

    private:
        void onAddClicked();
        void onFileChosen(const std::filesystem::path& file);
        void onRemoveRequested(std::size_t index);
        void rebuild();
        void showImportError(const std::string& reason);
        [[nodiscard]] Gtk::Widget* createThumbnail(std::size_t index);

        image::ImageLibrary& m_library;
        std::vector<domain::ImageId> m_photos;

        Gtk::ScrolledWindow m_scroller;
        Gtk::Box m_strip;
        Gtk::Label m_statusLabel;
        Gtk::Button m_addButton;
        sigc::signal<void()> m_signalChanged;
    };
}
