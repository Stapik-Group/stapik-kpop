#include "ImageSynchronizer.hpp"

#include "stapik/cloud/CloudAssetProtocol.hpp"

#include <algorithm>
#include <exception>
#include <set>

namespace kpop::image
{
    namespace
    {
        using IdSet = std::set<domain::ImageId>;

        std::vector<domain::ImageId> idsOfIntactFiles(const std::vector<stapik::cloud::AssetInfo>& assets)
        {
            std::vector<domain::ImageId> ids;
            for (const auto& asset : assets)
            {
                const auto fileId = ImageStore::idFromFileName(asset.filename);
                const auto checksumId = domain::ImageId::parse(asset.checksumSha256);

                if (fileId && checksumId && *fileId == *checksumId)
                    ids.push_back(*fileId);
            }

            return ids;
        }

        void upload(stapik::cloud::IAssetStorage& storage, const ImageStore& store, const domain::ImageId& id, ImageSyncReport& report)
        {
            const auto content = store.read(id);
            if (!content)
            {
                ++report.skipped;
                return;
            }

            try
            {
                static_cast<void>(storage.uploadAsset(ImageStore::fileName(id), std::string(ImageStore::MIME_TYPE), *content));
                ++report.uploaded;
            }
            catch (const stapik::cloud::AssetTooLargeException&)
            {
                ++report.skipped;
            }
        }

        void download(stapik::cloud::IAssetStorage& storage, ImageStore& store, const domain::ImageId& id, ImageSyncReport& report)
        {
            const auto content = storage.downloadAsset(ImageStore::fileName(id));
            if (!content || content->empty() || stapik::cloud::sha256Hex(*content) != id.hash())
            {
                ++report.skipped;
                return;
            }

            static_cast<void>(store.add(*content));
            ++report.downloaded;
        }
    }

    ImageSyncPlan planImageSync(
        const std::vector<domain::ImageId>& referenced,
        const std::vector<domain::ImageId>& local,
        const std::vector<domain::ImageId>& remote)
    {
        const IdSet localIds(local.begin(), local.end());
        const IdSet remoteIds(remote.begin(), remote.end());
        const IdSet wanted(referenced.begin(), referenced.end());

        ImageSyncPlan plan;
        for (const auto& id : wanted)
        {
            const bool isLocal = localIds.contains(id);
            const bool isRemote = remoteIds.contains(id);

            if (isLocal && !isRemote)
                plan.toUpload.push_back(id);
            else if (!isLocal && isRemote)
                plan.toDownload.push_back(id);
        }

        return plan;
    }

    ImageSyncReport synchronizeImages(stapik::cloud::IAssetStorage& storage, ImageStore& store, const std::vector<domain::ImageId>& referenced)
    {
        ImageSyncReport report;

        try
        {
            const auto remote = idsOfIntactFiles(storage.listAssets());

            std::vector<domain::ImageId> local;
            for (const auto& id : referenced)
            {
                if (store.contains(id))
                    local.push_back(id);
            }

            const auto [toUpload, toDownload] = planImageSync(referenced, local, remote);

            for (const auto& id : toUpload)
                upload(storage, store, id, report);

            for (const auto& id : toDownload)
                download(storage, store, id, report);
        }
        catch (const std::exception& error)
        {
            report.failed = true;
            report.failure = error.what();
        }

        return report;
    }
}
