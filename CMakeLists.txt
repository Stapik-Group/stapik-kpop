cmake_minimum_required(VERSION 4.2)
project(stapikkpop VERSION 1.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(STAPIK_KPOP_BUILD_TESTS "Build the Stapik K-Pop unit tests" OFF)

set(STAPIK_COMMON_GIT_TAG "v1.5.5" CACHE STRING "Tag, branch or commit of stapik-common to build against")

include(FetchContent)
FetchContent_Declare(
        stapikcommon
        GIT_REPOSITORY https://github.com/Stapik-Group/stapik-common.git
        GIT_TAG ${STAPIK_COMMON_GIT_TAG}
)
FetchContent_MakeAvailable(stapikcommon)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GDKPIXBUF REQUIRED IMPORTED_TARGET gdk-pixbuf-2.0)

# Everything that does not need a window lives in the core library, so it can be unit tested.
add_library(kpopcore STATIC
        src/kpop/app/CollectionController.cpp
        src/kpop/app/ImageSync.cpp
        src/kpop/command/CollectionCommands.cpp
        src/kpop/document/CollectionDocument.cpp
        src/kpop/document/CollectionSerializer.cpp
        src/kpop/domain/ArtistColor.cpp
        src/kpop/domain/CollectionStatistics.cpp
        src/kpop/domain/ImageId.cpp
        src/kpop/domain/ItemDetails.cpp
        src/kpop/domain/ItemFilter.cpp
        src/kpop/domain/ItemSort.cpp
        src/kpop/domain/PartialDate.cpp
        src/kpop/domain/PartialDateFormatter.cpp
        src/kpop/domain/TrackLength.cpp
        src/kpop/domain/Tracklist.cpp
        src/kpop/image/ImageArchiveStorage.cpp
        src/kpop/image/ImageEncoder.cpp
        src/kpop/image/ImageLibrary.cpp
        src/kpop/image/ImageStore.cpp
        src/kpop/image/ImageSynchronizer.cpp
)
target_include_directories(kpopcore PUBLIC src)
target_link_libraries(kpopcore PUBLIC stapik::common PkgConfig::GDKPIXBUF)
stapik_enable_warnings(kpopcore)

stapik_add_application(
        NAME stapikkpop
        DISPLAY_NAME "Stapik K-Pop"
        DESCRIPTION "Retro-styled K-pop collection manager with optional own cloud sync"
        APPLICATION_ID pl.stapik.kpop
        RESOURCES_DIRECTORY resources
        PACKAGING_DIRECTORY packaging
        HOMEPAGE https://github.com/Stapik-Group/stapik-kpop
        MAINTAINER "Sebastian Smoliński <sebastian.smolinski6@gmail.com>"
        LICENSE GPL-3.0-only
        LONG_DESCRIPTION "Stapik K-Pop keeps a K-pop collection in one place: albums with tracklists, photocards, merchandise, clips, lyrics and events. Entries have an artist, status, condition, price and covers with photos, everything can be searched, undone and restored from automatic backups, and the collection can optionally be synchronised, with end-to-end encryption, with your own Stapik Cloud."
        RELEASE_DATE 2026-10-14
        LOCALIZED_DESCRIPTIONS
                "pl=Retro menedżer kolekcji K-popu z opcjonalną synchronizacją z własną chmurą"
                "de=Retro-Verwaltung einer K-Pop-Sammlung mit optionalem Abgleich mit der eigenen Cloud"
        KEYWORDS kpop k-pop collection photocard album
        LOCALIZED_KEYWORDS
                "pl=kpop,k-pop,kolekcja,photocard,album,gadżety"
                "de=kpop,k-pop,sammlung,photocard,album,merchandise"
        SOURCES
                src/main.cpp
                src/kpop/ui/ItemPresenter.cpp
                src/kpop/ui/MainWindow.cpp
                src/kpop/ui/SelfTest.cpp
                src/kpop/ui/dialog/ArtistEditDialog.cpp
                src/kpop/ui/dialog/ArtistsDialog.cpp
                src/kpop/ui/dialog/ItemDialog.cpp
                src/kpop/ui/dialog/TracklistDialog.cpp
                src/kpop/ui/form/DetailsForms.cpp
                src/kpop/ui/form/FormGrid.cpp
                src/kpop/ui/view/ItemGridView.cpp
                src/kpop/ui/view/ItemListView.cpp
                src/kpop/ui/view/ItemShelfView.cpp
                src/kpop/ui/view/KindSidebar.cpp
                src/kpop/ui/widget/FilterBar.cpp
                src/kpop/ui/widget/MoneyEntry.cpp
                src/kpop/ui/widget/MultilineText.cpp
                src/kpop/ui/widget/PaginationBar.cpp
                src/kpop/ui/widget/ImageFileDialog.cpp
                src/kpop/ui/widget/ImagePicker.cpp
                src/kpop/ui/widget/ImageTexture.cpp
                src/kpop/ui/widget/ItemContextMenu.cpp
                src/kpop/ui/widget/PartialDateEntry.cpp
                src/kpop/ui/widget/PhotoGallery.cpp
                src/kpop/ui/widget/QuickPickEntry.cpp
                src/kpop/ui/widget/TrackLengthEntry.cpp
                src/kpop/ui/widget/TracklistEditor.cpp
        LINK_LIBRARIES kpopcore
)

if(STAPIK_KPOP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
