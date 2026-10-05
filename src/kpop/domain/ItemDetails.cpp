#include "ItemDetails.hpp"

namespace kpop::domain
{
    ItemKind kindOf(const ItemDetails& details)
    {
        return static_cast<ItemKind>(details.index());
    }

    ItemDetails defaultDetails(const ItemKind kind)
    {
        switch (kind)
        {
            using enum ItemKind;
            case Album: return AlbumDetails{};
            case Photocard: return PhotocardDetails{};
            case Merchandise: return MerchandiseDetails{};
            case Clip: return ClipDetails{};
            case Lyrics: return LyricsDetails{};
            case Event: return EventDetails{};
        }

        return AlbumDetails{};
    }
}
