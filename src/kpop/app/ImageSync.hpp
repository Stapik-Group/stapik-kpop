#pragma once

#include "CollectionController.hpp"

#include "kpop/image/ImageLibrary.hpp"
#include "kpop/image/ImageSynchronizer.hpp"

#include "stapik/task/BackgroundTaskRunner.hpp"
#include "stapik/task/DebouncedAction.hpp"

#include <sigc++/connection.h>
#include <sigc++/signal.h>

#include <functional>
#include <string>
#include <vector>

namespace kpop::app
{
    // Keeps one library of images in step with one BINARY_COLLECTION slot of the cloud. "referenced" says
    // which images the document needs right now.
    class ImageSync
    {
    public:
        using ReferencedImages = std::function<std::vector<domain::ImageId>()>;

        ImageSync(CollectionController& controller, image::ImageLibrary& library, ReferencedImages referenced, std::string slotKey);
        ~ImageSync();

        ImageSync(const ImageSync&) = delete;
        ImageSync& operator=(const ImageSync&) = delete;

        void requestSync();

        sigc::signal<void()>& signalImagesDownloaded();

    private:
        void start();
        void onFinished(const image::ImageSyncReport& report);

        CollectionController& m_controller;
        image::ImageLibrary& m_library;
        ReferencedImages m_referenced;
        std::string m_slotKey;

        stapik::task::BackgroundTaskRunner m_runner;
        stapik::task::DebouncedAction m_debounce;
        stapik::task::DebouncedAction m_retry;

        bool m_inFlight = false;
        bool m_requestedWhileRunning = false;
        int m_consecutiveFailures = 0;

        sigc::connection m_documentConnection;
        sigc::connection m_syncConnection;
        sigc::signal<void()> m_signalImagesDownloaded;
    };
}
