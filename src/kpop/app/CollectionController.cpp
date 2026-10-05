#include "CollectionController.hpp"

#include "kpop/command/CollectionCommands.hpp"
#include "kpop/document/CollectionSchema.hpp"

#include "stapik/storage/PathText.hpp"
#include "stapik/domain/IdGenerator.hpp"
#include "stapik/log/Log.hpp"

#include <chrono>
#include <memory>
#include <utility>

namespace kpop::app
{
    CollectionController::CollectionController(std::filesystem::path documentPath, stapik::sync::CloudSessionHooks cloudHooks) :
        m_file(std::move(documentPath), document::COLLECTION_SCHEMA_VERSION),
        m_cloudSession(std::move(cloudHooks))
    {
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

    std::string CollectionController::addItem(domain::CollectionItem item)
    {
        if (item.id.empty())
            item.id = stapik::domain::generateId();

        auto itemId = item.id;
        m_undoStack.execute(std::make_unique<command::AddItemCommand>(m_document, std::move(item)));
        return itemId;
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

    sigc::signal<void()>& CollectionController::signalSaveFailed()
    {
        return m_signalSaveFailed;
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

        m_cloudSession.signalStatusChanged().connect([this](const stapik::sync::SyncStatus status)
        {
            m_signalSyncStatusChanged.emit(status);
        });
    }

    void CollectionController::onHistoryChanged()
    {
        if (m_ignoreHistoryChanges)
            return;

        m_document.touch(std::chrono::system_clock::now());
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
        if (m_file.save(m_document))
            return;

        stapik::log::error("Cannot save the collection to {}", stapik::storage::pathText(m_file.filePath()));
        m_signalSaveFailed.emit();
    }
}
