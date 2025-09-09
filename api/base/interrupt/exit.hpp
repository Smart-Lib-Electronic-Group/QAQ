#ifndef __EXIT_HPP__
#define __EXIT_HPP__

#include "interrupt.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 基本对象
namespace base
{
/// @brief 名称空间 EXIT 中断
namespace exit
{
/// @brief 回调函数参数类型
using Exit_Args_t = void*;
/// @brief 回调函数类型
using Exit_Func_t = void (*)(Exit_Args_t);

/**
 * @brief Exit 中断管理器
 *
 */
class Exit_Interrupt_Manager
{
private:
  /// @brief Exit 数据结构体
  struct Exit_Data
  {
    Exit_Func_t               direct_function; /* 同步回调函数 - 中断服务函数内执行 */
    Exit_Func_t               queue_function;  /* 异步回调函数 - 管理器线程内执行 */
    Exit_Args_t               arg;             /* 回调函数参数 */
    interrupt::Interrupt_Type type;            /* 中断函数触发类型 */
  };

  /// @brief Exit 数组
  Exit_Data m_exit_data[16];

private:
  /// @brief Exit 中断管理器 回调处理句柄
  template <uint8_t... lines>
  struct exit_handler;

  /**
   * @brief  Exit 中断管理器 回调处理句柄
   *
   * @tparam line Exit 线号
   */
  template <uint8_t line>
  struct exit_handler<line>
  {
    static_assert(line < 5U, "Invalid exit line number");

    /**
     * @brief Exit 中断管理器 同步回调处理句柄
     *
     * @param arg         回调函数参数
     * @param line_num    Exit 线号
     */
    static void direct_handler(void* arg, uint8_t line_num)
    {
      if (line_num == line)
      {
        Exit_Data* exit_data = static_cast<Exit_Data*>(arg);
        if (exit_data[line].direct_function)
        {
          exit_data[line].direct_function(exit_data[line].arg);
        }
      }
    }

    /**
     * @brief Exit 中断管理器 异步回调处理句柄
     *
     * @param arg         回调函数参数
     * @param line_num    Exit 线号
     */
    static void queue_handler(void* arg, uint8_t line_num)
    {
      if (line_num == line)
      {
        Exit_Data* exit_data = static_cast<Exit_Data*>(arg);
        if (exit_data[line].queue_function)
        {
          exit_data[line].queue_function(exit_data[line].arg);
        }
      }
    }

    /**
     * @brief Exit 中断管理器 获取触发的 Exit 线号
     *
     * @return uint8_t Exit 线号
     */
    static uint8_t get_line_num()
    {
      if (EXTI->PR1 & (1U << line))
      {
        EXTI->PR1 = (1U << line);
        return line;
      }
      return 16U;
    }
  };

  /**
   * @brief  Exit  中断管理器 回调处理句柄
   *
   * @tparam line  Exit 线号
   * @tparam lines 后续的 Exit 线号
   */
  template <uint8_t line, uint8_t... lines>
  struct exit_handler<line, lines...>
  {
    static_assert((line == 5U && sizeof...(lines) == 4U) || (line == 10U && sizeof...(lines) == 5U), "Invalid exit line number");

    /**
     * @brief Exit 中断管理器 同步回调处理句柄
     *
     * @param arg         回调函数参数
     * @param id          Exit id号
     */
    static void direct_handler(void* arg, uint8_t id)
    {
      for (uint8_t i = 0; i < 5U; i++)
      {
        if (id & (1U << i))
        {
          Exit_Data* exit_data = static_cast<Exit_Data*>(arg);
          if (exit_data[line + i].direct_function)
          {
            exit_data[line + i].direct_function(exit_data[line + i].arg);
          }
        }
      }
    }

    /**
     * @brief Exit 中断管理器 异步回调处理句柄
     *
     * @param arg         回调函数参数
     * @param id          Exit id号
     */
    static void queue_handler(void* arg, uint8_t id)
    {
      for (uint8_t i = 0; i < 5U; i++)
      {
        if (id & (1U << i))
        {
          Exit_Data* exit_data = static_cast<Exit_Data*>(arg);
          if (exit_data[line + i].queue_function)
          {
            exit_data[line + i].queue_function(exit_data[line + i].arg);
          }
        }
      }
    }

    /**
     * @brief Exit 中断管理器 获取触发的 Exit id号
     *
     * @return uint8_t Exit id号
     */
    static uint8_t get_line_num()
    {
      uint8_t id = 0;
      for (uint8_t i = 0; i < 5U; i++)
      {
        if (EXTI->PR1 & (1U << (line + i)))
        {
          id        += (1U << i);
          EXTI->PR1  = (1U << (line + i));
        }
      }
      return id;
    }
  };

  /**
   * @brief Exit 中断管理器 注册回调函数
   *
   * @param channel      Exit 线号
   * @param type         中断函数触发类型
   * @param priority     中断优先级
   * @param subpriority  中断子优先级
   */
  void register_irq(uint8_t channel, interrupt::Interrupt_Type type, uint32_t priority, uint32_t subpriority) noexcept
  {
    if (type == interrupt::Interrupt_Type::Direct)
    {
      switch (channel)
      {
        case 0U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI0_IRQn, exit_handler<0U>::direct_handler, &m_exit_data[0], exit_handler<0U>::get_line_num, type, priority, subpriority);
          break;
        case 1U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI1_IRQn, exit_handler<1U>::direct_handler, &m_exit_data[0], exit_handler<1U>::get_line_num, type, priority, subpriority);
          break;
        case 2U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI2_IRQn, exit_handler<2U>::direct_handler, &m_exit_data[0], exit_handler<2U>::get_line_num, type, priority, subpriority);
          break;
        case 3U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI3_IRQn, exit_handler<3U>::direct_handler, &m_exit_data[0], exit_handler<3U>::get_line_num, type, priority, subpriority);
          break;
        case 4U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI4_IRQn, exit_handler<4U>::direct_handler, &m_exit_data[0], exit_handler<4U>::get_line_num, type, priority, subpriority);
          break;
        case 5U :
        case 6U :
        case 7U :
        case 8U :
        case 9U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI9_5_IRQn, exit_handler<5U, 6U, 7U, 8U, 9U>::direct_handler, &m_exit_data[0], exit_handler<5U, 6U, 7U, 8U, 9U>::get_line_num, type, priority, subpriority);
          break;
        case 10U :
        case 11U :
        case 12U :
        case 13U :
        case 14U :
        case 15U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI15_10_IRQn, exit_handler<10U, 11U, 12U, 13U, 14U, 15U>::direct_handler, &m_exit_data[0], exit_handler<10U, 11U, 12U, 13U, 14U, 15U>::get_line_num, type, priority, subpriority);
          break;
        default :
          break;
      }
    }
    else if (type == interrupt::Interrupt_Type::Queue)
    {
      switch (channel)
      {
        case 0U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI0_IRQn, exit_handler<0U>::queue_handler, &m_exit_data[0], exit_handler<0U>::get_line_num, type, priority, subpriority);
          break;
        case 1U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI1_IRQn, exit_handler<1U>::queue_handler, &m_exit_data[0], exit_handler<1U>::get_line_num, type, priority, subpriority);
          break;
        case 2U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI2_IRQn, exit_handler<2U>::queue_handler, &m_exit_data[0], exit_handler<2U>::get_line_num, type, priority, subpriority);
          break;
        case 3U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI3_IRQn, exit_handler<3U>::queue_handler, &m_exit_data[0], exit_handler<3U>::get_line_num, type, priority, subpriority);
          break;
        case 4U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI4_IRQn, exit_handler<4U>::queue_handler, &m_exit_data[0], exit_handler<4U>::get_line_num, type, priority, subpriority);
          break;
        case 5U :
        case 6U :
        case 7U :
        case 8U :
        case 9U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI9_5_IRQn, exit_handler<5U, 6U, 7U, 8U, 9U>::queue_handler, &m_exit_data[0], exit_handler<5U, 6U, 7U, 8U, 9U>::get_line_num, type, priority, subpriority);
          break;
        case 10U :
        case 11U :
        case 12U :
        case 13U :
        case 14U :
        case 15U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI15_10_IRQn, exit_handler<10U, 11U, 12U, 13U, 14U, 15U>::queue_handler, &m_exit_data[0], exit_handler<10U, 11U, 12U, 13U, 14U, 15U>::get_line_num, type, priority, subpriority);
          break;
        default :
          break;
      }
    }
    else if (type == interrupt::Interrupt_Type::Mixed)
    {
      switch (channel)
      {
        case 0U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI0_IRQn, exit_handler<0U>::direct_handler, exit_handler<0U>::queue_handler, &m_exit_data[0], exit_handler<0U>::get_line_num, priority, subpriority);
          break;
        case 1U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI1_IRQn, exit_handler<1U>::direct_handler, exit_handler<1U>::queue_handler, &m_exit_data[0], exit_handler<1U>::get_line_num, priority, subpriority);
          break;
        case 2U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI2_IRQn, exit_handler<2U>::direct_handler, exit_handler<2U>::queue_handler, &m_exit_data[0], exit_handler<2U>::get_line_num, priority, subpriority);
          break;
        case 3U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI3_IRQn, exit_handler<3U>::direct_handler, exit_handler<3U>::queue_handler, &m_exit_data[0], exit_handler<3U>::get_line_num, priority, subpriority);
          break;
        case 4U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI4_IRQn, exit_handler<4U>::direct_handler, exit_handler<4U>::queue_handler, &m_exit_data[0], exit_handler<4U>::get_line_num, priority, subpriority);
          break;
        case 5U :
        case 6U :
        case 7U :
        case 8U :
        case 9U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI9_5_IRQn, exit_handler<5U, 6U, 7U, 8U, 9U>::direct_handler, exit_handler<5U, 6U, 7U, 8U, 9U>::queue_handler, &m_exit_data[0], exit_handler<5U, 6U, 7U, 8U, 9U>::get_line_num, priority, subpriority);
          break;
        case 10U :
        case 11U :
        case 12U :
        case 13U :
        case 14U :
        case 15U :
          interrupt::Interrupt_Manager::get_instance().register_interrupt(EXTI15_10_IRQn, exit_handler<10U, 11U, 12U, 13U, 14U, 15U>::direct_handler, exit_handler<10U, 11U, 12U, 13U, 14U, 15U>::queue_handler, &m_exit_data[0], exit_handler<10U, 11U, 12U, 13U, 14U, 15U>::get_line_num, priority, subpriority);
          break;
        default :
          break;
      }
    }
  }

  /**
   * @brief Exit 中断管理器 注销回调函数
   *
   * @param channel Exit 线号
   */
  void unregister_irq(uint8_t channel) noexcept
  {
    switch (channel)
    {
      case 0U :
        interrupt::Interrupt_Manager::get_instance().unregister_interrupt(EXTI0_IRQn);
        break;
      case 1U :
        interrupt::Interrupt_Manager::get_instance().unregister_interrupt(EXTI1_IRQn);
        break;
      case 2U :
        interrupt::Interrupt_Manager::get_instance().unregister_interrupt(EXTI2_IRQn);
        break;
      case 3U :
        interrupt::Interrupt_Manager::get_instance().unregister_interrupt(EXTI3_IRQn);
        break;
      case 4U :
        interrupt::Interrupt_Manager::get_instance().unregister_interrupt(EXTI4_IRQn);
        break;
      case 5U :
      case 6U :
      case 7U :
      case 8U :
      case 9U :
        {
          bool all_unregistered = true;
          for (uint8_t i = 5U; i < 10U; i++)
          {
            if (m_exit_data[i].queue_function || m_exit_data[i].direct_function)
            {
              all_unregistered = false;
              break;
            }
          }
          if (all_unregistered)
          {
            interrupt::Interrupt_Manager::get_instance().unregister_interrupt(EXTI9_5_IRQn);
          }
          break;
        }
      case 10U :
      case 11U :
      case 12U :
      case 13U :
      case 14U :
      case 15U :
        {
          bool all_unregistered = true;
          for (uint8_t i = 10U; i < 16U; i++)
          {
            if (m_exit_data[i].queue_function || m_exit_data[i].direct_function)
            {
              all_unregistered = false;
              break;
            }
          }
          if (all_unregistered)
          {
            interrupt::Interrupt_Manager::get_instance().unregister_interrupt(EXTI15_10_IRQn);
          }
          break;
        }
      default :
        break;
    }
  }

  /**
   * @brief Exit 中断管理器 构造函数
   *
   */
  explicit Exit_Interrupt_Manager() noexcept
  {
    memset(m_exit_data, 0, sizeof(m_exit_data));
  }

  /**
   * @brief Exit 中断管理器 析构函数
   *
   */
  ~Exit_Interrupt_Manager() noexcept
  {
    for (uint8_t i = 0; i < 16U; i++)
    {
      if (m_exit_data[i].queue_function || m_exit_data[i].direct_function)
      {
        unregister_interrupt(i);
      }
    }
  }

public:
  /**
   * @brief  Exit 中断管理器 获取单例对象
   *
   * @return Exit_Interrupt_Manager& 单例对象
   */
  static Exit_Interrupt_Manager& get_instance() noexcept
  {
    static Exit_Interrupt_Manager instance;
    return instance;
  }

  /**
   * @brief Exit 中断管理器 注册回调函数 (同步或异步)
   *
   * @param channel     Exit 线号
   * @param type        中断函数触发类型
   * @param function    中断回调函数
   * @param arg         中断回调函数参数
   * @param priority    中断优先级
   * @param subpriority 中断子优先级
   */
  void register_interrupt(uint8_t channel, interrupt::Interrupt_Type type, Exit_Func_t function, Exit_Args_t arg, uint32_t priority, uint32_t subpriority) noexcept
  {
    if (interrupt::Interrupt_Type::Direct == type)
    {
      m_exit_data[channel].direct_function = function;
      m_exit_data[channel].arg             = arg;
      m_exit_data[channel].type            = type;
    }
    else if (interrupt::Interrupt_Type::Queue == type)
    {
      m_exit_data[channel].queue_function = function;
      m_exit_data[channel].arg            = arg;
      m_exit_data[channel].type           = type;
    }
    else
    {
      return;
    }

    register_irq(channel, type, priority, subpriority);
  }

  /**
   * @brief Exit 中断管理器 注册回调函数 (混合)
   *
   * @param channel         Exit 线号
   * @param queue_function  异步中断回调函数
   * @param direct_function 同步中断回调函数
   * @param arg             中断回调函数参数
   * @param priority        中断优先级
   * @param subpriority     中断子优先级
   */
  void register_interrupt(uint8_t channel, Exit_Func_t queue_function, Exit_Func_t direct_function, Exit_Args_t arg, uint32_t priority, uint32_t subpriority) noexcept
  {
    m_exit_data[channel].queue_function  = queue_function;
    m_exit_data[channel].direct_function = direct_function;
    m_exit_data[channel].arg             = arg;
    m_exit_data[channel].type            = interrupt::Interrupt_Type::Mixed;

    register_irq(channel, interrupt::Interrupt_Type::Mixed, priority, subpriority);
  }

  /**
   * @brief Exit 中断管理器 注销回调函数
   *
   * @param channel Exit 线号
   */
  void unregister_interrupt(uint8_t channel) noexcept
  {
    m_exit_data[channel].queue_function  = nullptr;
    m_exit_data[channel].direct_function = nullptr;
    m_exit_data[channel].arg             = nullptr;
    m_exit_data[channel].type            = interrupt::Interrupt_Type::Direct;

    unregister_irq(channel);
  }
};
} /* namespace exit */
} /* namespace base */
} /* namespace QAQ */

/* ----- 注册中断函数 ----- */
INTERRUPT_HANDLER(EXTI0)
INTERRUPT_HANDLER(EXTI1)
INTERRUPT_HANDLER(EXTI2)
INTERRUPT_HANDLER(EXTI3)
INTERRUPT_HANDLER(EXTI4)
INTERRUPT_HANDLER(EXTI9_5)
INTERRUPT_HANDLER(EXTI15_10)

#endif /* __EXIT_HPP__ */
