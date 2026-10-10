#pragma once

#include <filesystem>
#include <functional>
#include <string>

namespace Gtk
{
    class Window;
}

namespace kpop::ui
{
    // Lets the user choose one image file. Nothing is called when the dialog is closed without a choice;
    // onError gets the reason when the dialog itself failed.
    void chooseImageFile(
        Gtk::Window& parent,
        const std::string& title,
        std::function<void(const std::filesystem::path&)> onChosen,
        std::function<void(const std::string&)> onError);
}
