# DicNew - Simple Dependency Injection Container

A lightweight, header-only C++20 dependency injection container for managing service lifecycles and dependencies.

## Features

- **Simple and Scalable**: Map-based storage instead of complex template metaprogramming
- **Type-Safe**: Full compile-time type checking
- **Header-Only**: No compilation required, just include and use
- **Dependency Injection**: Automatic constructor injection via ServiceProviderRef
- **Fluent API**: Builder pattern for easy service registration
- **No External Dependencies**: Pure C++20 standard library

## Quick Start

### Basic Usage

```cpp
#include <dicnew/ServiceProviderBuilder.hpp>
#include <dicnew/ServiceProvider.hpp>

// Define your services
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

// Build the container
auto provider = dicnew::ServiceProviderBuilder()
    .addService<ILogger, ConsoleLogger>()
    .build();

// Retrieve services
auto logger = provider.get<ILogger>();
logger->log("Hello, DicNew!");
```

### Constructor Injection

Services can receive a `ServiceProviderRef` to access other services:

```cpp
class DatabaseService {
public:
    explicit DatabaseService(dicnew::ServiceProviderRef provider) 
        : _provider(provider) {
        // Access other services during construction
        auto logger = _provider.get<ILogger>();
        logger->log("DatabaseService initialized");
    }

private:
    dicnew::ServiceProviderRef _provider;
};

// Register with automatic injection
auto provider = dicnew::ServiceProviderBuilder()
    .addService<ILogger, ConsoleLogger>()
    .addService<DatabaseService>()  // Automatically receives provider
    .build();
```

### Register Pre-Constructed Instances

```cpp
auto customLogger = std::make_shared<ConsoleLogger>();

auto provider = dicnew::ServiceProviderBuilder()
    .addService<ILogger>(customLogger)
    .build();
```

## API Reference

### ServiceProviderBuilder

#### `addService<TImpl>()`
Register a concrete service without an interface.

#### `addService<TInterface, TImpl>()`
Register a service with an interface type.

#### `addService<T>(std::shared_ptr<T>)`
Register a pre-constructed instance.

#### `build()`
Finalize and return the ServiceProvider.

### ServiceProvider

#### `get<T>()`
Get a service by type. Throws if not found.

#### `tryGet<T>()`
Get a service by type. Returns nullptr if not found.

#### `has<T>()`
Check if a service is registered.

#### `ref()`
Get a lightweight reference to pass to constructors.

### ServiceProviderRef

Lightweight reference that can be passed to service constructors. Provides the same `get()`, `tryGet()`, and `has()` methods as ServiceProvider.

## Building

DicNew is header-only, but includes CMake support:

```bash
mkdir build && cd build
cmake ..
make
make install
```

## Requirements

- C++20 compatible compiler
- CMake 3.20 or higher (for building/installing)

## License

MIT License
