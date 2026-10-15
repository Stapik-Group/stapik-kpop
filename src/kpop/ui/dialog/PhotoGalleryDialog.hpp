#pragma once

#include "kpop/domain/CollectionItem.hpp"
#include "kpop/image/ImageLibrary.hpp"

#include "stapik/ui/dialog/StapikDialog.hpp"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/window.h>

#include <cstddef>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace kpop::ui
{
    // Imports the files into the library and appends them to the photos. A picture that is already there is not added
    // again. Returns whether at least one file was imported; failure gets the reason of the last file that was refused.
    [[nodiscard]] bool importPhotos(
        image::ImageLibrary& library,
        const std::vector<std::filesystem::path>& files,
        std::vector<domain::ItemPhoto>& photos,
        std::string& failure);

    // The additional photos of an entry in a window of their own: every photo is shown large, with a field for its
    // caption, and can be moved or removed. Nothing changes in the entry until the window is accepted.
    class PhotoGalleryDialog : public StapikDialog
    {
    public:
        PhotoGalleryDialog(
            Gtk::Window& parent,
            image::ImageLibrary& library,
            const std::vector<domain::ItemPhoto>& photos,
            std::function<void(std::vector<domain::ItemPhoto>)> onAccept);

    private:
        void onAddClicked();
        void onFilesChosen(const std::vector<std::filesystem::path>& files);
        void movePhoto(std::size_t index, int offset);
        void removePhoto(std::size_t index);
        void rebuild();
        void showImportError(const std::string& reason);
        [[nodiscard]] Gtk::Widget* createRow(std::size_t index);

        image::ImageLibrary& m_library;
        std::vector<domain::ItemPhoto> m_photos;

        Gtk::ScrolledWindow m_scroller;
        Gtk::Box m_list;
        Gtk::Label m_statusLabel;
        Gtk::Button m_addButton;
    };

    void showPhotoGalleryDialog(
        Gtk::Window& parent,
        image::ImageLibrary& library,
        const std::vector<domain::ItemPhoto>& photos,
        std::function<void(std::vector<domain::ItemPhoto>)> onAccept);
}
