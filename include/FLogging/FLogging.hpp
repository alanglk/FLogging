/**
 * @file FLogging.hpp
 * @author alangln (https://github.com/alanglk)
 * @brief Flexible Logging Library Core Header
 */

#pragma once

#include <functional>
#include <string>
#include <cstdint>
#include "FLogging/flogging_export.h"


// Undefine existing pollution
#ifdef LOG_INFO
    #undef LOG_INFO 
#endif
#ifdef LOG_DEBUG 
    #undef LOG_DEBUG 
#endif
#ifdef LOG_WARN 
    #undef LOG_WARN 
#endif
#ifdef LOG_ERROR 
    #undef LOG_ERROR 
#endif
#ifdef LOG_METRIC 
    #undef LOG_METRIC 
#endif


namespace flogging {
    enum class LogLevel : uint8_t { Trace, Debug, Info, Warn, Error, Critical };
    enum class LoggerType : uint8_t { None, Normal, Metrics };
    
    // New Backend selection
    enum class BackendType : uint8_t { 
        StdFormat, 
        Quill // Will gracefully fallback to StdFormat if compiled without Quill support
    };

    using LogCallback = std::function<void(LogLevel, const std::string&)>;

    // Set up logger sinks
    FLOGGING_API void AddConsoleSink(LogLevel level = LogLevel::Info, LoggerType target = LoggerType::Normal);
    FLOGGING_API void AddFileSink(const std::string& file_path, LogLevel level = LogLevel::Debug, LoggerType target = LoggerType::Normal);
    FLOGGING_API void AddNetworkUDPSink(int port, LogLevel level = LogLevel::Info, LoggerType target = LoggerType::Normal);
    FLOGGING_API void AddCallbackSink(LogCallback callback, LogLevel level = LogLevel::Info, LoggerType target = LoggerType::Normal);

    // Initialize logger with desired backend (defaults to Quill if available, else StdFormat)
    FLOGGING_API void InitLogger(LogLevel overall_level = LogLevel::Debug, BackendType backend = BackendType::Quill);
    FLOGGING_API void ResetLogger(LoggerType target = LoggerType::None);

    FLOGGING_API LogLevel GetLogLevel(LoggerType target = LoggerType::Normal);
    FLOGGING_API void SetLogLevel(LogLevel level, LoggerType target = LoggerType::Normal);

    // Backend state access
    FLOGGING_API BackendType GetActiveBackend();
    
    // StdFormat Internal State
    extern FLOGGING_API LogLevel g_active_log_level; 
    extern FLOGGING_API LogLevel g_metrics_log_level;
    FLOGGING_API void DispatchLog(LogLevel level, const std::string& message);
    FLOGGING_API void DispatchMetric(const std::string& message);
}



// --- Internal Macro Implementations ------------------------------

#include <format>

#ifdef ENABLE_QUILL
    #define QUILL_DISABLE_NON_PREFIXED_MACROS
    #include <quill/Frontend.h>
    #include <quill/LogMacros.h>
    
    namespace flogging {
        FLOGGING_API quill::Logger* get_logger();
        FLOGGING_API quill::Logger* get_metrics_logger();
    }

    #define FLOGGING_INTERNAL_INFO(fmt, ...) do { \
        if (flogging::GetActiveBackend() == flogging::BackendType::Quill) { \
            QUILL_LOG_INFO(flogging::get_logger(), fmt __VA_OPT__(,) __VA_ARGS__); \
        } else if (flogging::g_active_log_level <= flogging::LogLevel::Info) { \
            flogging::DispatchLog(flogging::LogLevel::Info, std::format("[INFO] " fmt __VA_OPT__(,) __VA_ARGS__)); \
        } \
    } while(0)

    #define FLOGGING_INTERNAL_DEBUG(fmt, ...) do { \
        if (flogging::GetActiveBackend() == flogging::BackendType::Quill) { \
            QUILL_LOG_DEBUG(flogging::get_logger(), fmt __VA_OPT__(,) __VA_ARGS__); \
        } else if (flogging::g_active_log_level <= flogging::LogLevel::Debug) { \
            flogging::DispatchLog(flogging::LogLevel::Debug, std::format("[DEBUG] " fmt __VA_OPT__(,) __VA_ARGS__)); \
        } \
    } while(0)

    #define FLOGGING_INTERNAL_WARN(fmt, ...) do { \
        if (flogging::GetActiveBackend() == flogging::BackendType::Quill) { \
            QUILL_LOG_ERROR(flogging::get_logger(), fmt __VA_OPT__(,) __VA_ARGS__); \
        } else if (flogging::g_active_log_level <= flogging::LogLevel::Warn) { \
            flogging::DispatchLog(flogging::LogLevel::Warn, std::format("[WARN] " fmt __VA_OPT__(,) __VA_ARGS__)); \
        } \
    } while(0)

    #define FLOGGING_INTERNAL_ERROR(fmt, ...) do { \
        if (flogging::GetActiveBackend() == flogging::BackendType::Quill) { \
            QUILL_LOG_ERROR(flogging::get_logger(), fmt __VA_OPT__(,) __VA_ARGS__); \
        } else if (flogging::g_active_log_level <= flogging::LogLevel::Error) { \
            flogging::DispatchLog(flogging::LogLevel::Error, std::format("[ERROR] " fmt __VA_OPT__(,) __VA_ARGS__)); \
        } \
    } while(0)

    #define FLOGGING_INTERNAL_METRIC(fmt, ...) do { \
        if (flogging::GetActiveBackend() == flogging::BackendType::Quill) { \
            QUILL_LOG_INFO(flogging::get_metrics_logger(), fmt __VA_OPT__(,) __VA_ARGS__); \
        } else if (flogging::g_metrics_log_level <= flogging::LogLevel::Info) { \
            flogging::DispatchMetric(std::format("[METRIC] " fmt __VA_OPT__(,) __VA_ARGS__)); \
        } \
    } while(0)

#else
    // std::format implementations
    #define FLOGGING_INTERNAL_INFO(fmt, ...) do { \
        if (flogging::g_active_log_level <= flogging::LogLevel::Info) { \
            flogging::DispatchLog(flogging::LogLevel::Info, std::format("[INFO] " fmt __VA_OPT__(,) __VA_ARGS__)); \
        } \
    } while(0)

    #define FLOGGING_INTERNAL_DEBUG(fmt, ...) do { \
        if (flogging::g_active_log_level <= flogging::LogLevel::Debug) { \
            flogging::DispatchLog(flogging::LogLevel::Debug, std::format("[DEBUG] " fmt __VA_OPT__(,) __VA_ARGS__)); \
        } \
    } while(0)

    #define FLOGGING_INTERNAL_WARN(fmt, ...) do { \
        if (flogging::g_active_log_level <= flogging::LogLevel::Warn) { \
            flogging::DispatchLog(flogging::LogLevel::Warn, std::format("[WARN] " fmt __VA_OPT__(,) __VA_ARGS__)); \
        } \
    } while(0)

    #define FLOGGING_INTERNAL_ERROR(fmt, ...) do { \
        if (flogging::g_active_log_level <= flogging::LogLevel::Error) { \
            flogging::DispatchLog(flogging::LogLevel::Error, std::format("[ERROR] " fmt __VA_OPT__(,) __VA_ARGS__)); \
        } \
    } while(0)

    #define FLOGGING_INTERNAL_METRIC(fmt, ...) do { \
        if (flogging::g_metrics_log_level <= flogging::LogLevel::Info) { \
            flogging::DispatchMetric(std::format("[METRIC] " fmt __VA_OPT__(,) __VA_ARGS__)); \
        } \
    } while(0)

#endif


// --- User Macros -------------------------------------------------
#ifdef FLOGGING_USE_F_PREFIX
    #define F_LOG_INFO(...)   FLOGGING_INTERNAL_INFO(__VA_ARGS__)
    #define F_LOG_ERROR(...)  FLOGGING_INTERNAL_ERROR(__VA_ARGS__)
    #define F_LOG_METRIC(...) FLOGGING_INTERNAL_METRIC(__VA_ARGS__)
#else
    #define LOG_INFO(...)   FLOGGING_INTERNAL_INFO(__VA_ARGS__)
    #define LOG_ERROR(...)  FLOGGING_INTERNAL_ERROR(__VA_ARGS__)
    #define LOG_METRIC(...) FLOGGING_INTERNAL_METRIC(__VA_ARGS__)
#endif