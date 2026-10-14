#include "CollectionController.hpp"

#include "kpop/command/CollectionCommands.hpp"
#include "kpop/document/CollectionSchema.hpp"
#include "kpop/image/ImageArchiveStorage.hpp"

#include "stapik/domain/IdGenerator.hpp"
#include "stapik/storage/PathText.hpp"

#include <algorithm>
#include <memory>
#include <utility>

namespace kpop::app
{
    namespace
    {
        // One copy is hardly worth restoring from: it is the file as it was before the very last edit.
        constexpr int BACKUP_COUNT = 5;

        std::size_t removeUnreferenced(image::ImageStore& store, const std::vector<domain::ImageId>& used)
        {
            std::size_t removed = 0;
            for (const auto& id : store.ids())
            {
                if (!std::ranges::binary_search(used, id) && store.remove(id))
                    ++removed;
            }

            return removed;
        }
    }

    CollectionController::CollectionController(
        std::filesystem::path documentPath,
        std::filesystem::path imagesDirectory,
        std::filesystem::path photosDirectory,
        stapik::sync::CloudSessionHooks cloudHooks,
        const stapik::time::IClock& clock) :
        m_clock(clock),
        m_file(std::move(documentPath), document::COLLECTION_SCHEMA_VERSION, clock),
        m_imageLibrary(std::move(imagesDirectory)),
        m_photoLibrary(std::move(photosDirectory), image::MAX_PHOTO_DIMENSION),
        m_apiKeyExpiryMonitor(cloudHooks.expiryMonitor),
        m_cloudSession(std::move(cloudHooks))
    {
        m_file.setBackupCount(BACKUP_COUNT);
        loadFromDisk();
        connectSignals();
    }

    stapik::document::LoadStatus CollectionController::loadStatus() const
    {
        return m_loadStatus;
    }

    const std::filesystem::path& CollectionController::documentPath() const
    {
        return m_file.filePath();
    }

    const document::CollectionDocument& CollectionController::document() const
    {
        return m_document;
    }

    stapik::command::UndoStack& CollectionController::undoStack()
    {
        return m_undoStack;
    }

    image::ImageLibrary& CollectionController::imageLibrary()
    {
        return m_imageLibrary;
    }

    const image::ImageLibrary& CollectionController::imageLibrary() const
    {
        return m_imageLibrary;
    }

    image::ImageLibrary& CollectionController::photoLibrary()
    {
        return m_photoLibrary;
    }

    const image::ImageLibrary& CollectionController::photoLibrary() const
    {
        return m_photoLibrary;
    }

    std::size_t CollectionController::removeUnusedImages()
    {
        using stapik::document::LoadStatus;
        if (m_loadStatus != LoadStatus::Loaded && m_loadStatus != LoadStatus::Missing)
            return 0;

        return removeUnreferenced(m_imageLibrary.store(), m_document.referencedImages())
            + removeUnreferenced(m_photoLibrary.store(), m_document.referencedPhotos());
    }

    std::string CollectionController::addItem(domain::CollectionItem item)
    {
        if (item.id.empty())
            item.id = stapik::domain::generateId();

        auto itemId = item.id;
        m_undoStack.execute(std::make_unique<command::AddItemCommand>(m_document, std::move(item)));
        return itemId;
    }

    std::string CollectionController::duplicateItem(const std::string& itemId)
    {
        const auto* existing = m_document.findItem(itemId);
        if (existing == nullptr)
            return {};

        return addItem(domain::duplicateOf(*existing));
    }

    void CollectionController::updateItem(const domain::CollectionItem& item)
    {
        const auto* existing = m_document.findItem(item.id);
        if (existing == nullptr || *existing == item)
            return;

        m_undoStack.execute(std::make_unique<command::UpdateItemCommand>(m_document, *existing, item));
    }

    void CollectionController::removeItem(const std::string& itemId)
    {
        const auto* existing = m_document.findItem(itemId);
        const auto index = m_document.indexOfItem(itemId);
        if (existing == nullptr || !index)
            return;

        m_undoStack.execute(std::make_unique<command::RemoveItemCommand>(m_document, *existing, *index));
    }

    std::string CollectionController::addArtist(domain::Artist artist)
    {
        if (artist.id.empty())
            artist.id = stapik::domain::generateId();

        auto artistId = artist.id;
        m_undoStack.execute(std::make_unique<command::AddArtistCommand>(m_document, std::move(artist)));
        return artistId;
    }

    void CollectionController::updateArtist(const domain::Artist& artist)
    {
        const auto* existing = m_document.findArtist(artist.id);
        if (existing == nullptr || *existing == artist)
            return;

        m_undoStack.execute(std::make_unique<command::UpdateArtistCommand>(m_document, *existing, artist));
    }

    ArtistRemoval CollectionController::removeArtist(const std::string& artistId)
    {
        const auto* existing = m_document.findArtist(artistId);
        const auto index = m_document.indexOfArtist(artistId);
        if (existing == nullptr || !index)
            return ArtistRemoval::NotFound;

        if (m_document.countItemsOfArtist(artistId) > 0)
            return ArtistRemoval::InUse;

        m_undoStack.execute(std::make_unique<command::RemoveArtistCommand>(m_document, *existing, *index));
        return ArtistRemoval::Removed;
    }

    bool CollectionController::isCloudConnected() const
    {
        return m_cloudSession.isConnected();
    }

    const std::optional<CloudStorageConfig>& CollectionController::cloudConfig() const
    {
        return m_cloudSession.config();
    }

    stapik::sync::SyncStatus CollectionController::syncStatus() const
    {
        return m_cloudSession.status();
    }

    void CollectionController::startCloudSync()
    {
        if (m_cloudSession.isConnected())
            m_cloudSession.syncNow(m_document);
    }

    bool CollectionController::connectCloud(const CloudStorageConfig& config)
    {
        return m_cloudSession.connectWith(config, m_document);
    }

    void CollectionController::syncNow()
    {
        m_cloudSession.syncNow(m_document);
    }

    void CollectionController::flush()
    {
        m_cloudSession.flush();
    }

    sigc::signal<void()>& CollectionController::signalDocumentChanged()
    {
        return m_signalDocumentChanged;
    }

    sigc::signal<void(stapik::sync::SyncStatus)>& CollectionController::signalSyncStatusChanged()
    {
        return m_signalSyncStatusChanged;
    }

    sigc::signal<void(const std::string&)>& CollectionController::signalSaveFailed()
    {
        return m_signalSaveFailed;
    }

    std::vector<stapik::document::BackupInfo> CollectionController::listBackups() const
    {
        return m_file.listBackups();
    }

    stapik::document::LoadStatus CollectionController::restoreBackup(const stapik::document::BackupInfo& backup)
    {
        using stapik::document::LoadStatus;

        auto loaded = m_file.loadBackup(backup);
        if (loaded.status != LoadStatus::Loaded)
            return loaded.status;

        if (!loaded.document)
            return LoadStatus::Corrupted;

        // The undo history refers to entries of the document that is being thrown away.
        m_ignoreHistoryChanges = true;
        m_undoStack.clear();
        m_ignoreHistoryChanges = false;

        m_document = std::move(*loaded.document);

        // The cloud keeps whichever version changed last, so the restored one has to count as changed now.
        m_document.touch(m_clock.now());
        persist();
        m_cloudSession.pushChange(m_document);
        m_signalDocumentChanged.emit();
        return LoadStatus::Loaded;
    }

    ArchiveExportTask CollectionController::prepareArchiveExport(std::filesystem::path target, ArchiveOptions options)
    {
        // The collection is written from memory, so changes that are not saved yet are included. The text is made here,
        // on the thread of the window, because the task runs while the collection may still be edited.
        return [this, text = m_file.serialize(m_document), target = std::move(target), options = std::move(options)]
        {
            image::ImageArchiveStorage images(m_imageLibrary.store(), m_photoLibrary.store());

            const stapik::archive::AppArchiveContext context{
                .appName = options.appName,
                .appVersion = options.appVersion,
                .settings = options.settings,
                .assets = &images };

            return stapik::archive::writeAppArchive(target, context, text, m_clock);
        };
    }

    stapik::archive::ArchiveResult<stapik::archive::RestoreSummary> CollectionController::importArchive(const std::filesystem::path& source, const ArchiveOptions& options)
    {
        using namespace stapik::archive;
        using stapik::document::LoadStatus;

        auto archive = AppArchive::open(source, options.appName);
        if (!archive)
            return stapik::Failure(std::move(archive).error());

        auto loaded = m_file.deserialize(archive->documentText(), "the archive " + stapik::storage::pathText(source));
        if (loaded.status == LoadStatus::NewerVersion)
            return archiveFailure(ArchiveErrorKind::Unsupported, "The archive was made by a newer version of the application");

        if (loaded.status != LoadStatus::Loaded || !loaded.document)
            return archiveFailure(ArchiveErrorKind::Corrupted, "The collection in the archive cannot be read");

        image::ImageArchiveStorage images(m_imageLibrary.store(), m_photoLibrary.store());
        auto restored = archive->restoreAssets(images);
        if (!restored)
            return stapik::Failure(std::move(restored).error());

        // The cloud keeps whichever version changed last, so the restored collection has to count as changed now.
        // It is saved before it replaces the one in memory, so a failure leaves the collection as it was.
        auto replacement = std::move(*loaded.document);
        replacement.touch(m_clock.now());
        if (const auto saved = m_file.save(replacement); !saved)
            return archiveFailure(ArchiveErrorKind::Io, saved.error().message());

        m_ignoreHistoryChanges = true;
        m_undoStack.clear();
        m_ignoreHistoryChanges = false;

        m_document = std::move(replacement);
        m_cloudSession.pushChange(m_document);
        m_signalDocumentChanged.emit();

        RestoreSummary summary = *restored;
        if (options.settings != nullptr)
        {
            // The collection is already restored here, so a failure is reported in the summary and not as an error.
            const auto settings = archive->restoreSettings(*options.settings);
            summary.settingsRestored = settings && *settings;
        }

        return summary;
    }

    sigc::signal<void(std::chrono::system_clock::time_point)>& CollectionController::signalApiKeyExpiring()
    {
        return m_signalApiKeyExpiring;
    }

    std::shared_ptr<stapik::cloud::ApiKeyExpiryMonitor> CollectionController::apiKeyExpiryMonitor() const
    {
        return m_apiKeyExpiryMonitor;
    }

    void CollectionController::loadFromDisk()
    {
        auto result = m_file.load();
        m_loadStatus = result.status;

        if (result.status != stapik::document::LoadStatus::Loaded || !result.document)
            return;

        m_document = std::move(*result.document);

        if (result.migrated)
            persist();
    }

    void CollectionController::connectSignals()
    {
        m_undoStack.signalChanged().connect([this] { onHistoryChanged(); });

        m_cloudSession.signalDocumentReplaced().connect([this](const document::CollectionDocument& replacement)
        {
            onDocumentReplacedByCloud(replacement);
        });

        m_cloudSession.signalBaselineChanged().connect([this](const TimePoint baseline)
        {
            onCloudBaselineChanged(baseline);
        });

        m_cloudSession.signalApiKeyExpiring().connect([this](const TimePoint expiresAt)
        {
            m_signalApiKeyExpiring.emit(expiresAt);
        });

        m_cloudSession.signalStatusChanged().connect([this](const stapik::sync::SyncStatus status)
        {
            m_signalSyncStatusChanged.emit(status);
        });
    }

    void CollectionController::onHistoryChanged()
    {
        if (m_ignoreHistoryChanges)
            return;

        m_document.touch(m_clock.now());
        persist();
        m_cloudSession.pushChange(m_document);
        m_signalDocumentChanged.emit();
    }

    void CollectionController::onDocumentReplacedByCloud(const document::CollectionDocument& replacement)
    {
        // The undo history refers to entries of the document that is being thrown away.
        m_ignoreHistoryChanges = true;
        m_undoStack.clear();
        m_ignoreHistoryChanges = false;

        m_document = replacement;
        persist();
        m_signalDocumentChanged.emit();
    }

    void CollectionController::onCloudBaselineChanged(const TimePoint baseline)
    {
        m_document = m_document.withLastKnownCloudUpdate(baseline);
        persist();
    }

    void CollectionController::persist() const
    {
        // stapik-common logs the failure itself, so only the reason is passed on.
        const auto result = m_file.save(m_document);
        if (result)
            return;

        m_signalSaveFailed.emit(result.error().code.message());
    }
}
