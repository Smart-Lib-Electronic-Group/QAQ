#ifndef __MUTEX_HPP__
#define __MUTEX_HPP__

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
 * @brief  互斥锁模板类
 *
 * @tparam priority_inherit 是否支持继承优先级 (默认支持)
 */
template <bool priority_inherit = true>
class Mutex final
{
  // 禁止拷贝与移动
  NO_COPY_MOVE(Mutex)

private:
  /// @brief 互斥锁默认名称
  static constexpr const char* const default_name = "Mutex";
  /// @brief 互斥锁句柄
  TX_MUTEX                           m_mutex;

public:
  /// @brief 互斥锁 状态
  enum class Status
  {
    SUCCESS = 0, /* 正常 */
    TIMEOUT,     /* 超时 */
    OWNERSHIP,   /* 互斥锁已经被其他线程持有 */
    IN_ISR,      /* 在中断中 */
    ERROR,       /* 其他错误 */
  };

  /**
   * @brief 互斥锁 构造函数
   *
   * @param name 互斥锁名称
   */
  explicit Mutex(const char* name = default_name)
  {
    const UINT inherit = priority_inherit ? TX_INHERIT : TX_NO_INHERIT;
#if (SYSTEM_ERROR_LOG_ENABLE && MUTEX_ERROR_LOG_ENABLE)
    const UINT status = tx_mutex_create(&m_mutex, const_cast<char*>(name), inherit);
    system::System_Monitor::check_status(status, "Mutex create failed");
#else
    tx_mutex_create(&m_mutex, const_cast<char*>(name), inherit);
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MUTEX_ERROR_LOG_ENABLE) */
  }

  /**
   * @brief  互斥锁   上锁
   *
   * @param  timeout 超时时间(ms)
   * @return Status  状态
   */
  Status O3 lock(uint32_t timeout = TX_WAIT_FOREVER) noexcept
  {
    if (IS_IN_ISR)
    {
      return Status::IN_ISR;
    }
    else if (IS_IN_TIMER)
    {
      timeout = 0;
    }

    UINT status = tx_mutex_get(&m_mutex, timeout);
    if (status == TX_SUCCESS)
    {
      return Status::SUCCESS;
    }
    else if (status == TX_NOT_AVAILABLE)
    {
      return Status::TIMEOUT;
    }
    else
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MUTEX_ERROR_LOG_ENABLE)
      system::System_Monitor::log_error(status, "Mutex get failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MUTEX_ERROR_LOG_ENABLE) */
      return Status::ERROR;
    }
  }

  /**
   * @brief  互斥锁  解锁
   *
   * @return Status 状态
   */
  Status O3 unlock() noexcept
  {
    if (IS_IN_ISR)
    {
      return Status::IN_ISR;
    }

    const UINT status = tx_mutex_put(&m_mutex);
    if (status == TX_SUCCESS)
    {
      return Status::SUCCESS;
    }
    else if (status == TX_NOT_OWNED)
    {
      return Status::OWNERSHIP;
    }
    else
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MUTEX_ERROR_LOG_ENABLE)
      system::System_Monitor::log_error(status, "Mutex put failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MUTEX_ERROR_LOG_ENABLE) */
      return Status::ERROR;
    }
  }

#ifdef TX_MUTEX_ENABLE_PERFORMANCE_INFO

  /**
   * @brief  互斥锁 性能信息 总锁次数
   *
   * @return uint32_t 总锁次数
   */
  uint32_t total_lock() const noexcept
  {
    ULONG      count;
    const UINT status = tx_mutex_performance_info_get(&m_mutex, TX_NULL, &count, TX_NULL, TX_NULL, TX_NULL, TX_NULL);
  #if (SYSTEM_ERROR_LOG_ENABLE && MUTEX_ERROR_LOG_ENABLE)
    system::System_Monitor::check_status(status, "Mutex performance info get failed");
  #endif /* (SYSTEM_ERROR_LOG_ENABLE && MUTEX_ERROR_LOG_ENABLE) */
    return status == TX_SUCCESS ? count : 0;
  }

  /**
   * @brief  互斥锁 性能信息 总解锁次数
   *
   * @return uint32_t 总解锁次数
   */
  uint32_t total_unlock() const noexcept
  {
    ULONG      count;
    const UINT status = tx_mutex_performance_info_get(&m_mutex, &count, TX_NULL, TX_NULL, TX_NULL, TX_NULL, TX_NULL);
  #if (SYSTEM_ERROR_LOG_ENABLE && MUTEX_ERROR_LOG_ENABLE)
    system::System_Monitor::check_status(status, "Mutex performance info get failed");
  #endif /* (SYSTEM_ERROR_LOG_ENABLE && MUTEX_ERROR_LOG_ENABLE) */
    return status == TX_SUCCESS ? count : 0;
  }

  /**
   * @brief  互斥锁 性能信息 总超时时间
   *
   * @return uint32_t 总超时时间
   */
  uint32_t total_timeout() const noexcept
  {
    ULONG      count;
    const UINT status = tx_mutex_performance_info_get(&m_mutex, TX_NULL, TX_NULL, TX_NULL, &count, TX_NULL, TX_NULL);
  #if (SYSTEM_ERROR_LOG_ENABLE && MUTEX_ERROR_LOG_ENABLE)
    system::System_Monitor::check_status(status, "Mutex performance info get failed");
  #endif /* (SYSTEM_ERROR_LOG_ENABLE && MUTEX_ERROR_LOG_ENABLE) */
    return status == TX_SUCCESS ? count : 0;
  }

#endif /* TX_MUTEX_ENABLE_PERFORMANCE_INFO */

  /**
   * @brief  互斥锁 析构函数
   *
   */
  ~Mutex()
  {
#if (SYSTEM_ERROR_LOG_ENABLE && MUTEX_ERROR_LOG_ENABLE)
    const UINT status = tx_mutex_delete(&m_mutex);
    system::System_Monitor::check_status(status, "Mutex delete failed");
#else
    tx_mutex_delete(&m_mutex);
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MUTEX_ERROR_LOG_ENABLE) */
  }
};

/**
 * @brief  互斥锁 保护器模板类
 *
 * @tparam T 互斥锁类型
 */
template <typename T>
class Mutex_Guard
{
  // 禁止拷贝与移动
  NO_COPY_MOVE(Mutex_Guard)

private:
  /// @brief 互斥锁引用
  T& m_mutex;

public:
  /**
   * @brief 互斥锁 保护器 构造函数
   *
   * @param mutex 互斥锁引用
   */
  explicit Mutex_Guard(T& mutex, uint32_t timeout = TX_WAIT_FOREVER) : m_mutex(mutex)
  {
    m_mutex.lock(timeout);
  }

  /**
   * @brief 互斥锁 保护器 析构函数
   *
   */
  ~Mutex_Guard()
  {
    m_mutex.unlock();
  }
};
} /* namespace kernel */
} /* namespace system */
} /* namespace QAQ */

#endif /* __MUTEX_HPP__ */
