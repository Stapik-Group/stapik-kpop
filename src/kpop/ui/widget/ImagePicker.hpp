#pragma once

#include "kpop/domain/ImageId.hpp"
#include "kpop/image/ImageLibrary.hpp"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/picture.h>

#include <sigc++/signal.h>

#include <filesystem>
#include <optional>
#include <string>

namespace kpop::ui
{
    class ImagePicker : public Gtk::Box
    {
    public:
        explicit ImagePicker(image::ImageLibrary& library);

        void setImage(const std::optional<domain::ImageId>& image);
        [[nodiscard]] std::optional<domain::ImageId> image() const;

        sigc::signal<void()>& signalChanged();

    private:
        void onChooseClicked();
        void onFileChosen(const std::filesystem::path& file);
        void onRemoveClicked();
        void refresh();
        void showImportError(const std::string& reason);

        image::ImageLibrary& m_library;
        std::optional<domain::ImageId> m_image;

        Gtk::Frame m_previewFrame;
        Gtk::Picture m_preview;
        Gtk::Label m_statusLabel;
        Gtk::Button m_chooseButton;
        Gtk::Button m_removeButton;
        sigc::signal<void()> m_signalChanged;
    };
}
