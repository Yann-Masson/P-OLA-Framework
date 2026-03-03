/**
 * @file IUserComfortService.hpp
 * @brief Interface for tracking user comfort.
 */

#pragma once

namespace POLA::Interfaces {

class IUserComfortService {
public:
    virtual ~IUserComfortService() = default;

    virtual double recordComfort(double indoorTemp) = 0;
    [[nodiscard]] virtual double getUserComfort() const = 0;
    virtual void reset() = 0;
};

} // namespace POLA::Interfaces
