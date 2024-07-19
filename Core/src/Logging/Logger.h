#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

class Logger
{
public:
    static void init();

    static inline std::shared_ptr<spdlog::logger>& getServerLogger() { return m_serverLogger; }
    static inline std::shared_ptr<spdlog::logger>& getClientLogger() { return m_clientLogger; }

private:
    static std::shared_ptr<spdlog::logger> m_serverLogger;
    static std::shared_ptr<spdlog::logger> m_clientLogger;
};

#ifdef DEBUG
// SPDLog Macros for the SERVER Library
#define SERVER_TRACE(...)    ::Logger::getServerLogger()->trace(__VA_ARGS__)
#define SERVER_INFO(...)     ::Logger::getServerLogger()->info(__VA_ARGS__)
#define SERVER_WARN(...)     ::Logger::getServerLogger()->warn(__VA_ARGS__)
#define SERVER_ERROR(...)    ::Logger::getServerLogger()->error(__VA_ARGS__)
#define SERVER_CRITICAL(...) ::Logger::getServerLogger()->critical(__VA_ARGS__)

// SPDLog Macros for projects made with the SERVER Libraries 
#define CLIENT_TRACE(...)    ::Logger::getClientLogger()->trace(__VA_ARGS__)
#define CLIENT_INFO(...)     ::Logger::getClientLogger()->info(__VA_ARGS__)
#define CLIENT_WARN(...)     ::Logger::getClientLogger()->warn(__VA_ARGS__)
#define CLIENT_ERROR(...)    ::Logger::getClientLogger()->error(__VA_ARGS__)
#define CLIENT_CRITICAL(...) ::Logger::getClientLogger()->critical(__VA_ARGS__)
#else
// SPDLog Macros for the SERVER Library
#define SERVER_TRACE(...)
#define SERVER_INFO(...)
#define SERVER_WARN(...)
#define SERVER_ERROR(...)
#define SERVER_CRITICAL(...)

// SPDLog Macros for projects made with the SERVER Libraries
#define CLIENT_TRACE(...) 
#define CLIENT_INFO(...)
#define CLIENT_WARN(...)
#define CLIENT_ERROR(...)
#define CLIENT_CRITICAL(...) 
#endif
