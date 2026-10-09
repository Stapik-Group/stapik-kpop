#pragma once

#include "stapik/cloud/CloudAssetProtocol.hpp"
#include "stapik/cloud/IAssetStorage.hpp"

#include <chrono>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace kpop::test
{
    class FakeAssetStorage final : public stapik::cloud::IAssetStorage
    {
    public:
        struct File
        {
            std::string mimeType;
            stapik::cloud::AssetBytes content;

            std::optional<std::string> reportedChecksum;
        };

        mutable std::map<std::string, File> files;

        std::optional<std::size_t> maxFileSize;
        bool failListing = false;
        bool failUploads = false;
        std::set<std::string> vanishingFiles;

        mutable int uploadCalls = 0;
        mutable int downloadCalls = 0;

        [[nodiscard]] std::vector<stapik::cloud::AssetInfo> listAssets() const override
        {
            if (failListing)
                throw CloudStorageException("API asset list failed with status 500");

            std::vector<stapik::cloud::AssetInfo> assets;
            for (const auto& [filename, file] : files)
                assets.push_back(infoOf(filename, file));

            return assets;
        }

        [[nodiscard]] std::optional<stapik::cloud::AssetBytes> downloadAsset(const std::string& filename) const override
        {
            ++downloadCalls;

            const auto file = files.find(filename);
            if (file == files.end() || vanishingFiles.contains(filename))
                return std::nullopt;

            return file->second.content;
        }

        stapik::cloud::AssetInfo uploadAsset(const std::string& filename, const std::string& mimeType, const stapik::cloud::AssetBytes& content) const override
        {
            ++uploadCalls;

            if (failUploads)
                throw CloudStorageException("API asset upload failed with status 403");

            if (maxFileSize && content.size() > *maxFileSize)
                throw stapik::cloud::AssetTooLargeException("too large");

            auto& file = files[filename];
            file = File{ .mimeType = mimeType, .content = content, .reportedChecksum = std::nullopt };
            return infoOf(filename, file);
        }

        [[nodiscard]] bool deleteAsset(const std::string& filename) const override
        {
            return files.erase(filename) > 0;
        }

    private:
        [[nodiscard]] static stapik::cloud::AssetInfo infoOf(const std::string& filename, const File& file)
        {
            return stapik::cloud::AssetInfo{
                .filename = filename,
                .mimeType = file.mimeType,
                .sizeBytes = static_cast<std::int64_t>(file.content.size()),
                .checksumSha256 = file.reportedChecksum.value_or(stapik::cloud::sha256Hex(file.content)),
                .updatedAt = std::chrono::system_clock::now() };
        }
    };
}
