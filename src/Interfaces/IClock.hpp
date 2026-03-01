/**
 * @file IClock.hpp
 * @brief Interface for clock services providing simulation time tracking.
 */

#pragma once
#include <cstdint>

namespace POLA::Interfaces {

class IClock
{
public:
    virtual ~IClock() = default;
    virtual void simulate() = 0;
    [[nodiscard]] virtual uint32_t getElapsedTimeSinceStart() const = 0;
    [[nodiscard]] virtual uint32_t getElapsedTime() const = 0;
};

} // namespace POLA::Interfaces
