#pragma once

#include "stapik/domain/Currency.hpp"
#include "stapik/domain/Money.hpp"

#include <gtkmm/box.h>
#include <gtkmm/dropdown.h>
#include <gtkmm/entry.h>

#include <sigc++/signal.h>

#include <optional>
#include <vector>

namespace kpop::ui
{
    class MoneyEntry : public Gtk::Box
    {
    public:
        MoneyEntry();

        void setValue(const std::optional<stapik::domain::Money>& money);
        [[nodiscard]] std::optional<stapik::domain::Money> value() const;

        [[nodiscard]] bool isValid() const;

        sigc::signal<void()>& signalChanged();

    private:
        [[nodiscard]] const stapik::domain::Currency* selectedCurrency() const;
        [[nodiscard]] std::optional<std::int64_t> parsedMinorUnits() const;
        void selectCurrency(const stapik::domain::Currency& currency);
        void rebuildCurrencyModel();
        void refreshValidityStyle();

        Gtk::Entry m_amountEntry;
        Gtk::DropDown m_currencyDropDown;
        std::vector<stapik::domain::Currency> m_currencies;
        sigc::signal<void()> m_signalChanged;
    };
}
