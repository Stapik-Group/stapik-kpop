#pragma once

#include "kpop/document/CollectionDocument.hpp"
#include "kpop/image/ImageLibrary.hpp"

#include "stapik/command/UndoStack.hpp"
#include "stapik/document/DocumentFile.hpp"
#include "stapik/sync/CloudSession.hpp"

#include <sigc++/signal.h>

#include <filesystem>
#include <optional>
#include <string>

namespace kpop::app
{
    enum class ArtistRemoval
    {
        Removed,
        InUse,
        NotFound
    };

    class CollectionController
    {
    public:
        CollectionController(
            std::filesystem::path documentPath,
            std::filesystem::path imagesDirectory,
            std::filesystem::path photosDirectory,
            stapik::sync::CloudSessionHooks cloudHooks);

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
        void updateItem(const domain::CollectionItem& item);
        void removeItem(const std::string& itemId);

        std::string addArtist(domain::Artist artist);
        void updateArtist(const domain::Artist& artist);
        [[nodiscard]] ArtistRemoval removeArtist(const std::string& artistId);

        [[nodiscard]] bool isCloudConnected() const;
        [[nodiscard]] const std::optional<CloudStorageConfig>& cloudConfig() const;
        [[nodiscard]] stapik::sync::SyncStatus syncStatus() const;
        void startCloudSync();
        bool connectCloud(const CloudStorageConfig& config);
        void syncNow();
        void flush();

        sigc::signal<void()>& signalDocumentChanged();
        sigc::signal<void(stapik::sync::SyncStatus)>& signalSyncStatusChanged();
        sigc::signal<void()>& signalSaveFailed();

    private:
        using DocumentFile = stapik::document::DocumentFile<document::CollectionDocument>;
        using TimePoint = document::CollectionDocument::TimePoint;

        void loadFromDisk();
        void connectSignals();
        void onHistoryChanged();
        void onDocumentReplacedByCloud(const document::CollectionDocument& replacement);
        void onCloudBaselineChanged(TimePoint baseline);
        void persist() const;

        DocumentFile m_file;
        stapik::document::LoadStatus m_loadStatus = stapik::document::LoadStatus::Missing;
        document::CollectionDocument m_document;
        stapik::command::UndoStack m_undoStack;
        image::ImageLibrary m_imageLibrary;
        image::ImageLibrary m_photoLibrary;
        stapik::sync::CloudSession<document::CollectionDocument> m_cloudSession;
        bool m_ignoreHistoryChanges = false;
        sigc::signal<void()> m_signalDocumentChanged;
        sigc::signal<void(stapik::sync::SyncStatus)> m_signalSyncStatusChanged;
        sigc::signal<void()> m_signalSaveFailed;
    };
}
