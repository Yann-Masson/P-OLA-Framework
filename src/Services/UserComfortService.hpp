/**
 * @file ConsumptionService.hpp
 * @brief Service for recording and tracking energy consumption and costs.
 */

#pragma once

#include <forge/provider.hpp>

#include "Interfaces/IUserComfortService.hpp"

namespace POLA::Services {

class UserComfortService : public Interfaces::IUserComfortService
{
public:
    explicit UserComfortService(const forge::ProviderRef& provider);

    double recordComfort(double indoorTemp) override;
    [[nodiscard]] double getUserComfort() const override;
    void reset() override;

private:
    forge::ProviderRef _provider;
    double _totalComfort = 0.0;
    uint32_t _comfortRecords = 0;
    const double ALPHA = 0.4;
};

} // namespace POLA::Services
