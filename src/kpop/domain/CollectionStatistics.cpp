#include "CollectionStatistics.hpp"

#include <algorithm>
#include <numeric>

namespace kpop::domain
{
    namespace
    {
        bool countsAsSpent(const ItemStatus status)
        {
            return status == ItemStatus::Owned || status == ItemStatus::Ordered;
        }

        void addSpent(std::vector<stapik::domain::Money>& spent, const stapik::domain::Money& price, const int quantity)
        {
            const auto minorUnits = price.minorUnits() * static_cast<std::int64_t>(quantity);
            const auto existing = std::ranges::find_if(spent, [&price](const stapik::domain::Money& total)
            {
                return total.currency().code == price.currency().code;
            });

            if (existing == spent.end())
                spent.emplace_back(minorUnits, price.currency());
            else
                *existing = stapik::domain::Money(existing->minorUnits() + minorUnits, existing->currency());
        }
    }

    std::size_t CollectionStatistics::entriesOfKind(const ItemKind kind) const
    {
        return entriesPerKind[static_cast<std::size_t>(kind)];
    }

    std::size_t CollectionStatistics::totalEntries() const
    {
        return std::accumulate(entriesPerKind.begin(), entriesPerKind.end(), std::size_t{ 0 });
    }

    CollectionStatistics computeStatistics(const std::span<const CollectionItem> items)
    {
        CollectionStatistics statistics;

        for (const auto& item : items)
        {
            ++statistics.entriesPerKind[static_cast<std::size_t>(item.kind())];

            if (item.status == ItemStatus::Owned)
                statistics.ownedCopies += static_cast<std::size_t>(std::max(item.quantity, 0));

            if (item.price && countsAsSpent(item.status))
                addSpent(statistics.spent, *item.price, item.quantity);
        }

        return statistics;
    }
}
