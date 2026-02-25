#pragma once

#include "ServiceProvider.hpp"
#include <memory>
#include <type_traits>
#include <concepts>

namespace dicnew
{

    /**
     * ServiceProviderBuilder - Builder pattern for constructing a ServiceProvider
     * Allows fluent registration of services with dependency injection support
     */
    class ServiceProviderBuilder
    {
    public:
        ServiceProviderBuilder()
        {
            provider.pImpl = std::make_shared<ServiceProviderImpl>();
        }

        /**
         * Register a concrete service (no interface)
         * The service will be accessible by its concrete type
         *
         * Service constructor can be:
         * - Default constructor: Service()
         * - Provider constructor: Service(ServiceProviderRef)
         *
         * @tparam TImpl The concrete implementation type
         * @return Reference to this builder for chaining
         */
        template <typename TImpl>
        ServiceProviderBuilder &addService()
        {
            static_assert(!std::is_abstract_v<TImpl>,
                          "Cannot instantiate abstract class. Use addService<TInterface, TImpl>() instead.");

            // Mark for deferred construction
            deferredConstructors.push_back([this]()
                                           {
            auto instance = createInstance<TImpl>();
            provider.pImpl->registerService<TImpl>(instance); });

            return *this;
        }

        /**
         * Register a service with an interface and implementation
         * The service will be accessible by the interface type
         *
         * Implementation constructor can be:
         * - Default constructor: Impl()
         * - Provider constructor: Impl(ServiceProviderRef)
         *
         * @tparam TInterface The interface/base type
         * @tparam TImpl The concrete implementation type
         * @return Reference to this builder for chaining
         */
        template <typename TInterface, typename TImpl>
        ServiceProviderBuilder &addService()
        {
            static_assert(std::is_base_of_v<TInterface, TImpl> || std::is_same_v<TInterface, TImpl>,
                          "TImpl must inherit from TInterface or be the same type");
            static_assert(!std::is_abstract_v<TImpl>,
                          "TImpl cannot be abstract");

            // Mark for deferred construction
            deferredConstructors.push_back([this]()
                                           {
            auto instance = createInstance<TImpl>();
            // Register as interface type
            provider.pImpl->registerService<TInterface>(
                std::static_pointer_cast<TInterface>(instance)
            ); });

            return *this;
        }

        /**
         * Register a pre-constructed service instance
         * Useful for services that require custom initialization
         *
         * @tparam T The type of the service
         * @param instance Pre-constructed shared pointer to the service
         * @return Reference to this builder for chaining
         */
        template <typename T>
        ServiceProviderBuilder &addService(std::shared_ptr<T> instance)
        {
            if (!instance)
            {
                throw std::runtime_error("Cannot register null service instance");
            }

            provider.pImpl->registerService<T>(instance);
            return *this;
        }

        /**
         * Register a service with interface and pre-constructed instance
         *
         * @tparam TInterface The interface/base type
         * @tparam TImpl The concrete implementation type
         * @param instance Pre-constructed shared pointer to the service
         * @return Reference to this builder for chaining
         */
        template <typename TInterface, typename TImpl>
        ServiceProviderBuilder &addService(std::shared_ptr<TImpl> instance)
        {
            static_assert(std::is_base_of_v<TInterface, TImpl> || std::is_same_v<TInterface, TImpl>,
                          "TImpl must inherit from TInterface or be the same type");

            if (!instance)
            {
                throw std::runtime_error("Cannot register null service instance");
            }

            provider.pImpl->registerService<TInterface>(
                std::static_pointer_cast<TInterface>(instance));
            return *this;
        }

        // ===== Multi-Service Registration =====

        /**
         * Register a concrete service into a multi-service collection (no interface)
         * Multiple instances of the same type can be registered and retrieved with getAll<T>()
         *
         * @tparam TImpl The concrete implementation type
         * @return Reference to this builder for chaining
         */
        template <typename TImpl>
        ServiceProviderBuilder &addMultiService()
        {
            static_assert(!std::is_abstract_v<TImpl>,
                          "Cannot instantiate abstract class. Use addMultiService<TInterface, TImpl>() instead.");

            deferredConstructors.push_back([this]()
                                           {
            auto instance = createInstance<TImpl>();
            provider.pImpl->registerMultiService<TImpl>(instance); });

            return *this;
        }

        /**
         * Register a service with an interface into a multi-service collection
         * Multiple implementations of the same interface can be registered
         * and retrieved with getAll<TInterface>()
         *
         * @tparam TInterface The interface/base type
         * @tparam TImpl The concrete implementation type
         * @return Reference to this builder for chaining
         */
        template <typename TInterface, typename TImpl>
        ServiceProviderBuilder &addMultiService()
        {
            static_assert(std::is_base_of_v<TInterface, TImpl> || std::is_same_v<TInterface, TImpl>,
                          "TImpl must inherit from TInterface or be the same type");
            static_assert(!std::is_abstract_v<TImpl>,
                          "TImpl cannot be abstract");

            deferredConstructors.push_back([this]()
                                           {
            auto instance = createInstance<TImpl>();
            provider.pImpl->registerMultiService<TInterface>(
                std::static_pointer_cast<TInterface>(instance)
            ); });

            return *this;
        }

        /**
         * Register a pre-constructed service instance into a multi-service collection
         *
         * @tparam T The type of the service
         * @param instance Pre-constructed shared pointer to the service
         * @return Reference to this builder for chaining
         */
        template <typename T>
        ServiceProviderBuilder &addMultiService(std::shared_ptr<T> instance)
        {
            if (!instance)
            {
                throw std::runtime_error("Cannot register null service instance");
            }

            provider.pImpl->registerMultiService<T>(instance);
            return *this;
        }

        /**
         * Register a service with interface and pre-constructed instance
         * into a multi-service collection
         *
         * @tparam TInterface The interface/base type
         * @tparam TImpl The concrete implementation type
         * @param instance Pre-constructed shared pointer to the service
         * @return Reference to this builder for chaining
         */
        template <typename TInterface, typename TImpl>
        ServiceProviderBuilder &addMultiService(std::shared_ptr<TImpl> instance)
        {
            static_assert(std::is_base_of_v<TInterface, TImpl> || std::is_same_v<TInterface, TImpl>,
                          "TImpl must inherit from TInterface or be the same type");

            if (!instance)
            {
                throw std::runtime_error("Cannot register null service instance");
            }

            provider.pImpl->registerMultiService<TInterface>(
                std::static_pointer_cast<TInterface>(instance));
            return *this;
        }

        /**
         * Build and finalize the ServiceProvider
         * Constructs all deferred services with dependency injection
         *
         * @return The constructed ServiceProvider instance
         */
        ServiceProvider build()
        {
            // Execute all deferred constructors
            // Services can now depend on previously registered services
            for (auto &constructor : deferredConstructors)
            {
                constructor();
            }

            deferredConstructors.clear();

            return provider;
        }

    private:
        /**
         * Create an instance of a service with automatic constructor detection
         * Tries constructors in this order:
         * 1. Constructor taking ServiceProviderRef
         * 2. Default constructor
         */
        template <typename T>
        std::shared_ptr<T> createInstance()
        {
            auto providerRef = provider.ref();

            // Check if constructible with ServiceProviderRef
            if constexpr (std::is_constructible_v<T, ServiceProviderRef>)
            {
                return std::make_shared<T>(providerRef);
            }
            // Check if default constructible
            else if constexpr (std::is_default_constructible_v<T>)
            {
                return std::make_shared<T>();
            }
            else
            {
                static_assert(std::is_constructible_v<T, ServiceProviderRef> ||
                                  std::is_default_constructible_v<T>,
                              "Service must be either default constructible or "
                              "constructible with ServiceProviderRef");
                // This won't compile, but provides a better error message
                return nullptr;
            }
        }

        ServiceProvider provider;
        std::vector<std::function<void()>> deferredConstructors;
    };

} // namespace dicnew
