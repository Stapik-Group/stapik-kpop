#pragma once

#include "FormGrid.hpp"

#include "kpop/domain/ItemDetails.hpp"

#include <sigc++/signal.h>

namespace kpop::ui
{
    class DetailsForm : public FormGrid
    {
    public:
        virtual void setDetails(const domain::ItemDetails& details) = 0;
        [[nodiscard]] virtual domain::ItemDetails details() const = 0;
        [[nodiscard]] virtual bool isValid() const;

        sigc::signal<void()>& signalChanged();

    protected:
        void notifyChanged() const;

    private:
        sigc::signal<void()> m_signalChanged;
    };

    [[nodiscard]] DetailsForm* createManagedDetailsForm(domain::ItemKind kind);
}
