#pragma once

#include "ImageStore.hpp"

#include "stapik/cloud/IAssetStorage.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace kpop::image
{
    // Presents the covers and the photos kept on this computer as one storage of files, which is what the archive of
    // the whole application works on. Both libraries are stored in one list, so the file names start with the library
    // ("covers-<hash>.jpg", "photos-<hash>.jpg"). The archive is therefore complete without a cloud connection, and
    // after a restore the usual image synchronisation sends what the cloud is missing.
    class ImageArchiveStorage final : public stapik::cloud::IAssetStorage
    {
    public:
        static constexpr std::string_view COVERS_PREFIX = "covers-";
        static constexpr std::string_view PHOTOS_PREFIX = "photos-";

        ImageArchiveStorage(ImageStore& covers, ImageStore& photos);

        [[nodiscard]] std::vector<stapik::cloud::AssetInfo> listAssets() const override;
        [[nodiscard]] std::optional<stapik::cloud::AssetBytes> downloadAsset(const std::string& filename) const override;

        using stapik::cloud::IAssetStorage::uploadAsset;

        // The content has to be what the file name says it is: the images are named after their SHA-256.
        stapik::cloud::AssetInfo uploadAsset(
            const std::string& filename,
            const std::string& mimeType,
            const stapik::cloud::AssetBytes& content,
            const stapik::cloud::AssetUploadHooks& hooks) const override;

        [[nodiscard]] bool deleteAsset(const std::string& filename) const override;

    private:
        struct Target
        {
            ImageStore& store;
            domain::ImageId id;
        };

        [[nodiscard]] Target resolve(const std::string& filename) const;
        [[nodiscard]] static std::string archiveName(std::string_view prefix, const domain::ImageId& id);
        void listStore(std::string_view prefix, const ImageStore& store, std::vector<stapik::cloud::AssetInfo>& assets) const;

        ImageStore& m_covers;
        ImageStore& m_photos;
    };
}
