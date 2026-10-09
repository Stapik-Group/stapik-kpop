#pragma once

#include "kpop/document/CollectionDocument.hpp"

#include "stapik/domain/IdGenerator.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

namespace kpop::test
{
    inline stapik::domain::Currency zloty()
    {
        return stapik::domain::Currency{ .code = "PLN", .symbol = "zł", .symbolBeforeAmount = false, .decimalPlaces = 2 };
    }

    inline stapik::domain::Currency won()
    {
        return stapik::domain::Currency{ .code = "KRW", .symbol = "₩", .symbolBeforeAmount = true, .decimalPlaces = 0 };
    }

    inline domain::Artist sampleArtist(const std::string& id = "artist-1", const std::string& name = "Stray Kids")
    {
        return domain::Artist{ .id = id, .name = name, .type = domain::ArtistType::Group, .members = { "Bang Chan", "Felix" } };
    }

    inline domain::CollectionItem sampleAlbum(const std::string& id = "item-1", const std::string& title = "NOEASY")
    {
        domain::CollectionItem item;
        item.id = id;
        item.title = title;
        item.artistId = "artist-1";
        item.status = domain::ItemStatus::Owned;
        item.condition = domain::ItemCondition::Sealed;
        item.quantity = 2;
        item.price = stapik::domain::Money(8900, zloty());
        item.acquiredOn = domain::PartialDate::tryCreate(2024, 3, 15);
        item.acquiredFrom = "Kpopstore";
        item.notes = "Limited ver.";
        item.image = domain::ImageId::parse("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
        item.details = domain::AlbumDetails{
            .type = domain::AlbumType::Full,
            .format = domain::AlbumFormat::Cd,
            .edition = "Standard B",
            .releaseDate = domain::PartialDate::tryCreate(2021, 8),
            .label = "JYP",
            .region = "KR",
            .catalogNumber = "L200002046",
            .inclusions = "photocard, poster",
            .tracklist = {
                { .title = "Intro", .writers = "Bang Chan, Changbin", .length = domain::TrackLength::fromSeconds(65), .titleTrack = false },
                { .title = "Dance the Night Away", .writers = "Han", .length = domain::TrackLength::fromSeconds(201), .titleTrack = true },
                { .title = "Outro", .writers = "", .length = std::nullopt, .titleTrack = false } } };
        return item;
    }

    inline domain::CollectionItem sampleItemOfKind(const domain::ItemKind kind)
    {
        auto item = sampleAlbum("item-" + std::string(domain::ITEM_KINDS.idOf(kind)), "Sample");
        item.details = domain::defaultDetails(kind);
        return item;
    }

    inline document::CollectionDocument sampleDocument()
    {
        document::CollectionDocument document;
        document.insertArtist(0, sampleArtist());
        document.insertItem(0, sampleAlbum());
        document.touch(std::chrono::sys_days{ std::chrono::year{ 2026 } / std::chrono::October / 3 } + std::chrono::hours{ 12 });
        return document;
    }

    class TemporaryDirectory
    {
    public:
        TemporaryDirectory() :
            m_path(std::filesystem::temp_directory_path() / ("stapikkpop-test-" + stapik::domain::generateId()))
        {
            std::filesystem::create_directories(m_path);
        }

        ~TemporaryDirectory()
        {
            std::error_code errorCode;
            std::filesystem::remove_all(m_path, errorCode);
        }

        TemporaryDirectory(const TemporaryDirectory&) = delete;
        TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

        [[nodiscard]] const std::filesystem::path& path() const
        {
            return m_path;
        }

    private:
        std::filesystem::path m_path;
    };

    inline void writeTextFile(const std::filesystem::path& path, const std::string& content)
    {
        std::ofstream file(path);
        file << content;
    }
}
