#include "log.h"

#include <unistd.h>

#include <cstdlib>
#include <mutex>
#include <system_error>
#include <vector>

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

std::atomic<bool> FreewbLog::s_cleaned{false};

FreewbLog::FreewbLog(const std::string &logFilePath) : m_logFilePath(logFilePath)
{
    init(LogOption());
}

FreewbLog::~FreewbLog()
{
    cleanUp();
}

void FreewbLog::init(const LogOption &option)
{
    spdlog::level::level_enum level = logLevel();

    std::vector<spdlog::sink_ptr> sinks;
    spdlog::sink_ptr console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    sinks.push_back(console_sink);

    try
    {
        spdlog::sink_ptr file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(m_logFilePath, option.fileSize, option.fileCounts, option.rotateEnable);
        sinks.push_back(file_sink);
    }
    catch (const spdlog::spdlog_ex &e)
    {
        spdlog::warn("Failed to create log file:{} Error:{} Falling back to console output only.", m_logFilePath, e.what());
        spdlog::drop(m_logFilePath);
        m_logger = nullptr;
        return;
    }

    m_logger = std::make_shared<spdlog::logger>(m_logFilePath, sinks.begin(), sinks.end());
    m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%!:%#] %v");
    m_logger->set_level(level);
    m_logger->flush_on(level);

    spdlog::register_logger(m_logger);
    spdlog::set_default_logger(m_logger);
    s_cleaned.store(false);
}

void FreewbLog::cleanUp()
{
    static std::mutex cleanup_mutex;
    std::lock_guard<std::mutex> lock(cleanup_mutex);
    if (m_logger == nullptr)
    {
        return;
    }

    s_cleaned.store(true);
    try
    {
        m_logger->flush();
        if (spdlog::get(m_logFilePath))
        {
            spdlog::drop(m_logFilePath);
        }
        m_logger.reset();
    }
    catch (const std::system_error &e)
    {
        std::cerr << "Error during log cleanup (system): " << e.what() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error during log cleanup: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown error during log cleanup" << std::endl;
    }
}

bool FreewbLog::isCleaned()
{
    return s_cleaned.load() == true;
}

const spdlog::level::level_enum FreewbLog::logLevel() const
{
    const char *home = std::getenv("HOME");
    if (!home || !home[0])
    {
        return spdlog::level::warn;
    }
    const std::string path = std::string(home) + "/.local/freewb/enable_debug.txt";
    if (access(path.c_str(), F_OK) == 0)
    {
        return spdlog::level::debug;
    }

    return spdlog::level::warn;
}
