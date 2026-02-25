![Forge Logo](./assets/forge.png)

# Forge - Simple Dependency Injection Container

A lightweight, header-only C++20 dependency injection container for managing service lifecycles and dependencies.

## Features

- **Simple and Scalable**: Map-based storage instead of complex template metaprogramming
- **Type-Safe**: Full compile-time type checking
- **Header-Only**: No compilation required, just include and use
- **Dependency Injection**: Automatic constructor injection via `ProviderRef`
- **Multi-Service Support**: Register multiple implementations under the same interface and retrieve them all at once
- **Fluent API**: Builder pattern for easy service registration
- **Deferred Construction**: Services are constructed only when `build()` is called, so all registrations are available during construction
- **No External Dependencies**: Pure C++20 standard library

## Quick Start

### Basic Usage

```cpp
#include <forge/provider_builder.hpp>
#include <forge/provider.hpp>

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(const std::string& msg) = 0;
};

class ConsoleLogger : public ILogger {
public:
    void log(const std::string& msg) override {
        std::cout << msg << std::endl;
    }
};

auto provider = forge::ProviderBuilder()
    .addService<ILogger, ConsoleLogger>()
    .build();

auto logger = provider.get<ILogger>();
logger->log("Hello, Forge!");
```

### Constructor Injection

Services can receive a `ProviderRef` to access other registered services at construction time:

```cpp
class DatabaseService {
public:
    explicit DatabaseService(forge::ProviderRef provider) : _provider(provider) {
        auto logger = _provider.get<ILogger>();
        logger->log("DatabaseService initialized");
    }

    void query() {
        auto logger = _provider.get<ILogger>();
        logger->log("Executing query...");
    }

private:
    forge::ProviderRef _provider;
};

auto provider = forge::ProviderBuilder()
    .addService<ILogger, ConsoleLogger>()
    .addService<DatabaseService>()  // Automatically receives ProviderRef
    .build();
```

A service constructor is detected in this order:
1. `Service(forge::ProviderRef)` — preferred when you need dependencies
2. `Service()` — default constructor as a fallback

### Register Pre-Constructed Instances

```cpp
auto customLogger = std::make_shared<ConsoleLogger>();

auto provider = forge::ProviderBuilder()
    .addService<ILogger>(customLogger)
    .build();
```

### Multi-Service Registration

Register multiple implementations under the same interface and retrieve them all with `getAll<T>()`:

```cpp
class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual void execute() = 0;
};

class EmailPlugin : public IPlugin { /* ... */ };
class SlackPlugin : public IPlugin { /* ... */ };
class LogPlugin   : public IPlugin { /* ... */ };

auto provider = forge::ProviderBuilder()
    .addMultiService<IPlugin, EmailPlugin>()
    .addMultiService<IPlugin, SlackPlugin>()
    .addMultiService<IPlugin, LogPlugin>()
    .build();

auto plugins = provider.getAll<IPlugin>();
for (const auto& plugin : plugins) {
    plugin->execute();
}
```

## API Reference

### ProviderBuilder

#### `addService<TImpl>()`
Register a concrete service without an interface. Accessible by its concrete type.

#### `addService<TInterface, TImpl>()`
Register a service mapped to an interface type. Accessible by `TInterface`.

#### `addService<T>(std::shared_ptr<T>)`
Register a pre-constructed instance by its own type.

#### `addService<TInterface, TImpl>(std::shared_ptr<TImpl>)`
Register a pre-constructed instance mapped to an interface type.

#### `addMultiService<TImpl>()`
Register a concrete service into a multi-service collection. Retrieve with `getAll<TImpl>()`.

#### `addMultiService<TInterface, TImpl>()`
Register a service with an interface into a multi-service collection. Retrieve with `getAll<TInterface>()`.

#### `addMultiService<T>(std::shared_ptr<T>)`
Register a pre-constructed instance into a multi-service collection.

#### `addMultiService<TInterface, TImpl>(std::shared_ptr<TImpl>)`
Register a pre-constructed instance mapped to an interface into a multi-service collection.

#### `build()`
Finalize and return the `Provider`. All deferred services are constructed at this point.

---

### Provider

#### `get<T>()`
Get a single-registered service by type. Throws `std::runtime_error` if not found.

#### `tryGet<T>()`
Get a single-registered service by type. Returns `nullptr` if not found.

#### `has<T>()`
Check if a single service is registered under type `T`. Returns `bool`.

#### `getAll<T>()`
Get all services registered under type `T` via `addMultiService`. Returns an empty vector if none are found.

#### `ref()`
Get a lightweight `ProviderRef` to pass to service constructors.

---
