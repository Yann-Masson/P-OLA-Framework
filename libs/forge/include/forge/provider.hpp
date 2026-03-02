#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <stdexcept>
#include <string>
#include <vector>

namespace forge
{

    // Forward declaration
    class Provider;

    // Internal implementation structure
    struct ProviderImpl
    {
        // Map of type_index to service instance
        std::unordered_map<std::type_index, std::shared_ptr<void>> services;

        // Map of type_index to multiple service instances (for multi-service registration)
        std::unordered_map<std::type_index, std::vector<std::shared_ptr<void>>> multiServices;

        template <typename T>
        void registerService(std::shared_ptr<T> instance)
        {
            const std::type_index typeIdx(typeid(T));
            services[typeIdx] = std::static_pointer_cast<void>(instance);
        }

        template <typename T>
        void registerMultiService(std::shared_ptr<T> instance)
        {
            const std::type_index typeIdx(typeid(T));
            multiServices[typeIdx].push_back(std::static_pointer_cast<void>(instance));
        }
    };

    /**
     * ProviderRef - A lightweight reference to the Provider
     * Can be passed to service constructors for dependency injection
     */
    class ProviderRef
    {
    public:
        ProviderRef() = default;

        /**
         * Get a service by its interface/base type
         * @tparam T The type of service to retrieve
         * @return Shared pointer to the service instance
         * @throws std::runtime_error if service not found
         */
        template <typename T>
        std::shared_ptr<T> get() const;

        /**
         * Try to get a service, returns nullptr if not found
         * @tparam T The type of service to retrieve
         * @return Shared pointer to the service instance or nullptr
         */
        template <typename T>
        std::shared_ptr<T> tryGet() const;

        /**
         * Check if a service is registered
         * @tparam T The type of service to check
         * @return true if service exists, false otherwise
         */
        template <typename T>
        bool has() const;

        /**
         * Get all services registered under a given interface type
         * @tparam T The interface type
         * @return Vector of shared pointers to all registered instances
         */
        template <typename T>
        std::vector<std::shared_ptr<T>> getAll() const;

    private:
        explicit ProviderRef(std::shared_ptr<ProviderImpl> impl) : pImpl(impl) {}

        std::shared_ptr<ProviderImpl> pImpl;

        friend class Provider;
    };

    /**
     * Provider - The main dependency injection container
     * Stores and manages service instances
     */
    class Provider
    {
    public:
        Provider() : pImpl(std::make_shared<ProviderImpl>()) {}

        /**
         * Get a service by its interface/base type
         * If multiple services are registered, returns the first one
         * @tparam T The type of service to retrieve
         * @return Shared pointer to the service instance
         * @throws std::runtime_error if service not found
         */
        template <typename T>
        std::shared_ptr<T> get() const
        {
            std::type_index typeIdx(typeid(T));
            auto it = pImpl->services.find(typeIdx);

            if (it != pImpl->services.end())
            {
                return std::static_pointer_cast<T>(it->second);
            }

            // Check multiServices and return first instance
            auto multiIt = pImpl->multiServices.find(typeIdx);
            if (multiIt != pImpl->multiServices.end() && !multiIt->second.empty())
            {
                return std::static_pointer_cast<T>(multiIt->second[0]);
            }

            throw std::runtime_error(
                std::string("Service not found: ") + typeid(T).name());
        }

        /**
         * Try to get a service, returns nullptr if not found
         * If multiple services are registered, returns the first one
         * @tparam T The type of service to retrieve
         * @return Shared pointer to the service instance or nullptr
         */
        template <typename T>
        std::shared_ptr<T> tryGet() const
        {
            std::type_index typeIdx(typeid(T));
            auto it = pImpl->services.find(typeIdx);

            if (it != pImpl->services.end())
            {
                return std::static_pointer_cast<T>(it->second);
            }

            // Check multiServices and return first instance
            auto multiIt = pImpl->multiServices.find(typeIdx);
            if (multiIt != pImpl->multiServices.end() && !multiIt->second.empty())
            {
                return std::static_pointer_cast<T>(multiIt->second[0]);
            }

            return nullptr;
        }

        /**
         * Check if a service is registered
         * @tparam T The type of service to check
         * @return true if service exists, false otherwise
         */
        template <typename T>
        bool has() const
        {
            std::type_index typeIdx(typeid(T));
            if (pImpl->services.find(typeIdx) != pImpl->services.end())
            {
                return true;
            }
            // Also check multiServices
            auto multiIt = pImpl->multiServices.find(typeIdx);
            return multiIt != pImpl->multiServices.end() && !multiIt->second.empty();
        }

        /**
         * Get all services registered under a given interface type
         * @tparam T The interface type
         * @return Vector of shared pointers to all registered instances
         */
        template <typename T>
        std::vector<std::shared_ptr<T>> getAll() const
        {
            std::type_index typeIdx(typeid(T));
            auto it = pImpl->multiServices.find(typeIdx);

            if (it == pImpl->multiServices.end())
            {
                return {};
            }

            std::vector<std::shared_ptr<T>> result;
            result.reserve(it->second.size());
            for (const auto &svc : it->second)
            {
                result.push_back(std::static_pointer_cast<T>(svc));
            }
            return result;
        }

        /**
         * Create a lightweight reference to this Provider
         * @return ProviderRef that can be passed to constructors
         */
        ProviderRef ref() const
        {
            return ProviderRef(pImpl);
        }

    private:
        std::shared_ptr<ProviderImpl> pImpl;

        friend class ProviderBuilder;
        friend class ProviderRef;
    };

    // ProviderRef template method implementations (after Provider is defined)

    template <typename T>
    std::shared_ptr<T> ProviderRef::get() const
    {
        if (!pImpl)
        {
            throw std::runtime_error("ProviderRef is not initialized");
        }

        std::type_index typeIdx(typeid(T));
        auto it = pImpl->services.find(typeIdx);

        if (it != pImpl->services.end())
        {
            return std::static_pointer_cast<T>(it->second);
        }

        // Check multiServices and return first instance
        auto multiIt = pImpl->multiServices.find(typeIdx);
        if (multiIt != pImpl->multiServices.end() && !multiIt->second.empty())
        {
            return std::static_pointer_cast<T>(multiIt->second[0]);
        }

        throw std::runtime_error(
            std::string("Service not found: ") + typeid(T).name());
    }

    template <typename T>
    std::shared_ptr<T> ProviderRef::tryGet() const
    {
        if (!pImpl)
        {
            return nullptr;
        }

        std::type_index typeIdx(typeid(T));
        auto it = pImpl->services.find(typeIdx);

        if (it != pImpl->services.end())
        {
            return std::static_pointer_cast<T>(it->second);
        }

        // Check multiServices and return first instance
        auto multiIt = pImpl->multiServices.find(typeIdx);
        if (multiIt != pImpl->multiServices.end() && !multiIt->second.empty())
        {
            return std::static_pointer_cast<T>(multiIt->second[0]);
        }

        return nullptr;
    }

    template <typename T>
    bool ProviderRef::has() const
    {
        if (!pImpl)
        {
            return false;
        }

        std::type_index typeIdx(typeid(T));
        if (pImpl->services.find(typeIdx) != pImpl->services.end())
        {
            return true;
        }
        // Also check multiServices
        auto multiIt = pImpl->multiServices.find(typeIdx);
        return multiIt != pImpl->multiServices.end() && !multiIt->second.empty();
    }

    template <typename T>
    std::vector<std::shared_ptr<T>> ProviderRef::getAll() const
    {
        if (!pImpl)
        {
            return {};
        }

        const std::type_index typeIdx(typeid(T));
        const auto it = pImpl->multiServices.find(typeIdx);

        if (it == pImpl->multiServices.end())
        {
            return {};
        }

        std::vector<std::shared_ptr<T>> result;
        result.reserve(it->second.size());
        for (const auto &svc : it->second)
        {
            result.push_back(std::static_pointer_cast<T>(svc));
        }
        return result;
    }


} // namespace forge
