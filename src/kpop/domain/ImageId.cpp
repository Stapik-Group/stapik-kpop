#include "ImageId.hpp"

#include <utility>

namespace kpop::domain
{
    ImageId::ImageId(std::string hash) :
        m_hash(std::move(hash))
    {}

    std::optional<ImageId> ImageId::parse(const std::string_view hash)
    {
        if (hash.size() != HASH_LENGTH)
            return std::nullopt;

        std::string normalized;
        normalized.reserve(HASH_LENGTH);

        for (const char character : hash)
        {
            if ((character >= '0' && character <= '9') || (character >= 'a' && character <= 'f'))
                normalized.push_back(character);
            else if (character >= 'A' && character <= 'F')
                normalized.push_back(static_cast<char>(character - 'A' + 'a'));
            else
                return std::nullopt;
        }

        return ImageId(std::move(normalized));
    }

    const std::string& ImageId::hash() const
    {
        return m_hash;
    }
}
