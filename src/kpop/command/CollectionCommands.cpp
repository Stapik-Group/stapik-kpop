#include "CollectionCommands.hpp"

#include "stapik/locale/LocaleManager.hpp"

#include <utility>

namespace kpop::command
{
    LocalizedCommand::LocalizedCommand(std::string descriptionKey, LocalizationEngine::Arguments descriptionArguments) :
        m_descriptionKey(std::move(descriptionKey)),
        m_descriptionArguments(std::move(descriptionArguments))
    {}

    std::string LocalizedCommand::description() const
    {
        return LocaleManager::instance().translate(m_descriptionKey, m_descriptionArguments);
    }

    AddItemCommand::AddItemCommand(document::CollectionDocument& document, domain::CollectionItem item) :
        LocalizedCommand("kpop.undo.addItem", { { "title", item.title } }),
        m_document(document),
        m_item(std::move(item))
    {}

    void AddItemCommand::execute()
    {
        m_document.insertItem(m_document.items().size(), m_item);
    }

    void AddItemCommand::undo()
    {
        m_document.removeItem(m_item.id);
    }

    UpdateItemCommand::UpdateItemCommand(document::CollectionDocument& document, domain::CollectionItem before, domain::CollectionItem after) :
        LocalizedCommand("kpop.undo.editItem", { { "title", after.title } }),
        m_document(document),
        m_before(std::move(before)),
        m_after(std::move(after))
    {}

    void UpdateItemCommand::execute()
    {
        m_document.replaceItem(m_after);
    }

    void UpdateItemCommand::undo()
    {
        m_document.replaceItem(m_before);
    }

    RemoveItemCommand::RemoveItemCommand(document::CollectionDocument& document, domain::CollectionItem item, const std::size_t index) :
        LocalizedCommand("kpop.undo.removeItem", { { "title", item.title } }),
        m_document(document),
        m_item(std::move(item)),
        m_index(index)
    {}

    void RemoveItemCommand::execute()
    {
        m_document.removeItem(m_item.id);
    }

    void RemoveItemCommand::undo()
    {
        m_document.insertItem(m_index, m_item);
    }

    AddArtistCommand::AddArtistCommand(document::CollectionDocument& document, domain::Artist artist) :
        LocalizedCommand("kpop.undo.addArtist", { { "name", artist.name } }),
        m_document(document),
        m_artist(std::move(artist))
    {}

    void AddArtistCommand::execute()
    {
        m_document.insertArtist(m_document.artists().size(), m_artist);
    }

    void AddArtistCommand::undo()
    {
        m_document.removeArtist(m_artist.id);
    }

    UpdateArtistCommand::UpdateArtistCommand(document::CollectionDocument& document, domain::Artist before, domain::Artist after) :
        LocalizedCommand("kpop.undo.editArtist", { { "name", after.name } }),
        m_document(document),
        m_before(std::move(before)),
        m_after(std::move(after))
    {}

    void UpdateArtistCommand::execute()
    {
        m_document.replaceArtist(m_after);
    }

    void UpdateArtistCommand::undo()
    {
        m_document.replaceArtist(m_before);
    }

    RemoveArtistCommand::RemoveArtistCommand(document::CollectionDocument& document, domain::Artist artist, const std::size_t index) :
        LocalizedCommand("kpop.undo.removeArtist", { { "name", artist.name } }),
        m_document(document),
        m_artist(std::move(artist)),
        m_index(index)
    {}

    void RemoveArtistCommand::execute()
    {
        m_document.removeArtist(m_artist.id);
    }

    void RemoveArtistCommand::undo()
    {
        m_document.insertArtist(m_index, m_artist);
    }
}
