#pragma once

#include "kpop/document/CollectionDocument.hpp"
#include "kpop/image/ImageLibrary.hpp"

#include "stapik/archive/AppArchive.hpp"
#include "stapik/cloud/ApiKeyExpiry.hpp"
#include "stapik/command/UndoStack.hpp"
#include "stapik/document/DocumentFile.hpp"
#include "stapik/settings/SettingsStore.hpp"
#include "stapik/sync/CloudSession.hpp"
#include "stapik/time/Clock.hpp"

#include <sigc++/signal.h>

#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace kpop::app
{
    enum class ArtistRemoval
    {
        Removed,
        InUse,
        NotFound
    };

    // What the archive of the whole application needs to know about the program. The archive of another program is
    // refused, and the settings are archived and restored together with the collection (none when null).
    struct ArchiveOptions
    {
        std::string appName;
        std::string appVersion;
        stapik::settings::SettingsStore* settings = nullptr;
    };

    using ArchiveExportTask = std::function<stapik::archive::ArchiveResult<stapik::archive::ArchiveInfo>()>;

    class CollectionController
    {
    public:
        CollectionController(
            std::filesystem::path documentPath,
            std::filesystem::path imagesDirectory,
            std::filesystem::path photosDirectory,
            stapik::sync::CloudSessionHooks cloudHooks,
            const stapik::time::IClock& clock = stapik::time::systemClock());

        CollectionController(const CollectionController&) = delete;
        CollectionController& operator=(const CollectionController&) = delete;

        [[nodiscard]] stapik::document::LoadStatus loadStatus() const;
        [[nodiscard]] const std::filesystem::path& documentPath() const;

        [[nodiscard]] const document::CollectionDocument& document() const;
        [[nodiscard]] stapik::command::UndoStack& undoStack();

        [[nodiscard]] image::ImageLibrary& imageLibrary();
        [[nodiscard]] const image::ImageLibrary& imageLibrary() const;

        // The additional photos of the entries (the covers are in imageLibrary()).
        [[nodiscard]] image::ImageLibrary& photoLibrary();
        [[nodiscard]] const image::ImageLibrary& photoLibrary() const;

        std::size_t removeUnusedImages();

        std::string addItem(domain::CollectionItem item);

        // Adds a copy of the entry (with a new id) and returns that id, or an empty text when there is no such entry.
        std::string duplicateItem(const std::string& itemId);
        void updateItem(const domain::CollectionItem& item);
        void removeItem(const std::string& itemId);

        std::string addArtist(domain::Artist artist);
        void updateArtist(const domain::Artist& artist);
        [[nodiscard]] ArtistRemoval removeArtist(const std::string& artistId);

        // The copies the file keeps of the collection from before the last saves, the newest first.
        [[nodiscard]] std::vector<stapik::document::BackupInfo> listBackups() const;

        // Replaces the collection with the one of the backup and saves it, so the collection from before the
        // restore becomes the newest backup. The undo history is dropped. Anything but Loaded means the backup could
        // not be used and nothing was changed.
        [[nodiscard]] stapik::document::LoadStatus restoreBackup(const stapik::document::BackupInfo& backup);

        // Captures the collection as it is now and returns the work of writing it, with the settings and all covers and
        // photos on this computer, to the ZIP file. Run the returned function on a background thread: copying the
        // images takes as long as the disk needs. The controller has to outlive it.
        [[nodiscard]] ArchiveExportTask prepareArchiveExport(std::filesystem::path target, ArchiveOptions options);

        // Replaces the collection, the images and the settings with the ones of the archive. The archive is checked as
        // a whole first, and nothing changes when it is damaged, of another program or newer than this one. Images
        // that are already here are kept and nothing is deleted. The collection from before the restore becomes the
        // newest backup. The undo history is dropped. The settings in memory of the program are not refreshed:
        // call reload() on the ones that are shown.
        [[nodiscard]] stapik::archive::ArchiveResult<stapik::archive::RestoreSummary> importArchive(const std::filesystem::path& source, const ArchiveOptions& options);

        [[nodiscard]] bool isCloudConnected() const;
        [[nodiscard]] const std::optional<CloudStorageConfig>& cloudConfig() const;
        [[nodiscard]] stapik::sync::SyncStatus syncStatus() const;
        void startCloudSync();
        bool connectCloud(const CloudStorageConfig& config);
        void syncNow();
        void flush();

        sigc::signal<void()>& signalDocumentChanged();
        sigc::signal<void(stapik::sync::SyncStatus)>& signalSyncStatusChanged();
        // The argument is the reason reported by the system, for example "No space left on device".
        sigc::signal<void(const std::string&)>& signalSaveFailed();

        // The cloud said the API key expires at the given time (it does so in the last 30 days of the key).
        sigc::signal<void(std::chrono::system_clock::time_point)>& signalApiKeyExpiring();

        // What the answers of the cloud said about the API key. The clients that talk to the cloud for files fill it
        // too, so it is passed to them. It is empty when the application has no cloud hooks.
        [[nodiscard]] std::shared_ptr<stapik::cloud::ApiKeyExpiryMonitor> apiKeyExpiryMonitor() const;

    private:
        using DocumentFile = stapik::document::DocumentFile<document::CollectionDocument>;
        using TimePoint = document::CollectionDocument::TimePoint;

        void loadFromDisk();
        void connectSignals();
        void onHistoryChanged();
        void onDocumentReplacedByCloud(const document::CollectionDocument& replacement);
        void onCloudBaselineChanged(TimePoint baseline);
        void persist() const;

        const stapik::time::IClock& m_clock;
        DocumentFile m_file;
        stapik::document::LoadStatus m_loadStatus = stapik::document::LoadStatus::Missing;
        document::CollectionDocument m_document;
        stapik::command::UndoStack m_undoStack;
        image::ImageLibrary m_imageLibrary;
        image::ImageLibrary m_photoLibrary;
        // Declared before the session, which takes the hooks this is copied from.
        std::shared_ptr<stapik::cloud::ApiKeyExpiryMonitor> m_apiKeyExpiryMonitor;
        stapik::sync::CloudSession<document::CollectionDocument> m_cloudSession;
        bool m_ignoreHistoryChanges = false;
        sigc::signal<void()> m_signalDocumentChanged;
        sigc::signal<void(stapik::sync::SyncStatus)> m_signalSyncStatusChanged;
        sigc::signal<void(const std::string&)> m_signalSaveFailed;
        sigc::signal<void(std::chrono::system_clock::time_point)> m_signalApiKeyExpiring;
    };
}
