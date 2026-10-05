#include "MoneyEntry.hpp"

#include "kpop/ui/Translate.hpp"

#include "stapik/domain/CurrencyCatalog.hpp"
#include "stapik/domain/MoneyFormatter.hpp"

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
        m_amountEntry.set_placeholder_text("0,00");
        m_amountEntry.set_input_purpose(Gtk::InputPurpose::NUMBER);

        rebuildCurrencyModel();

        append(m_amountEntry);
        append(m_currencyDropDown);

        m_amountEntry.signal_changed().connect([this]
        {
            refreshValidityStyle();
            m_signalChanged.emit();
        });

        m_currencyDropDown.property_selected().signal_changed().connect([this]
        {
            refreshValidityStyle();
            m_signalChanged.emit();
        });
    }

    void MoneyEntry::setValue(const std::optional<stapik::domain::Money>& money)
    {
        if (!money)
        {
            m_amountEntry.set_text("");
            return;
        }

        selectCurrency(money->currency());

        m_amountEntry.set_text(stapik::domain::formatAmount(
            money->minorUnits(),
            money->currency().decimalPlaces,
            LocaleManager::instance().languageCode(),
            false));
    }

    std::optional<stapik::domain::Money> MoneyEntry::value() const
    {
        const auto* currency = selectedCurrency();
        const auto minorUnits = parsedMinorUnits();
        if (currency == nullptr || !minorUnits)
            return std::nullopt;

        return stapik::domain::Money(*minorUnits, *currency);
    }

    bool MoneyEntry::isValid() const
    {
        return trimmedText(m_amountEntry).empty() || value().has_value();
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

    std::optional<std::int64_t> MoneyEntry::parsedMinorUnits() const
    {
        const auto* currency = selectedCurrency();
        if (currency == nullptr)
            return std::nullopt;

        const auto text = trimmedText(m_amountEntry);
        if (text.empty())
            return std::nullopt;

        return stapik::domain::parseAmount(text, currency->decimalPlaces);
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
    }

    void MoneyEntry::rebuildCurrencyModel()
    {
        std::vector<Glib::ustring> labels;
        for (const auto& currency : m_currencies)
            labels.emplace_back(currency.code);

        m_currencyDropDown.set_model(Gtk::StringList::create(labels));
    }

    void MoneyEntry::refreshValidityStyle()
    {
        if (isValid())
            m_amountEntry.remove_css_class("kpop-invalid");
        else
            m_amountEntry.add_css_class("kpop-invalid");
    }
}
