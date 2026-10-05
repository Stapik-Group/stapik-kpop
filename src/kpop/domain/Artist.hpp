#pragma once

#include "Enumerations.hpp"

#include <string>
#include <vector>

namespace kpop::domain
{
    struct Artist
    {
        std::string id;
        std::string name;
        ArtistType type = ArtistType::Group;
        std::vector<std::string> members;

        bool operator==(const Artist&) const = default;
    };
}
