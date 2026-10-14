#pragma once

#include "stapik/domain/Currency.hpp"
#include "stapik/domain/Money.hpp"
#include "stapik/ui/widget/MoneyEntry.hpp"

#include <gtkmm/box.h>
#include <gtkmm/dropdown.h>

#include <sigc++/signal.h>

#include <optional>
#include <vector>

namespace kpop::ui
{
    // The amount field of stapik-common (::MoneyEntry) next to a list of currencies.
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
        void selectCurrency(const stapik::domain::Currency& currency);
        void rebuildCurrencyModel();
        void applySelectedCurrency();

        ::MoneyEntry m_amountEntry;
        Gtk::DropDown m_currencyDropDown;
        std::vector<stapik::domain::Currency> m_currencies;
        sigc::signal<void()> m_signalChanged;
    };
}
