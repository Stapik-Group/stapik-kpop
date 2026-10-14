#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace Gtk
{
    class Window;
}

namespace kpop::ui
{
    // The extensions (with the dot, in lower case) of the image files this computer can read, for example ".png".
    // The same formats are offered by the file chooser, so a dropped file is accepted exactly when it could be chosen.
    [[nodiscard]] std::vector<std::string> imageFileExtensions();

    // Lets the user choose one image file, starting in the folder of the previous choice. Nothing is called when the
    // dialog is closed without a choice; onError gets the reason when the dialog itself failed.
    void chooseImageFile(
        Gtk::Window& parent,
        const std::string& title,
        std::function<void(const std::filesystem::path&)> onChosen,
        std::function<void(const std::string&)> onError);
}
