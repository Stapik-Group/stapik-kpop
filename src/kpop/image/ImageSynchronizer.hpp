#pragma once

#include "ImageStore.hpp"

#include "stapik/cloud/IAssetStorage.hpp"

#include <string>
#include <vector>

namespace kpop::image
{
    struct ImageSyncPlan
    {
        std::vector<domain::ImageId> toUpload;
        std::vector<domain::ImageId> toDownload;

        bool operator==(const ImageSyncPlan&) const = default;
    };

    [[nodiscard]] ImageSyncPlan planImageSync(
        const std::vector<domain::ImageId>& referenced,
        const std::vector<domain::ImageId>& local,
        const std::vector<domain::ImageId>& remote);

    struct ImageSyncReport
    {
        int uploaded = 0;
        int downloaded = 0;

        int skipped = 0;

        bool failed = false;
        std::string failure;
    };

    [[nodiscard]] ImageSyncReport synchronizeImages(
        stapik::cloud::IAssetStorage& storage,
        ImageStore& store,
        const std::vector<domain::ImageId>& referenced);
}
