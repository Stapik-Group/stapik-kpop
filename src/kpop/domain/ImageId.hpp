#pragma once

#include <compare>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace kpop::domain
{
    class ImageId
    {
    public:
        static constexpr std::size_t HASH_LENGTH = 64;
        [[nodiscard]] static std::optional<ImageId> parse(std::string_view hash);
        [[nodiscard]] const std::string& hash() const;
        [[nodiscard]] std::strong_ordering operator<=>(const ImageId& other) const = default;
        [[nodiscard]] bool operator==(const ImageId& other) const = default;
    private:
        explicit ImageId(std::string hash);

        std::string m_hash;
    };
}
