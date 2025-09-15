#ifndef __SYSTEM_CLOCK_HPP__
#define __SYSTEM_CLOCK_HPP__

#include <stdint.h>
#include "system_define.hpp"

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
 * @brief 系统时钟
 *
 */
class System_Clock
{
public:
  /**
   * @brief 时间结构体
   *
   */
  struct Time_t
  {
    uint32_t hour;        /* 小时 */
    uint32_t minute;      /* 分钟 */
    uint32_t second;      /* 秒 */
    uint32_t millisecond; /* 毫秒 */
  };

  /**
   * @brief  获取系统时钟
   *
   * @return uint32_t 系统时钟 - ticks
   */
  static uint32_t now(void) noexcept
  {
    return tx_time_get() * 1000 / TX_TIMER_TICKS_PER_SECOND;
  }

  /**
   * @brief  获取系统时钟
   *
   * @return Time_t 系统时钟 - 时间结构体
   */
  static Time_t now_time(void) noexcept
  {
    Time_t   time;
    uint32_t ticks   = tx_time_get() * 1000 / TX_TIMER_TICKS_PER_SECOND;
    time.hour        = ticks / 3600000;
    time.minute      = (ticks % 3600000) / 60000;
    time.second      = (ticks % 60000) / 1000;
    time.millisecond = ticks % 1000;
    return time;
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
} /* namespace system */
} /* namespace QAQ */

#endif /* __SYSTEM_CLOCK_HPP__ */
