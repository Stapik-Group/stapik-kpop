#pragma once

#include "CollectionController.hpp"

#include "kpop/image/ImageSynchronizer.hpp"

#include "stapik/task/BackgroundTaskRunner.hpp"
#include "stapik/task/DebouncedAction.hpp"

#include <sigc++/connection.h>
#include <sigc++/signal.h>

#include <string>

namespace kpop::app
{
    class ImageSync
    {
    public:
        ImageSync(CollectionController& controller, std::string slotKey);
        ~ImageSync();

        ImageSync(const ImageSync&) = delete;
        ImageSync& operator=(const ImageSync&) = delete;

        void requestSync();

        sigc::signal<void()>& signalImagesDownloaded();

    private:
        void start();
        void onFinished(const image::ImageSyncReport& report);

        CollectionController& m_controller;
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
