#ifndef __SYSTEM_MONITOR_HPP__
#define __SYSTEM_MONITOR_HPP__

#include <stdio.h>
#include "system_clock.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 系统
namespace system
{ /**
   * @brief 类对象 系统监视器
   *
   */
class System_Monitor
{
  /**
   * @brief 系统监视器 日志最大数量
   */
  static constexpr uint32_t SYSTEM_LOG_MAX_COUNT          = 64;

  /**
   * @brief 系统监视器 日志输出缓冲区大小
   */
  static constexpr uint32_t SYSTEM_LOG_OUTPUT_BUFFER_SIZE = 512;

public:
  /**
   * @brief 系统监视器 日志类型
   */
  enum class Log_Type : uint32_t
  {
    ERROR   = 0x01, /* 错误 */
    WARNING = 0x02, /* 警告 */
    INFO    = 0x04, /* 信息 */
  };

  /**
   * @brief 系统监视器 日志结构体
   */
  struct Log_t
  {
    Log_Type type; /* 日志类型 */
    uint32_t time; /* 时间戳 */
    uint32_t code; /* 错误代码 */
  };

  /// @brief 系统监视器 日志输出函数类型
  using Log_Output_Func = void (*)(const char* log, uint32_t length);

private:
  /**
   * @brief 系统监视器 日志缓存
   */
  static inline Log_t m_logs[SYSTEM_LOG_MAX_COUNT]                  = {};

  /**
   * @brief 系统监视器 日志输出函数
   */
  static inline Log_Output_Func m_output_func                       = nullptr;

  /**
   * @brief 系统监视器 日志缓存索引
   */
  static inline uint32_t m_index                                    = 0;

  /**
   * @brief 系统监视器 日志输出缓冲区
   */
  static inline char m_output_buffer[SYSTEM_LOG_OUTPUT_BUFFER_SIZE] = {};

public:
  /**
   * @brief 系统监视器 设置日志输出函数
   * @param func 日志输出函数
   */
  static void QAQ_O3 set_output_func(Log_Output_Func func) noexcept
  {
    m_output_func = func;
  }

  /**
   * @brief 系统监视器 记录错误信息
   *
   * @param error_code  错误代码
   * @param log         错误日志信息
   */
  template <typename T>
  static void QAQ_O3 log_error(T error_code, const char* log)
  {
    m_logs[m_index].type = Log_Type::ERROR;
    m_logs[m_index].time = tx_time_get() * 1000 / TX_TIMER_TICKS_PER_SECOND;
    m_logs[m_index].code = static_cast<uint32_t>(error_code);

    m_index              = (m_index + 1) % SYSTEM_LOG_MAX_COUNT;

#ifdef DEMO_DEBUG
    if (m_output_func != nullptr)
    {
      kernel::System_Clock::Time_t time   = kernel::System_Clock::now_time();
      int                          length = sprintf(m_output_buffer, "%lu ERROR:[%02lu:%02lu:%02lu] %s\n", static_cast<uint32_t>(error_code), time.hour, time.minute, time.second, log);
      m_output_buffer[length]             = '\0';
      m_output_func(m_output_buffer, length);
    }

    while (1)
    {
    }
#endif
  }

  /**
   * @brief 系统监视器 记录警告信息
   *
   * @param warning_code  警告代码
   * @param log           警告日志信息
   */
  template <typename T>
  static void QAQ_O3 log_warning(T warning_code, const char* log)
  {
    m_logs[m_index].type = Log_Type::WARNING;
    m_logs[m_index].time = tx_time_get() * 1000 / TX_TIMER_TICKS_PER_SECOND;
    m_logs[m_index].code = static_cast<uint32_t>(warning_code);

    m_index              = (m_index + 1) % SYSTEM_LOG_MAX_COUNT;

#ifdef DEMO_DEBUG
    if (m_output_func != nullptr)
    {
      kernel::System_Clock::Time_t time   = kernel::System_Clock::now_time();
      int                          length = sprintf(m_output_buffer, "%lu WARNING:[%02lu:%02lu:%02lu] %s\n", static_cast<uint32_t>(warning_code), time.hour, time.minute, time.second, log);
      m_output_buffer[length]             = '\0';
      m_output_func(m_output_buffer, length);
    }
#endif
  }

  /**
   * @brief 系统监视器 记录信息
   *
   * @param line_number     行数
   * @param function_name   函数名
   * @param log             日志
   */
  static void QAQ_O3 log_info(int line_number, const char* function_name, const char* log)
  {
#ifdef DEMO_DEBUG
    if (m_output_func != nullptr)
    {
      kernel::System_Clock::Time_t time   = kernel::System_Clock::now_time();
      int                          length = 0;

      if (log)
      {
        length = sprintf(m_output_buffer, "[%02lu:%02lu:%02lu.%03lu] %5d : %s() %s", time.hour, time.minute, time.second, time.millisecond, line_number, function_name, log);
      }
      else
      {
        length = sprintf(m_output_buffer, "[%02lu:%02lu:%02lu.%03lu] %5d : %s()\n", time.hour, time.minute, time.second, time.millisecond, line_number, function_name);
      }

      m_output_buffer[length] = '\0';
      m_output_func(m_output_buffer, length);
    }
#endif
  }

  /**
   * @brief 系统监视器 记录信息
   *
   * @param line_number     行数
   * @param function_name   函数名
   * @param format          格式化字符串
   * @param ...             参数
   */
  static void QAQ_O3 log_format(int line_number, const char* function_name, const char* format, ...)
  {
#ifdef DEMO_DEBUG
    if (m_output_func != nullptr)
    {
      if (format)
      {
        va_list args;
        va_start(args, format);

        char log_buf[256];
        vsnprintf(log_buf, sizeof(log_buf), format, args);

        va_end(args);

        log_info(line_number, function_name, log_buf);
      }
      else
      {
        log_info(line_number, function_name, nullptr);
      }
    }
#endif
  }

  /**
   * @brief 全局系统监视器 函数状态检查
   *
   * @note  用于检查ThreadX API调用的状态，并记录错误日志
   * @param status  ThreadX API调用的状态
   * @param message 错误日志信息
   */
  static void QAQ_O3 check_status(uint32_t status, const char* message) noexcept
  {
    if (status != TX_SUCCESS)
    {
      log_error(status, message);
    }
  }

  /**
   * @brief 全局系统监视器 安全执行
   *
   * @note   用于安全执行ThreadX API调用，并记录错误日志
   * @tparam Func     函数类型
   * @tparam Args     参数类型
   * @param  func     函数对象
   * @param  args     函数参数
   * @return uint32_t 函数调用的状态
   */
  template <typename Func, typename... Args>
  static uint32_t QAQ_O3 safe_execute(Func&& func, Args&&... args) noexcept
  {
    const UINT result = func(std::forward<Args>(args)...);
    check_status(result, "ThreadX API call failed");
    return result;
  }
};
} /* namespace system */
} /* namespace QAQ */

#define QAQ_ERROR_LOG(code, logs)   QAQ::system::System_Monitor::log_error(code, logs)
#define QAQ_WARNING_LOG(code, logs) QAQ::system::System_Monitor::log_warning(code, logs)
#define QAQ_INFO_INFO()             QAQ::system::System_Monitor::log_info(__LINE__, __FUNCTION__, nullptr)
#define QAQ_INFO_LOG(fmt, ...)      QAQ::system::System_Monitor::log_format(__LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)

#endif /* __SYSTEM_MONITOR_HPP__ */
