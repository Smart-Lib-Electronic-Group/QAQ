#ifndef __SYSTEM_INCLUDE_HPP__
#define __SYSTEM_INCLUDE_HPP__

#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <atomic>
#include <tuple>
#include <typeinfo>
#include <type_traits>
#include <string>

#include "tx_api.h"
#include "stm32h7xx.h"
#include "user_config.h"

#include "SEGGER_RTT.h"
#include "SEGGER_SYSVIEW.h"

extern TX_THREAD _tx_timer_thread;

/// @brief 宏定义 系统是否处于中断中
#define IS_IN_ISR   (__get_IPSR() != 0)
/// @brief 宏定义 系统是否处于软件定时器中
#define IS_IN_TIMER (tx_thread_identify() == &_tx_timer_thread)
// /// @brief 宏定义 内联函数
// #define INLINE      __attribute__((always_inline)) inline
/// @brief 宏定义 弱函数
#define WEAK        __attribute__((weak))
/// @brief 宏定义 对齐至N字节
#define ALIGN(N)    __attribute__((aligned(N)))
/// @brief 宏定义 结构体对齐
#define PACKED      __attribute__((packed))
/// @brief 宏定义 -O3优化
#define O3          __attribute__((optimize("O3")))
/// @brief 宏定义 置于DTCM中运行
#define DTCM        __attribute__((section(".dtcm")))
/// @brief 宏定义 置于SRAM1中运行
#define SRAM1       __attribute__((section(".sram1")))
/// @brief 宏定义 置于SRAM2中运行
#define SRAM2       __attribute__((section(".sram2")))
/// @brief 宏定义 置于SRAM3中运行
#define SRAM3       __attribute__((section(".sram3")))
/// @brief 宏定义 置于SRAM4中运行
#define SRAM4       __attribute__((section(".sram4")))

/// @brief 宏定义 临界区起点
#define start_critical() \
  TX_INTERRUPT_SAVE_AREA \
  TX_DISABLE

/// @brief 宏定义 临界区终点
#define end_critical() TX_RESTORE

/// @brief 宏定义 禁止拷贝
#define NO_COPY(NAME)                    \
  NAME(const NAME&)            = delete; \
  NAME& operator=(const NAME&) = delete;

/// @brief 宏定义 禁止移动
#define NO_MOVE(NAME)               \
  NAME(NAME&&)            = delete; \
  NAME& operator=(NAME&&) = delete;

/// @brief 宏定义 禁止拷贝和移动
#define NO_COPY_MOVE(NAME)               \
  NAME(const NAME&)            = delete; \
  NAME& operator=(const NAME&) = delete; \
  NAME(NAME&&)                 = delete; \
  NAME& operator=(NAME&&)      = delete;

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 系统
namespace system
{
/// @brief 名称空间 内核
namespace kernel
{
/**
 * @brief 中断 保护器类 (集成ThreadX)
 *
 */
class Interrupt_Guard
{
private:
  /// @brief 保存的中断状态
  uint32_t interrupt_save;

public:
  /**
   * @brief 中断保护器 构造函数
   *
   */
  Interrupt_Guard()
  {
    TX_DISABLE
  }

  /**
   * @brief 中断保护器 析构函数
   *
   */
  ~Interrupt_Guard()
  {
    TX_RESTORE
  }
};

/**
 * @brief 系统时钟
 *
 */
class System_Clock
{
public:
  /**
   * @brief  获取系统时钟
   *
   * @return uint32_t 系统时钟
   */
  static uint32_t now(void) noexcept
  {
    return tx_time_get() * 1000 / TX_TIMER_TICKS_PER_SECOND;
  }

  /**
   * @brief  获取系统时钟的间隔时间
   *
   * @param  start    开始时间
   * @return uint32_t 间隔时间
   */
  static uint32_t elapsed(uint32_t start) noexcept
  {
    const uint32_t now = tx_time_get() * 1000 / TX_TIMER_TICKS_PER_SECOND;
    return now - start;
  }
};
} /* namespace kernel */

/**
 * @brief 类对象 系统监视器
 *
 */
class System_Monitor
{
private:
  /**
   * @brief 日志缓冲区大小
   */
  static constexpr uint32_t ERROR_LOG_SIZE = 64;

public:
  /**
   * @brief 日志结构体
   */
  struct Error_Log
  {
    uint32_t    time;       /* 时间戳 */
    uint32_t    error_code; /* 错误代码 */
    const char* log;        /* 日志信息 */
  };

  /**
   * @brief 系统监视器 记录错误信息
   *
   * @param error_code  错误代码
   * @param log         错误日志信息
   */
  template <typename T>
  static void O3 log_error(T error_code, const char* log)
  {
    static Error_Log error_log_buffer[ERROR_LOG_SIZE];
    static uint32_t  index;

    error_log_buffer[index] = { tx_time_get(), static_cast<uint32_t>(error_code), log };
    index                   = (index + 1) % ERROR_LOG_SIZE;

#ifdef DEMO_DEBUG
    SEGGER_RTT_printf(0, "error code: %d, log: %s", error_code, log);

    while (1)
    {
    }
#endif
  }

  /**
   * @brief 系统监视器 记录系统崩溃信息
   *
   */
  static void log_crash(void)
  {
    log_error(TX_SUSPENDED, "system crashed");
  }

  /**
   * @brief 系统监视器 检查资源是否有效
   *
   * @tparam T          资源类型(指针)
   * @param  resource   资源指针
   * @param  name       资源名称
   */
  template <typename T>
  static void O3 check_resouce(T* resource, const char* name)
  {
    if (resource == nullptr)
    {
      log_error(TX_PTR_ERROR, name);
    }
  }

  /**
   * @brief 全局系统监视器 函数状态检查
   *
   * @note  用于检查ThreadX API调用的状态，并记录错误日志
   * @param status  ThreadX API调用的状态
   * @param message 错误日志信息
   */
  static void O3 check_status(uint32_t status, const char* message) noexcept
  {
    if (status != TX_SUCCESS)
    {
      System_Monitor::log_error(status, message);
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
  static uint32_t O3 safe_execute(Func&& func, Args&&... args) noexcept
  {
    const UINT result = func(std::forward<Args>(args)...);
    check_status(result, "ThreadX API call failed");
    return result;
  }
};
} /* namespace system */
} /* namespace QAQ */

#endif /* __SYSTEM_INCLUDE_HPP__ */
