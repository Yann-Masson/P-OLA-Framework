#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <stdexcept>
#include <string>

namespace dicnew
{

    // Forward declaration
    class ServiceProvider;

    // Internal implementation structure
    struct ServiceProviderImpl
    {
        // Map of type_index to service instance
        std::unordered_map<std::type_index, std::shared_ptr<void>> services;

        template <typename T>
        void registerService(std::shared_ptr<T> instance)
        {
            std::type_index typeIdx(typeid(T));
            services[typeIdx] = std::static_pointer_cast<void>(instance);
        }
    };

    /**
     * ServiceProviderRef - A lightweight reference to the ServiceProvider
     * Can be passed to service constructors for dependency injection
     */
    class ServiceProviderRef
    {
    public:
        ServiceProviderRef() = default;

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

    private:
        explicit ServiceProviderRef(std::shared_ptr<ServiceProviderImpl> impl) : pImpl(impl) {}

        std::shared_ptr<ServiceProviderImpl> pImpl;

        friend class ServiceProvider;
    };

    /**
     * ServiceProvider - The main dependency injection container
     * Stores and manages service instances
     */
    class ServiceProvider
    {
    public:
        ServiceProvider() : pImpl(std::make_shared<ServiceProviderImpl>()) {}

        /**
         * Get a service by its interface/base type
         * @tparam T The type of service to retrieve
         * @return Shared pointer to the service instance
         * @throws std::runtime_error if service not found
         */
        template <typename T>
        std::shared_ptr<T> get() const
        {
            std::type_index typeIdx(typeid(T));
            auto it = pImpl->services.find(typeIdx);

            if (it == pImpl->services.end())
            {
                throw std::runtime_error(
                    std::string("Service not found: ") + typeid(T).name());
            }

            return std::static_pointer_cast<T>(it->second);
        }

        /**
         * Try to get a service, returns nullptr if not found
         * @tparam T The type of service to retrieve
         * @return Shared pointer to the service instance or nullptr
         */
        template <typename T>
        std::shared_ptr<T> tryGet() const
        {
            std::type_index typeIdx(typeid(T));
            auto it = pImpl->services.find(typeIdx);

            if (it == pImpl->services.end())
            {
                return nullptr;
            }

            return std::static_pointer_cast<T>(it->second);
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
            return pImpl->services.find(typeIdx) != pImpl->services.end();
        }

        /**
         * Create a lightweight reference to this provider
         * @return ServiceProviderRef that can be passed to constructors
         */
        ServiceProviderRef ref() const
        {
            return ServiceProviderRef(pImpl);
        }

    private:
        std::shared_ptr<ServiceProviderImpl> pImpl;

        friend class ServiceProviderBuilder;
        friend class ServiceProviderRef;
    };

    // ServiceProviderRef template method implementations (after ServiceProvider is defined)

    template <typename T>
    std::shared_ptr<T> ServiceProviderRef::get() const
    {
        if (!pImpl)
        {
            throw std::runtime_error("ServiceProviderRef is not initialized");
        }

        std::type_index typeIdx(typeid(T));
        auto it = pImpl->services.find(typeIdx);

        if (it == pImpl->services.end())
        {
            throw std::runtime_error(
                std::string("Service not found: ") + typeid(T).name());
        }

        return std::static_pointer_cast<T>(it->second);
    }

    template <typename T>
    std::shared_ptr<T> ServiceProviderRef::tryGet() const
    {
        if (!pImpl)
        {
            return nullptr;
        }

        std::type_index typeIdx(typeid(T));
        auto it = pImpl->services.find(typeIdx);

        if (it == pImpl->services.end())
        {
            return nullptr;
        }

        return std::static_pointer_cast<T>(it->second);
    }

    template <typename T>
    bool ServiceProviderRef::has() const
    {
        if (!pImpl)
        {
            return false;
        }

        std::type_index typeIdx(typeid(T));
        return pImpl->services.find(typeIdx) != pImpl->services.end();
    }

} // namespace dicnew
