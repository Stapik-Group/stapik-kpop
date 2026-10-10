#include "ImagePicker.hpp"

#include "ImageFileDialog.hpp"
#include "ImageTexture.hpp"

#include "kpop/image/ImageEncoder.hpp"
#include "kpop/ui/Translate.hpp"

#include <gtkmm/window.h>

#include <stdexcept>

namespace kpop::ui
{
    namespace
    {
        constexpr int PREVIEW_SIZE = 160;
        constexpr int SPACING = 12;
        constexpr int BUTTONS_SPACING = 6;
    }

    ImagePicker::ImagePicker(image::ImageLibrary& library) :
        Gtk::Box(Gtk::Orientation::HORIZONTAL, SPACING),
        m_library(library),
        m_chooseButton(translate("kpop.image.choose")),
        m_removeButton(translate("kpop.image.remove"))
    {
        m_preview.set_size_request(PREVIEW_SIZE, PREVIEW_SIZE);
        m_preview.set_content_fit(Gtk::ContentFit::CONTAIN);
        m_previewFrame.set_child(m_preview);
        m_previewFrame.set_valign(Gtk::Align::START);

        m_statusLabel.set_xalign(0.0F);
        m_statusLabel.set_wrap(true);
        m_statusLabel.set_max_width_chars(32);

        auto* buttons = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, BUTTONS_SPACING);
        buttons->append(m_chooseButton);
        buttons->append(m_removeButton);

        auto* controls = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, BUTTONS_SPACING);
        controls->set_valign(Gtk::Align::START);
        controls->append(m_statusLabel);
        controls->append(*buttons);

        append(m_previewFrame);
        append(*controls);

        m_chooseButton.signal_clicked().connect([this] { onChooseClicked(); });
        m_removeButton.signal_clicked().connect([this] { onRemoveClicked(); });

        refresh();
    }

    void ImagePicker::setImage(const std::optional<domain::ImageId>& image)
    {
        m_image = image;
        refresh();
    }

    std::optional<domain::ImageId> ImagePicker::image() const
    {
        return m_image;
    }

    sigc::signal<void()>& ImagePicker::signalChanged()
    {
        return m_signalChanged;
    }

    void ImagePicker::onChooseClicked()
    {
        auto* window = dynamic_cast<Gtk::Window*>(get_root());
        if (window == nullptr)
            return;

        chooseImageFile(
            *window,
            translate("kpop.image.dialogTitle"),
            [this](const std::filesystem::path& file) { onFileChosen(file); },
            [this](const std::string& reason) { showImportError(reason); });
    }

    void ImagePicker::onFileChosen(const std::filesystem::path& file)
    {
        try
        {
            m_image = m_library.importFile(file);
        }
        catch (const std::runtime_error& error)
        {
            showImportError(error.what());
            return;
        }

        refresh();
        m_signalChanged.emit();
    }

    void ImagePicker::onRemoveClicked()
    {
        m_image.reset();
        refresh();
        m_signalChanged.emit();
    }

    void ImagePicker::refresh()
    {
        const auto path = m_image ? m_library.pathOf(*m_image) : std::nullopt;
        const Glib::RefPtr<Gdk::Paintable> texture = path ? loadTexture(*path, PREVIEW_SIZE) : Glib::RefPtr<Gdk::Texture>();

        m_preview.set_paintable(texture);
        m_removeButton.set_sensitive(m_image.has_value());

        m_statusLabel.remove_css_class("error");
        if (!m_image)
            m_statusLabel.set_text(translate("kpop.image.none"));
        else if (!texture)
            m_statusLabel.set_text(translate("kpop.image.missing"));
        else
            m_statusLabel.set_text({});
    }

    void ImagePicker::showImportError(const std::string& reason)
    {
        m_statusLabel.add_css_class("error");
        m_statusLabel.set_text(translate("kpop.image.importFailed", { { "reason", reason } }));
    }
}
