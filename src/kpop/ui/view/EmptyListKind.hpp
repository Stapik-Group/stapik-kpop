#pragma once

#include "stapik/ui/widget/EmptyState.hpp"

#include <optional>

namespace kpop::ui
{
    // Why a list has no rows: there is nothing in the collection yet, or the filters hide everything.
    enum class EmptyListKind
    {
        NoEntries,
        NoMatches
    };

    // Fills the widget with the texts of the reason. Only the first one offers to add an entry.
    inline void showEmptyListKind(::EmptyState& state, const EmptyListKind kind)
    {
        if (kind == EmptyListKind::NoEntries)
        {
            state.setIcon(::EmptyState::DEFAULT_ICON);
            state.setTitleKey("kpop.list.empty.title");
            state.setDescriptionKey("kpop.list.empty.hint");
            state.setActionKey("kpop.button.addEntry");
            return;
        }

        state.setIcon("edit-find-symbolic");
        state.setTitleKey("kpop.list.noMatches.title");
        state.setDescriptionKey("kpop.list.noMatches.hint");
        state.setActionKey(std::nullopt);
    }
}
