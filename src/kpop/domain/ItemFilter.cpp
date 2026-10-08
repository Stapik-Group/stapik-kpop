#include "ItemFilter.hpp"

#include <glibmm/ustring.h>

#include <initializer_list>
#include <sstream>
#include <string_view>
#include <variant>

namespace kpop::domain
{
    namespace
    {
        template<typename... Handlers>
        struct Overloaded : Handlers...
        {
            using Handlers::operator()...;
        };

        template<typename... Handlers>
        Overloaded(Handlers...) -> Overloaded<Handlers...>;

        void appendFields(std::string& target, const std::initializer_list<std::string_view> fields)
        {
            for (const auto field : fields)
            {
                target.append(field);
                target.push_back('\n');
            }
        }

        std::string casefolded(const std::string& text)
        {
            return Glib::ustring(text).casefold().raw();
        }

        std::string searchableText(const CollectionItem& item, const Artist* artist)
        {
            std::string text;
            appendFields(text, { item.title, item.acquiredFrom, item.notes });

            if (artist != nullptr)
            {
                appendFields(text, { artist->name });
                for (const auto& member : artist->members)
                    appendFields(text, { member });
            }

            std::visit(Overloaded{
                [&text](const AlbumDetails& album)
                {
                    appendFields(text, { album.edition, album.label, album.region, album.catalogNumber, album.inclusions });
                    for (const auto& track : album.tracklist)
                        appendFields(text, { track.title, track.writers });
                },
                [&text](const PhotocardDetails& photocard)
                {
                    appendFields(text, { photocard.member, photocard.source });
                },
                [&text](const MerchandiseDetails& merchandise)
                {
                    appendFields(text, { merchandise.version, merchandise.size });
                },
                [&text](const ClipDetails& clip)
                {
                    appendFields(text, { clip.platform, clip.url, clip.albumTitle });
                },
                [&text](const LyricsDetails& lyrics)
                {
                    appendFields(text, { lyrics.albumTitle, lyrics.writers, lyrics.originalText, lyrics.romanizedText, lyrics.translatedText });
                },
                [&text](const EventDetails& event)
                {
                    appendFields(text, { event.venue, event.city, event.seat });
                }
            }, item.details);

            return casefolded(text);
        }
    }

    bool matchesFilter(const ItemFilter& filter, const CollectionItem& item, const Artist* artist)
    {
        if (filter.kind && item.kind() != *filter.kind)
            return false;

        if (filter.status && item.status != *filter.status)
            return false;

        if (filter.artistId && item.artistId != *filter.artistId)
            return false;

        std::istringstream terms(casefolded(filter.text));
        std::string term;
        std::string haystack;
        bool haystackBuilt = false;

        while (terms >> term)
        {
            if (!haystackBuilt)
            {
                haystack = searchableText(item, artist);
                haystackBuilt = true;
            }

            if (haystack.find(term) == std::string::npos)
                return false;
        }

        return true;
    }
}
