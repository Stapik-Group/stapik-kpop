#pragma once

#include "stapik/time/Clock.hpp"

#include <chrono>

namespace kpop::test
{
    // A clock that stands still until the test moves it.
    class FakeClock final : public stapik::time::IClock
    {
    public:
        explicit FakeClock(const stapik::time::TimePoint start) :
            m_now(start)
        {}

        [[nodiscard]] stapik::time::TimePoint now() const override
        {
            return m_now;
        }

        void advance(const std::chrono::system_clock::duration duration)
        {
            m_now += duration;
        }

    private:
        stapik::time::TimePoint m_now;
    };
}
