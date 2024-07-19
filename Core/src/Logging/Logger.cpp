#include "Logger.h"

std::shared_ptr<spdlog::logger> Logger::m_serverLogger;
std::shared_ptr<spdlog::logger> Logger::m_clientLogger;

void Logger::init()
{
    std::vector<spdlog::sink_ptr> logSinks;
    logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("SERVER.log", true));

    logSinks[0]->set_pattern("%^[%T] %n: %v%$");
    logSinks[1]->set_pattern("[%T] [%l] %n: %v");

    m_serverLogger = std::make_shared<spdlog::logger>("SERVER", begin(logSinks), end(logSinks));
    spdlog::register_logger(m_serverLogger);
    m_serverLogger->set_level(spdlog::level::trace);
    m_serverLogger->flush_on(spdlog::level::trace);

    m_clientLogger = std::make_shared<spdlog::logger>("CLIENT", begin(logSinks), end(logSinks));
    spdlog::register_logger(m_clientLogger);
    m_clientLogger->set_level(spdlog::level::trace);
    m_clientLogger->flush_on(spdlog::level::trace);
}
