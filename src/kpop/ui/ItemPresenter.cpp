#include "ItemPresenter.hpp"

#include "Translate.hpp"

#include "stapik/domain/MoneyFormatter.hpp"

#include <initializer_list>
#include <optional>
#include <variant>

namespace kpop::ui
{
    namespace
    {
        using namespace domain;

        template<typename... Handlers>
        struct Overloaded : Handlers...
        {
            using Handlers::operator()...;
        };

        template<typename... Handlers>
        Overloaded(Handlers...) -> Overloaded<Handlers...>;

        std::string joinNonEmpty(const std::initializer_list<std::string> parts)
        {
            constexpr std::string_view SEPARATOR = " · ";

            std::string joined;
            for (const auto& part : parts)
            {
                if (part.empty())
                    continue;

                if (!joined.empty())
                    joined += SEPARATOR;

                joined += part;
            }

            return joined;
        }

        std::string dateText(const std::optional<PartialDate>& date)
        {
            return date ? date->toKey() : std::string();
        }

        std::string detailsSummary(const ItemDetails& details)
        {
            return std::visit(Overloaded{
                [](const AlbumDetails& album)
                {
                    return joinNonEmpty({ translate(ALBUM_TYPES.nameKey(album.type)), translate(ALBUM_FORMATS.nameKey(album.format)), album.edition });
                },
                [](const PhotocardDetails& photocard)
                {
                    return joinNonEmpty({ photocard.member, translate(PHOTOCARD_ORIGINS.nameKey(photocard.origin)) });
                },
                [](const MerchandiseDetails& merchandise)
                {
                    return joinNonEmpty({ translate(MERCHANDISE_TYPES.nameKey(merchandise.type)), merchandise.version });
                },
                [](const ClipDetails& clip)
                {
                    return joinNonEmpty({ translate(CLIP_TYPES.nameKey(clip.type)), clip.platform });
                },
                [](const LyricsDetails& lyrics)
                {
                    return joinNonEmpty({ lyrics.albumTitle, lyrics.writers });
                },
                [](const EventDetails& event)
                {
                    return joinNonEmpty({ translate(EVENT_TYPES.nameKey(event.type)), dateText(event.date), event.city });
                }
            }, details);
        }
    }

    ItemRow describeItem(const CollectionItem& item, const Artist* artist, const std::string_view languageCode)
    {
        ItemRow row;
        row.id = item.id;
        row.kind = item.kind();
        row.status = item.status;
        row.title = item.title;
        row.subtitle = joinNonEmpty({ artist != nullptr ? artist->name : std::string(), detailsSummary(item.details) });
        row.statusText = translate(ITEM_STATUSES.nameKey(item.status));

        if (item.quantity > 1)
            row.quantityText = "×" + std::to_string(item.quantity);

        if (item.price)
            row.priceText = stapik::domain::formatMoney(*item.price, languageCode);

        return row;
    }
}
