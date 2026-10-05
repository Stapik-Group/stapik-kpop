#pragma once

#include "stapik/locale/LocaleManager.hpp"
#include "stapik/text/Trim.hpp"

#include <gtkmm/editable.h>

#include <string>
#include <string_view>

namespace kpop::ui
{
    [[nodiscard]] inline std::string translate(const std::string_view key)
    {
        return LocaleManager::instance().translate(key);
    }

    [[nodiscard]] inline std::string translate(const std::string_view key, const LocalizationEngine::Arguments& arguments)
    {
        return LocaleManager::instance().translate(key, arguments);
    }

    [[nodiscard]] inline std::string trimmedText(const Gtk::Editable& editable)
    {
        return stapik::text::trim(editable.get_text().raw());
    }
}
