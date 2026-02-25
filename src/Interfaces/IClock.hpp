/**
 * @file IClock.hpp
 * @brief Interface for clock services providing simulation time tracking.
 */

#pragma once

namespace POLA::Interfaces {

class IClock
{
public:
    virtual ~IClock() = default;
    virtual void simulate() = 0;
    [[nodiscard]] virtual double getElapsedTimeSinceStart() const = 0;
    [[nodiscard]] virtual double getElapsedTime() const = 0;
};

} // namespace POLA::Interfaces
