#pragma once

#include "provider.hpp"
#include <memory>
#include <type_traits>
#include <functional>

namespace forge
{

    /**
     * ProviderBuilder - Builder pattern for constructing a Provider
     * Allows fluent registration of services with dependency injection support
     */
    class ProviderBuilder
    {
    public:
        ProviderBuilder()
        {
            provider.pImpl = std::make_shared<ProviderImpl>();
        }

        /**
         * Register a concrete service (no interface)
         * The service will be accessible by its concrete type
         *
         * Service constructor can be:
         * - Default constructor: Service()
         * - Provider constructor: Service(ProviderRef)
         *
         * @tparam TImpl The concrete implementation type
         * @return Reference to this builder for chaining
         */
        template <typename TImpl>
        ProviderBuilder &addService()
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
         * Automatically registers under BOTH interface and concrete types
         * The service can be retrieved via get<TInterface>() or getAll<TInterface>()
         *
         * Implementation constructor can be:
         * - Default constructor: Impl()
         * - Provider constructor: Impl(ProviderRef)
         *
         * @tparam TInterface The interface/base type
         * @tparam TImpl The concrete implementation type
         * @return Reference to this builder for chaining
         */
        template <typename TInterface, typename TImpl>
        ProviderBuilder &addService()
        {
            static_assert(std::is_base_of_v<TInterface, TImpl> || std::is_same_v<TInterface, TImpl>,
                          "TImpl must inherit from TInterface or be the same type");
            static_assert(!std::is_abstract_v<TImpl>,
                          "TImpl cannot be abstract");

            // Mark for deferred construction with dual registration
            deferredConstructors.push_back([this]()
                                           {
            auto instance = createInstance<TImpl>();
            // Register under interface type
            provider.pImpl->registerMultiService<TInterface>(
                std::static_pointer_cast<TInterface>(instance)
            );
            // Also register under concrete type
            provider.pImpl->registerMultiService<TImpl>(instance); });

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
        ProviderBuilder &addService(std::shared_ptr<T> instance)
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
         * Automatically registers under BOTH interface and concrete types
         *
         * @tparam TInterface The interface/base type
         * @tparam TImpl The concrete implementation type
         * @param instance Pre-constructed shared pointer to the service
         * @return Reference to this builder for chaining
         */
        template <typename TInterface, typename TImpl>
        ProviderBuilder &addService(std::shared_ptr<TImpl> instance)
        {
            static_assert(std::is_base_of_v<TInterface, TImpl> || std::is_same_v<TInterface, TImpl>,
                          "TImpl must inherit from TInterface or be the same type");

            if (!instance)
            {
                throw std::runtime_error("Cannot register null service instance");
            }

            // Register under interface type
            provider.pImpl->registerMultiService<TInterface>(
                std::static_pointer_cast<TInterface>(instance));
            // Also register under concrete type
            provider.pImpl->registerMultiService<TImpl>(instance);
            return *this;
        }

        /**
         * Register a service using a factory function
         * Automatically registers under BOTH interface and concrete types
         * Factory receives ProviderRef and should return a shared_ptr to the implementation
         *
         * @tparam TInterface The interface/base type
         * @tparam TImpl The concrete implementation type
         * @param factory Function that creates the service instance
         * @return Reference to this builder for chaining
         */
        template <typename TInterface, typename TImpl>
        ProviderBuilder &addService(std::function<std::shared_ptr<TImpl>(ProviderRef)> factory)
        {
            static_assert(std::is_base_of_v<TInterface, TImpl> || std::is_same_v<TInterface, TImpl>,
                          "TImpl must inherit from TInterface or be the same type");
            static_assert(!std::is_abstract_v<TImpl>,
                          "TImpl cannot be abstract");

            deferredConstructors.push_back([this, factory]()
                                           {
            auto instance = factory(provider.ref());
            // Register under interface type
            provider.pImpl->registerMultiService<TInterface>(
                std::static_pointer_cast<TInterface>(instance)
            );
            // Also register under concrete type
            provider.pImpl->registerMultiService<TImpl>(instance); });

            return *this;
        }

        /**
         * Build and finalize the Provider
         * Constructs all deferred services with dependency injection
         *
         * @return The constructed Provider instance
         */
        Provider build()
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
         * 1. Constructor taking ProviderRef
         * 2. Default constructor
         */
        template <typename T>
        std::shared_ptr<T> createInstance()
        {
            auto providerRef = provider.ref();

            // Check if constructible with ProviderRef
            if constexpr (std::is_constructible_v<T, ProviderRef>)
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
                static_assert(std::is_constructible_v<T, ProviderRef> ||
                                  std::is_default_constructible_v<T>,
                              "Service must be either default constructible or "
                              "constructible with ProviderRef");
                // This won't compile, but provides a better error message
                return nullptr;
            }
        }

        Provider provider;
        std::vector<std::function<void()>> deferredConstructors;
    };

} // namespace forge
