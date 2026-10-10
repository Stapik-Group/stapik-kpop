#pragma once

#include "PartialDate.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace kpop::domain
{
    // Dates the way people of a language write them: "23.08.2021" (pl, de), "08/23/2021" (en). A month is
    // "08.2021" and a year just "2021". Any other language gets the ISO form "2021-08-23".
    [[nodiscard]] std::string formatPartialDate(const PartialDate& date, std::string_view languageCode);

    // Reads what formatPartialDate writes. The ISO form is understood in every language.
    [[nodiscard]] std::optional<PartialDate> parsePartialDate(std::string_view text, std::string_view languageCode);
}
