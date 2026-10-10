#include "ImageSync.hpp"

#include "kpop/image/ImageSynchronizer.hpp"

#include "stapik/cloud/CloudAssetClient.hpp"
#include "stapik/log/Log.hpp"
#include "stapik/sync/Backoff.hpp"
#include "stapik/sync/SyncStatus.hpp"

#include <chrono>
#include <memory>
#include <optional>
#include <utility>

namespace kpop::app
{
    namespace
    {
        using std::chrono::milliseconds;
        using std::chrono::seconds;

        constexpr milliseconds DEBOUNCE_DELAY = seconds(2);
        constexpr milliseconds RETRY_INITIAL_DELAY = seconds(30);
        constexpr milliseconds RETRY_MAX_DELAY = std::chrono::minutes(10);
    }

    ImageSync::ImageSync(CollectionController& controller, image::ImageLibrary& library, ReferencedImages referenced, std::string slotKey) :
        m_controller(controller),
        m_library(library),
        m_referenced(std::move(referenced)),
        m_slotKey(std::move(slotKey)),
        m_debounce(DEBOUNCE_DELAY, [this] { start(); }),
        m_retry(RETRY_INITIAL_DELAY, [this] { start(); })
    {
        m_documentConnection = m_controller.signalDocumentChanged().connect([this] { requestSync(); });
        m_syncConnection = m_controller.signalSyncStatusChanged().connect([this](const stapik::sync::SyncStatus status)
        {
            // A finished sync of the document may have brought entries with images that are not here yet.
            if (status == stapik::sync::SyncStatus::Idle)
                requestSync();
        });
    }

    ImageSync::~ImageSync()
    {
        m_documentConnection.disconnect();
        m_syncConnection.disconnect();
    }

    void ImageSync::requestSync()
    {
        m_debounce.trigger();
    }

    sigc::signal<void()>& ImageSync::signalImagesDownloaded()
    {
        return m_signalImagesDownloaded;
    }

    void ImageSync::start()
    {
        if (m_inFlight)
        {
            m_requestedWhileRunning = true;
            return;
        }

        const auto& config = m_controller.cloudConfig();
        if (!config || !config->isConfigured())
            return;

        auto storage = std::make_shared<stapik::cloud::CloudAssetClient>(*config, m_slotKey);
        auto* store = &m_library.store();
        auto referenced = m_referenced();

        m_inFlight = true;
        m_requestedWhileRunning = false;

        m_runner.submit(
            [storage, store, referenced = std::move(referenced)]
            {
                return image::synchronizeImages(*storage, *store, referenced);
            },
            [this](std::optional<image::ImageSyncReport> report)
            {
                if (report)
                {
                    onFinished(*report);
                    return;
                }

                image::ImageSyncReport failed;
                failed.failed = true;
                failed.failure = "image synchronization ended without a result";
                onFinished(failed);
            });
    }

    void ImageSync::onFinished(const image::ImageSyncReport& report)
    {
        m_inFlight = false;

        if (report.downloaded > 0)
            m_signalImagesDownloaded.emit();

        if (report.skipped > 0)
            stapik::log::warning("{} image(s) could not be synced and were skipped", report.skipped);

        if (report.failed)
        {
            ++m_consecutiveFailures;
            stapik::log::warning("Syncing images failed (attempt {}): {}", m_consecutiveFailures, report.failure);

            m_retry.setDelay(stapik::sync::backoffDelay(RETRY_INITIAL_DELAY, RETRY_MAX_DELAY, m_consecutiveFailures));
            m_retry.trigger();
            return;
        }

        m_consecutiveFailures = 0;
        m_retry.cancel();

        if (m_requestedWhileRunning)
            requestSync();
    }
}
