/**
 * @file AInputService.hpp
 * @brief Abstract base class for input services, providing DI provider access.
 */

#pragma once

#include <forge/provider.hpp>
#include "Interfaces/IInputService.hpp"

namespace POLA::Services::Inputs {

template <typename T>
class AInputService : public Interfaces::IInputService<T>
{
public:
    explicit AInputService(const forge::ProviderRef& provider)
        : _provider(provider)
    {
    }

protected:
    forge::ProviderRef _provider;
};

} // namespace POLA::Services::Inputs

