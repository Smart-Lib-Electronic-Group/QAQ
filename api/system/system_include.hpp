#ifndef __SYSTEM_INCLUDE_HPP__
#define __SYSTEM_INCLUDE_HPP__

#include <math.h>
#include <string.h>
#include <stdbool.h>

#include <atomic>
#include <tuple>
#include <typeinfo>
#include <type_traits>
#include <string>

#include "tx_api.h"
#include "stm32h7xx.h"
#include "user_config.h"

#include "system_monitor.hpp"

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
} /* namespace kernel */
} /* namespace system */
} /* namespace QAQ */

#endif /* __SYSTEM_INCLUDE_HPP__ */
