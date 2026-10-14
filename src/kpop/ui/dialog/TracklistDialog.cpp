#include "TracklistDialog.hpp"

#include "kpop/ui/Translate.hpp"

#include "stapik/ui/dialog/DialogUtils.hpp"

#include <gtkmm/scrolledwindow.h>

#include <utility>

namespace kpop::ui
{
    namespace
    {
        constexpr int DIALOG_WIDTH = 980;
        constexpr int DIALOG_HEIGHT = 680;
        constexpr int EDITOR_MARGIN = 6;
    }

    TracklistDialog::TracklistDialog(Gtk::Window& parent, const domain::Tracklist& tracklist, std::function<void(domain::Tracklist)> onAccept) :
        StapikDialog(parent, translate("kpop.dialog.tracklist.title"))
    {
        set_default_size(DIALOG_WIDTH, DIALOG_HEIGHT);

        auto* scroller = Gtk::make_managed<Gtk::ScrolledWindow>();
        scroller->set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
        scroller->set_hexpand(true);
        scroller->set_vexpand(true);

        m_editor.set_margin(EDITOR_MARGIN);
        scroller->set_child(m_editor);

        // The editor rebuilds its rows on every change and keeps the scroll position through it.
        m_editor.setScrollAdjustment(scroller->get_vadjustment());

        contentBox().append(*scroller);

        m_editor.signalChanged().connect([this] { updateConfirmSensitivity(); });

        addCancelButton();
        addOkButton();

        m_editor.setValue(tracklist);

        signal_response().connect([this, onAccept = std::move(onAccept)](const int response)
        {
            if (response == static_cast<int>(Gtk::ResponseType::OK) && onAccept)
                onAccept(m_editor.value());
        });

        updateConfirmSensitivity();
    }

    void TracklistDialog::updateConfirmSensitivity()
    {
        set_response_sensitive(Gtk::ResponseType::OK, m_editor.isValid());
    }

    void showTracklistDialog(Gtk::Window& parent, const domain::Tracklist& tracklist, std::function<void(domain::Tracklist)> onAccept)
    {
        showAutoDeletingDialog<TracklistDialog>(parent, tracklist, std::move(onAccept));
    }
}
