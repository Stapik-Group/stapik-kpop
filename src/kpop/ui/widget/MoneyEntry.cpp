#include "MoneyEntry.hpp"

#include "stapik/domain/CurrencyCatalog.hpp"

#include <glibmm/ustring.h>
#include <gtkmm/stringlist.h>

#include <algorithm>

namespace kpop::ui
{
    namespace
    {
        constexpr int SPACING = 6;
    }

    MoneyEntry::MoneyEntry() :
        Box(Gtk::Orientation::HORIZONTAL, SPACING),
        m_currencies(stapik::domain::CurrencyCatalog::instance().currencies())
    {
        m_amountEntry.set_hexpand(true);

        rebuildCurrencyModel();
        applySelectedCurrency();

        append(m_amountEntry);
        append(m_currencyDropDown);

        // The field validates its own text and shows it; this widget only passes the changes on.
        m_amountEntry.signalAmountChanged().connect([this] { m_signalChanged.emit(); });

        m_currencyDropDown.property_selected().signal_changed().connect([this]
        {
            applySelectedCurrency();
            m_signalChanged.emit();
        });
    }

    void MoneyEntry::setValue(const std::optional<stapik::domain::Money>& money)
    {
        if (!money)
        {
            m_amountEntry.setAmount(std::nullopt);
            return;
        }

        selectCurrency(money->currency());
        m_amountEntry.setAmount(money->minorUnits());
    }

    std::optional<stapik::domain::Money> MoneyEntry::value() const
    {
        return m_amountEntry.money();
    }

    bool MoneyEntry::isValid() const
    {
        return m_amountEntry.hasValidInput();
    }

    sigc::signal<void()>& MoneyEntry::signalChanged()
    {
        return m_signalChanged;
    }

    const stapik::domain::Currency* MoneyEntry::selectedCurrency() const
    {
        const auto position = m_currencyDropDown.get_selected();
        return position < m_currencies.size() ? &m_currencies[position] : nullptr;
    }

    void MoneyEntry::selectCurrency(const stapik::domain::Currency& currency)
    {
        auto position = std::ranges::find(m_currencies, currency.code, &stapik::domain::Currency::code);
        if (position == m_currencies.end())
        {
            // A currency stored by another device or version is kept instead of silently replaced.
            m_currencies.push_back(currency);
            rebuildCurrencyModel();
            position = m_currencies.end() - 1;
        }

        m_currencyDropDown.set_selected(static_cast<guint>(position - m_currencies.begin()));
        applySelectedCurrency();
    }

    void MoneyEntry::rebuildCurrencyModel()
    {
        std::vector<Glib::ustring> labels;
        for (const auto& currency : m_currencies)
            labels.emplace_back(currency.code);

        m_currencyDropDown.set_model(Gtk::StringList::create(labels));
    }

    void MoneyEntry::applySelectedCurrency()
    {
        if (const auto* currency = selectedCurrency())
            m_amountEntry.setCurrency(*currency);
    }
}
