#ifndef _LOG_H_
#define _LOG_H_

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include <atomic>
#include <iostream>
#include <memory>

#include <spdlog/spdlog.h>

#define FREEWB_DEBUG(...)                                                                                                                                                                                                                                                                                  \
    do                                                                                                                                                                                                                                                                                                     \
    {                                                                                                                                                                                                                                                                                                      \
        if (!FreewbLog::isCleaned() && spdlog::default_logger_raw() != nullptr)                                                                                                                                                                                                                            \
        {                                                                                                                                                                                                                                                                                                  \
            SPDLOG_LOGGER_DEBUG(spdlog::default_logger_raw(), __VA_ARGS__);                                                                                                                                                                                                                                \
        }                                                                                                                                                                                                                                                                                                  \
    } while (0)

#define FREEWB_WARN(...)                                                                                                                                                                                                                                                                                   \
    do                                                                                                                                                                                                                                                                                                     \
    {                                                                                                                                                                                                                                                                                                      \
        if (!FreewbLog::isCleaned() && spdlog::default_logger_raw() != nullptr)                                                                                                                                                                                                                            \
        {                                                                                                                                                                                                                                                                                                  \
            SPDLOG_LOGGER_WARN(spdlog::default_logger_raw(), __VA_ARGS__);                                                                                                                                                                                                                                 \
        }                                                                                                                                                                                                                                                                                                  \
    } while (0)

#define FREEWB_ERROR(...)                                                                                                                                                                                                                                                                                  \
    do                                                                                                                                                                                                                                                                                                     \
    {                                                                                                                                                                                                                                                                                                      \
        if (!FreewbLog::isCleaned() && spdlog::default_logger_raw() != nullptr)                                                                                                                                                                                                                            \
        {                                                                                                                                                                                                                                                                                                  \
            SPDLOG_LOGGER_ERROR(spdlog::default_logger_raw(), __VA_ARGS__);                                                                                                                                                                                                                                \
        }                                                                                                                                                                                                                                                                                                  \
    } while (0)

class FreewbLog
{
public:
    explicit FreewbLog(const std::string &logFilePath = "/tmp/freewb.log");
    ~FreewbLog();

    class LogOption
    {
    public:
        long long fileSize = 1024 * 1024 * 10;
        int fileCounts = 1;
        bool rotateEnable = false;
    };

    static bool isCleaned();

private:
    void init(const LogOption &option);
    void cleanUp();
    const spdlog::level::level_enum logLevel() const;

private:
    std::shared_ptr<spdlog::logger> m_logger;
    static std::atomic<bool> s_cleaned;
    std::string m_logFilePath;
};

#endif