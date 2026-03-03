#include <memory>
#include <iostream>
#include <forge/provider.hpp>
#include <forge/provider_builder.hpp>

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
    forge::ProviderRef _provider;

public:
    // Constructor with dependency injection
    PostgresDatabase(forge::ProviderRef provider) : _provider(provider)
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
    forge::ProviderRef _provider;

public:
    // Constructor with dependency injection
    UserService(forge::ProviderRef provider) : _provider(provider)
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

// Example for multi-service feature
class IPlugin
{
public:
    virtual ~IPlugin() = default;
    virtual void execute() = 0;
    virtual std::string getName() const = 0;
};

class EmailPlugin : public IPlugin
{
public:
    void execute() override
    {
        std::cout << "  [EmailPlugin] Sending email notification..." << std::endl;
    }
    std::string getName() const override { return "EmailPlugin"; }
};

class SlackPlugin : public IPlugin
{
public:
    void execute() override
    {
        std::cout << "  [SlackPlugin] Posting to Slack..." << std::endl;
    }
    std::string getName() const override { return "SlackPlugin"; }
};

class LogPlugin : public IPlugin
{
public:
    void execute() override
    {
        std::cout << "  [LogPlugin] Writing to log file..." << std::endl;
    }
    std::string getName() const override { return "LogPlugin"; }
};

int main()
{
    std::cout << "=== Forge Example ===" << std::endl;
    std::cout << std::endl;

    // Build the service provider
    std::cout << "Building service provider..." << std::endl;
    auto provider = forge::ProviderBuilder()
                        .addService<ILogger, ConsoleLogger>()
                        .addService<IDatabase, PostgresDatabase>()
                        .addService<UserService>()
                        // Register multiple plugins using addService
                        .addService<IPlugin, EmailPlugin>()
                        .addService<IPlugin, SlackPlugin>()
                        .addService<IPlugin, LogPlugin>()
                        .build();

    std::cout << std::endl;
    std::cout << "Services registered successfully!" << std::endl;
    std::cout << std::endl;

    // Use the services
    std::cout << "Using services..." << std::endl;
    auto userService = provider.get<UserService>();
    userService->createUser("john_doe");

    std::cout << std::endl;
    std::cout << "=== Multi-Service Example ===" << std::endl;
    std::cout << "Retrieving all plugins..." << std::endl;
    auto plugins = provider.getAll<IPlugin>();
    std::cout << "Found " << plugins.size() << " plugins:" << std::endl;
    for (const auto& plugin : plugins)
    {
        std::cout << "Running " << plugin->getName() << ":" << std::endl;
        plugin->execute();
    }

    std::cout << std::endl;
    std::cout << "=== Example Complete ===" << std::endl;

    return 0;
}
