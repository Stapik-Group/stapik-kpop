#include "ImageStore.hpp"

#include "stapik/cloud/CloudAssetProtocol.hpp"
#include "stapik/storage/AtomicFile.hpp"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <system_error>
#include <utility>

namespace kpop::image
{
    namespace
    {
        constexpr std::string_view FILE_EXTENSION = ".jpg";
    }

    ImageStore::ImageStore(std::filesystem::path directory) :
        m_directory(std::move(directory))
    {}

    std::string ImageStore::fileName(const domain::ImageId& id)
    {
        return id.hash() + std::string(FILE_EXTENSION);
    }

    std::optional<domain::ImageId> ImageStore::idFromFileName(const std::string_view name)
    {
        if (!name.ends_with(FILE_EXTENSION))
            return std::nullopt;

        const auto id = domain::ImageId::parse(name.substr(0, name.size() - FILE_EXTENSION.size()));
        if (!id || fileName(*id) != name)
            return std::nullopt;

        return id;
    }

    std::filesystem::path ImageStore::pathOf(const domain::ImageId& id) const
    {
        return m_directory / fileName(id);
    }

    domain::ImageId ImageStore::add(const ImageBytes& content)
    {
        if (content.empty())
            throw std::invalid_argument("An image cannot be empty");

        const auto id = domain::ImageId::parse(stapik::cloud::sha256Hex(content));
        if (!id)
            throw std::logic_error("SHA-256 did not produce a valid image id");

        if (read(*id))
            return *id;

        std::error_code errorCode;
        std::filesystem::create_directories(m_directory, errorCode);

        if (const std::string_view bytes(reinterpret_cast<const char*>(content.data()), content.size()); !stapik::storage::writeFileAtomically(pathOf(*id), bytes))
            throw std::runtime_error("Cannot save the image " + fileName(*id));

        return *id;
    }

    bool ImageStore::contains(const domain::ImageId& id) const
    {
        std::error_code errorCode;
        return std::filesystem::is_regular_file(pathOf(id), errorCode);
    }

    std::optional<ImageBytes> ImageStore::read(const domain::ImageId& id) const
    {
        std::ifstream file(pathOf(id), std::ios::binary);
        if (!file)
            return std::nullopt;

        ImageBytes content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        if (stapik::cloud::sha256Hex(content) != id.hash())
            return std::nullopt;

        return content;
    }

    std::vector<domain::ImageId> ImageStore::ids() const
    {
        std::vector<domain::ImageId> result;

        std::error_code errorCode;
        for (std::filesystem::directory_iterator entry(m_directory, errorCode), end; !errorCode && entry != end; entry.increment(errorCode))
        {
            if (std::error_code entryError; !entry->is_regular_file(entryError))
                continue;

            if (const auto id = idFromFileName(entry->path().filename().string()))
                result.push_back(*id);
        }

        std::ranges::sort(result);
        return result;
    }

    bool ImageStore::remove(const domain::ImageId& id)
    {
        std::error_code errorCode;
        return std::filesystem::remove(pathOf(id), errorCode);
    }
}
