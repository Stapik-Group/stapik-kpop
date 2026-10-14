#include "ImageArchiveStorage.hpp"

#include "stapik/cloud/CloudAssetProtocol.hpp"

#include <chrono>

namespace kpop::image
{
    ImageArchiveStorage::ImageArchiveStorage(ImageStore& covers, ImageStore& photos) :
        m_covers(covers),
        m_photos(photos)
    {}

    std::string ImageArchiveStorage::archiveName(const std::string_view prefix, const domain::ImageId& id)
    {
        return std::string(prefix) + ImageStore::fileName(id);
    }

    ImageArchiveStorage::Target ImageArchiveStorage::resolve(const std::string& filename) const
    {
        const auto parse = [&filename](const std::string_view prefix) -> std::optional<domain::ImageId>
        {
            if (!std::string_view(filename).starts_with(prefix))
                return std::nullopt;

            return ImageStore::idFromFileName(std::string_view(filename).substr(prefix.size()));
        };

        if (const auto id = parse(COVERS_PREFIX))
            return Target{ m_covers, *id };

        if (const auto id = parse(PHOTOS_PREFIX))
            return Target{ m_photos, *id };

        throw stapik::cloud::InvalidAssetFilenameException("\"" + filename + "\" is not the name of a cover or a photo");
    }

    void ImageArchiveStorage::listStore(const std::string_view prefix, const ImageStore& store, std::vector<stapik::cloud::AssetInfo>& assets) const
    {
        for (const auto& id : store.ids())
        {
            // A file whose content does not match its name is damaged and is left out of the archive.
            const auto content = store.read(id);
            if (!content)
                continue;

            assets.push_back(stapik::cloud::AssetInfo{
                .filename = archiveName(prefix, id),
                .mimeType = std::string(ImageStore::MIME_TYPE),
                .sizeBytes = static_cast<std::int64_t>(content->size()),
                .checksumSha256 = id.hash(),
                .updatedAt = std::chrono::system_clock::now() });
        }
    }

    std::vector<stapik::cloud::AssetInfo> ImageArchiveStorage::listAssets() const
    {
        std::vector<stapik::cloud::AssetInfo> assets;
        listStore(COVERS_PREFIX, m_covers, assets);
        listStore(PHOTOS_PREFIX, m_photos, assets);
        return assets;
    }

    std::optional<stapik::cloud::AssetBytes> ImageArchiveStorage::downloadAsset(const std::string& filename) const
    {
        const auto target = resolve(filename);
        return target.store.read(target.id);
    }

    stapik::cloud::AssetInfo ImageArchiveStorage::uploadAsset(
        const std::string& filename,
        const std::string& mimeType,
        const stapik::cloud::AssetBytes& content,
        const stapik::cloud::AssetUploadHooks& /*hooks*/) const
    {
        const auto target = resolve(filename);
        if (content.empty() || stapik::cloud::sha256Hex(content) != target.id.hash())
            throw ::CloudStorageException("The content of \"" + filename + "\" does not match its name");

        static_cast<void>(target.store.add(content));

        return stapik::cloud::AssetInfo{
            .filename = filename,
            .mimeType = mimeType,
            .sizeBytes = static_cast<std::int64_t>(content.size()),
            .checksumSha256 = target.id.hash(),
            .updatedAt = std::chrono::system_clock::now() };
    }

    bool ImageArchiveStorage::deleteAsset(const std::string& filename) const
    {
        const auto target = resolve(filename);
        return target.store.remove(target.id);
    }
}
