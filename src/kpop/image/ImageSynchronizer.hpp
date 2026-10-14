#pragma once

#include "ImageStore.hpp"

#include "stapik/cloud/IAssetStorage.hpp"

#include <chrono>
#include <optional>
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

        // Set when the cloud refused the requests because there were too many of them and said how long to wait.
        std::optional<std::chrono::seconds> retryAfter;
    };

    [[nodiscard]] ImageSyncReport synchronizeImages(
        stapik::cloud::IAssetStorage& storage,
        ImageStore& store,
        const std::vector<domain::ImageId>& referenced);
}
