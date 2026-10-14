#pragma once

#include "kpop/domain/Tracklist.hpp"
#include "kpop/ui/widget/TracklistEditor.hpp"

#include "stapik/ui/dialog/StapikDialog.hpp"

#include <gtkmm/window.h>

#include <functional>

namespace kpop::ui
{
    // The tracklist of an album in a window of its own, which leaves room for long lists and several discs.
    class TracklistDialog : public StapikDialog
    {
    public:
        TracklistDialog(Gtk::Window& parent, const domain::Tracklist& tracklist, std::function<void(domain::Tracklist)> onAccept);

    private:
        void updateConfirmSensitivity();

        TracklistEditor m_editor;
    };

    void showTracklistDialog(Gtk::Window& parent, const domain::Tracklist& tracklist, std::function<void(domain::Tracklist)> onAccept);
}
