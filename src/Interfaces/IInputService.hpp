/**
 * @file IInputService.hpp
 * @brief Generic interface for input data services providing sensor/external data.
 */

#pragma once

namespace POLA::Interfaces {

template<typename T>
class IInputService {
public:
    virtual ~IInputService() = default;
    virtual T getInput() = 0;
};

} // namespace POLA::Interfaces

