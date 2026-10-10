#pragma once

#include "stapik/domain/CategoryColor.hpp"

#include <cstddef>

namespace kpop::domain
{
    [[nodiscard]] stapik::domain::CategoryColor artistColor(std::size_t artistIndex);
    [[nodiscard]] std::size_t artistColorCount();
}
