#pragma once

#include "kpop/domain/CollectionStatistics.hpp"

#include <gtkmm/label.h>
#include <gtkmm/listbox.h>
#include <gtkmm/scrolledwindow.h>

#include <sigc++/signal.h>

#include <array>
#include <optional>

namespace kpop::ui
{
    class KindSidebar : public Gtk::ScrolledWindow
    {
    public:
        KindSidebar();

        void setStatistics(const domain::CollectionStatistics& statistics) const;
        void refreshLabels() const;

        [[nodiscard]] std::optional<domain::ItemKind> selectedKind() const;
        void selectKind(std::optional<domain::ItemKind> kind);

        sigc::signal<void(std::optional<domain::ItemKind>)>& signalKindSelected();

    private:
        struct RowLabels
        {
            Gtk::Label* name = nullptr;
            Gtk::Label* count = nullptr;
        };

        static constexpr std::size_t ALL_ROW_COUNT = 1;
        static constexpr std::size_t ROW_COUNT = ALL_ROW_COUNT + domain::ITEM_KIND_COUNT;

        void appendRow(std::optional<domain::ItemKind> kind, std::size_t rowIndex);
        [[nodiscard]] static std::string nameOfRow(std::size_t rowIndex);

        Gtk::ListBox m_listBox;
        std::array<RowLabels, ROW_COUNT> m_rows;
        bool m_updatingSelection = false;
        sigc::signal<void(std::optional<domain::ItemKind>)> m_signalKindSelected;
    };
}
