#include "ArtistEditDialog.hpp"

#include "kpop/ui/Translate.hpp"

#include "stapik/ui/dialog/DialogUtils.hpp"

#include <sstream>
#include <utility>

namespace kpop::ui
{
    namespace
    {
        constexpr int MEMBERS_MIN_HEIGHT = 110;

        std::string joinLines(const std::vector<std::string>& lines)
        {
            std::string joined;
            for (const auto& line : lines)
            {
                if (!joined.empty())
                    joined += '\n';

                joined += line;
            }

            return joined;
        }

        std::vector<std::string> splitLines(const std::string& text)
        {
            std::vector<std::string> lines;
            std::istringstream stream(text);
            std::string line;

            while (std::getline(stream, line))
            {
                if (auto trimmed = stapik::text::trim(line); !trimmed.empty())
                    lines.push_back(std::move(trimmed));
            }

            return lines;
        }
    }

    ArtistEditDialog::ArtistEditDialog(Window& parent, const std::optional<domain::Artist>& existing, std::function<void(domain::Artist)> onAccept) :
        StapikDialog(parent, translate(existing ? "kpop.dialog.editArtist.title" : "kpop.dialog.addArtist.title")),
        m_existingId(existing ? existing->id : std::string()),
        m_typeDropDown(domain::ARTIST_TYPES),
        m_membersText(MEMBERS_MIN_HEIGHT)
    {
        m_grid.addRow("kpop.field.artistName", m_nameEntry);
        m_grid.addRow("kpop.field.artistType", m_typeDropDown);
        m_grid.addRow("kpop.field.members", m_membersText);
        contentBox().append(m_grid);

        m_nameEntry.set_activates_default(true);
        m_nameEntry.signal_changed().connect([this] { updateConfirmSensitivity(); });

        addCancelButton();
        addOkButton();

        if (existing)
        {
            m_nameEntry.set_text(existing->name);
            m_typeDropDown.setValue(existing->type);
            m_membersText.setText(joinLines(existing->members));
        }

        signal_response().connect([this, onAccept = std::move(onAccept)](const int response)
        {
            if (response == static_cast<int>(Gtk::ResponseType::OK) && onAccept)
                onAccept(collect());
        });

        updateConfirmSensitivity();
    }

    domain::Artist ArtistEditDialog::collect() const
    {
        return domain::Artist{
            .id = m_existingId,
            .name = trimmedText(m_nameEntry),
            .type = m_typeDropDown.value(),
            .members = splitLines(m_membersText.text()) };
    }

    void ArtistEditDialog::updateConfirmSensitivity()
    {
        set_response_sensitive(Gtk::ResponseType::OK, !trimmedText(m_nameEntry).empty());
    }

    void showArtistEditDialog(Gtk::Window& parent, const std::optional<domain::Artist>& existing, std::function<void(domain::Artist)> onAccept)
    {
        showAutoDeletingDialog<ArtistEditDialog>(parent, existing, std::move(onAccept));
    }
}
