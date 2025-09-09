#ifndef __EVENT_FLAGS_HPP__
#define __EVENT_FLAGS_HPP__

#include "system_include.hpp"

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
 * @brief 事件标志类
 *
 */
class Event_Flags final
{
  // 禁止拷贝和移动
  NO_COPY_MOVE(Event_Flags)

private:
  /// @brief 事件标志默认名称
  constexpr static const char* default_name = "Event_Flags";
  /// @brief 事件标志组句柄
  TX_EVENT_FLAGS_GROUP         m_event_group;

public:
  /// @brief 事件标志 选项
  enum class Options : uint32_t
  {
    Or        = 0,
    Or_Clear  = 1,
    And       = 2,
    And_Clear = 3,
  };

  /**
   * @brief 事件标志 构造函数
   *
   * @param name 事件标志名称
   */
  explicit O3 Event_Flags(const char* name = default_name)
  {
#if (SYSTEM_ERROR_LOG_ENABLE && EVENT_FLAGS_ERROR_LOG_ENABLE)
    const UINT status = tx_event_flags_create(&m_event_group, const_cast<CHAR*>(name));
    System_Monitor::check_status(status, "Failed to create event flags group");
#else
    tx_event_flags_create(&m_event_group, const_cast<CHAR*>(name));
#endif /* (SYSTEM_ERROR_LOG_ENABLE && EVENT_FLAGS_ERROR_LOG_ENABLE) */
  }

  /**
   * @brief  事件标志 等待事件标志
   *
   * @param  flags        等待的事件标志
   * @param  timeout      超时时间
   * @param  wait_option  选项
   * @return uint32_t     结果
   */
  uint32_t O3 wait(uint32_t flags, uint32_t timeout = TX_WAIT_FOREVER, Options wait_option = Options::Or)
  {
    uint32_t   result = 0;
    const UINT status = tx_event_flags_get(&m_event_group, flags, static_cast<UINT>(wait_option), &result, timeout);

    if (status == TX_NO_EVENTS)
    {
      result = 0;
    }

#if (SYSTEM_ERROR_LOG_ENABLE && EVENT_FLAGS_ERROR_LOG_ENABLE)
    else if (status != TX_SUCCESS)
    {
      System_Monitor::log_error(status, "Failed to get event flags");
    }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && EVENT_FLAGS_ERROR_LOG_ENABLE) */

    return result;
  }

  /**
   * @brief  事件标志 获取事件标志
   *
   * @param  timeout   超时时间
   * @return uint32_t  结果
   */
  uint32_t O3 get(uint32_t timeout = TX_NO_WAIT)
  {
    uint32_t   result = 0;
    const UINT status = tx_event_flags_get(&m_event_group, 0xFFFFFFFF, static_cast<UINT>(Options::Or), &result, timeout);

#if (SYSTEM_ERROR_LOG_ENABLE && EVENT_FLAGS_ERROR_LOG_ENABLE)
    System_Monitor::check_status(status, "Failed to get event flags");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && EVENT_FLAGS_ERROR_LOG_ENABLE) */

    return (status == TX_SUCCESS) ? result : 0;
  }

  /**
   * @brief  事件标志 设置事件标志
   *
   * @param  flags   待设置的事件标志
   * @return true    设置成功
   * @return false   设置失败
   */
  bool O3 set(uint32_t flags)
  {
    const UINT status = tx_event_flags_set(&m_event_group, flags, TX_OR);

#if (SYSTEM_ERROR_LOG_ENABLE && EVENT_FLAGS_ERROR_LOG_ENABLE)
    System_Monitor::check_status(status, "Failed to set event flags");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && EVENT_FLAGS_ERROR_LOG_ENABLE) */

    return (status == TX_SUCCESS);
  }

  /**
   * @brief  事件标志 清除事件标志
   *
   * @param  flags   待清除的事件标志
   * @return true    清除成功
   * @return false   清除失败
   */
  bool O3 clear(uint32_t flags)
  {
    const UINT status = tx_event_flags_set(&m_event_group, ~flags, TX_AND);

#if (SYSTEM_ERROR_LOG_ENABLE && EVENT_FLAGS_ERROR_LOG_ENABLE)
    System_Monitor::check_status(status, "Failed to clear event flags");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && EVENT_FLAGS_ERROR_LOG_ENABLE) */

    return (status == TX_SUCCESS);
  }

  /**
   * @brief  事件标志 析构函数
   *
   */
  ~Event_Flags()
  {
#if (SYSTEM_ERROR_LOG_ENABLE && EVENT_FLAGS_ERROR_LOG_ENABLE)
    const UINT status = tx_event_flags_delete(&m_event_group);
    System_Monitor::check_status(status, "Failed to delete event flags group");
#else
    tx_event_flags_delete(&m_event_group);
#endif /* (SYSTEM_ERROR_LOG_ENABLE && EVENT_FLAGS_ERROR_LOG_ENABLE) */
  }
};
} /* namespace kernel */
} /* namespace system */
} /* namespace QAQ */

#endif /* __EVENT_FLAGS_HPP__ */
