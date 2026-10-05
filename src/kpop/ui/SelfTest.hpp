#pragma once

#include "kpop/app/CollectionController.hpp"

#include <gtkmm/window.h>

namespace kpop::ui
{
    // Checks that everything a packaged build needs at run time is really there: a mapped window, the icons GTK draws
    // its own widgets with, the compiled GSettings schemas, the application resources with their translations and a
    // writable data directory. Every check is logged. The SVG image loader of gdk-pixbuf is only reported.
    //
    // Run by `--self-test`, which the CI of the Windows build starts on the unpacked package with nothing but the
    // Windows directories in PATH. Returns the number of failed checks.
    [[nodiscard]] int runSelfTest(Gtk::Window& window, const app::CollectionController& controller);
}
