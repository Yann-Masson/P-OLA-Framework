#include <dicnew/ServiceProvider.hpp>
#include <dicnew/ServiceProviderBuilder.hpp>
#include <iostream>
#include <memory>

// Example interfaces and implementations

class ILogger
{
public:
    virtual ~ILogger() = default;
    virtual void log(const std::string &message) = 0;
};

class IDatabase
{
public:
    virtual ~IDatabase() = default;
    virtual void connect() = 0;
};

// Concrete implementations

class ConsoleLogger : public ILogger
{
public:
    void log(const std::string &message) override
    {
        std::cout << "[LOG] " << message << std::endl;
    }
};

class PostgresDatabase : public IDatabase
{
private:
    dicnew::ServiceProviderRef _provider;

public:
    // Constructor with dependency injection
    PostgresDatabase(dicnew::ServiceProviderRef provider) : _provider(provider)
    {
        auto logger = _provider.get<ILogger>();
        logger->log("PostgresDatabase created");
    }

    void connect() override
    {
        auto logger = _provider.get<ILogger>();
        logger->log("Connected to PostgreSQL database");
    }
};

class UserService
{
private:
    dicnew::ServiceProviderRef _provider;

public:
    // Constructor with dependency injection
    UserService(dicnew::ServiceProviderRef provider) : _provider(provider)
    {
        auto logger = _provider.get<ILogger>();
        logger->log("UserService created");
    }

    void createUser(const std::string &username)
    {
        auto logger = _provider.get<ILogger>();
        auto db = _provider.get<IDatabase>();

        db->connect();
        logger->log("Creating user: " + username);
    }
};

int main()
{
    std::cout << "=== DicNew Example ===" << std::endl;
    std::cout << std::endl;

    // Build the service provider
    std::cout << "Building service provider..." << std::endl;
    auto provider = dicnew::ServiceProviderBuilder()
                        .addService<ILogger, ConsoleLogger>()
                        .addService<IDatabase, PostgresDatabase>()
                        .addService<UserService>()
                        .build();

    std::cout << std::endl;
    std::cout << "Services registered successfully!" << std::endl;
    std::cout << std::endl;

    // Use the services
    std::cout << "Using services..." << std::endl;
    auto userService = provider.get<UserService>();
    userService->createUser("john_doe");

    std::cout << std::endl;
    std::cout << "=== Example Complete ===" << std::endl;

    return 0;
}
