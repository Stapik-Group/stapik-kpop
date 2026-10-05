#pragma once

#include "kpop/document/CollectionDocument.hpp"

#include "stapik/command/ICommand.hpp"
#include "stapik/locale/LocalizationEngine.hpp"

#include <cstddef>
#include <string>

namespace kpop::command
{
    class LocalizedCommand : public stapik::command::ICommand
    {
    public:
        [[nodiscard]] std::string description() const override;

    protected:
        LocalizedCommand(std::string descriptionKey, LocalizationEngine::Arguments descriptionArguments);

    private:
        std::string m_descriptionKey;
        LocalizationEngine::Arguments m_descriptionArguments;
    };

    class AddItemCommand final : public LocalizedCommand
    {
    public:
        AddItemCommand(document::CollectionDocument& document, domain::CollectionItem item);

        void execute() override;
        void undo() override;

    private:
        document::CollectionDocument& m_document;
        domain::CollectionItem m_item;
    };

    class UpdateItemCommand final : public LocalizedCommand
    {
    public:
        UpdateItemCommand(document::CollectionDocument& document, domain::CollectionItem before, domain::CollectionItem after);

        void execute() override;
        void undo() override;

    private:
        document::CollectionDocument& m_document;
        domain::CollectionItem m_before;
        domain::CollectionItem m_after;
    };

    class RemoveItemCommand final : public LocalizedCommand
    {
    public:
        RemoveItemCommand(document::CollectionDocument& document, domain::CollectionItem item, std::size_t index);

        void execute() override;
        void undo() override;

    private:
        document::CollectionDocument& m_document;
        domain::CollectionItem m_item;
        std::size_t m_index;
    };

    class AddArtistCommand final : public LocalizedCommand
    {
    public:
        AddArtistCommand(document::CollectionDocument& document, domain::Artist artist);

        void execute() override;
        void undo() override;

    private:
        document::CollectionDocument& m_document;
        domain::Artist m_artist;
    };

    class UpdateArtistCommand final : public LocalizedCommand
    {
    public:
        UpdateArtistCommand(document::CollectionDocument& document, domain::Artist before, domain::Artist after);

        void execute() override;
        void undo() override;

    private:
        document::CollectionDocument& m_document;
        domain::Artist m_before;
        domain::Artist m_after;
    };

    class RemoveArtistCommand final : public LocalizedCommand
    {
    public:
        RemoveArtistCommand(document::CollectionDocument& document, domain::Artist artist, std::size_t index);

        void execute() override;
        void undo() override;

    private:
        document::CollectionDocument& m_document;
        domain::Artist m_artist;
        std::size_t m_index;
    };
}
