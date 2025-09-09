#ifndef __SEMAPHORE_HPP__
#define __SEMAPHORE_HPP__

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
 * @brief 信号量类
 *
 */
class Semaphore final
{
  // 禁止拷贝和移动
  NO_COPY_MOVE(Semaphore)

private:
  /// @brief 信号量默认名称
  static constexpr const char* default_name = "Semaphore";
  /// @brief 信号量句柄
  TX_SEMAPHORE                 m_semaphore;

public:
  /// @brief 信号量 状态
  enum class Status
  {
    SUCCESS = 0, /* 成功 */
    TIMEOUT,     /* 超时 */
    ERROR,       /* 错误 */
  };

  /**
   * @brief 信号量   构造函数
   *
   * @param initial 信号量初始值
   * @param name    信号量名称
   */
  explicit Semaphore(uint32_t initial = 0, const char* name = default_name)
  {
#if (SYSTEM_ERROR_LOG_ENABLE && SEMAPHORE_ERROR_LOG_ENABLE)
    const UINT status = tx_semaphore_create(&m_semaphore, const_cast<CHAR*>(name), initial);
    system::System_Monitor::check_status(status, "Semaphore create failed");
#else
    tx_semaphore_create(&m_semaphore, const_cast<CHAR*>(name), initial);
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SEMAPHORE_ERROR_LOG_ENABLE) */
  }

  /**
   * @brief  信号量   获取信号
   *
   * @param  timeout 等待超时时间
   * @return Status  状态
   */
  Status O3 acquire(uint32_t timeout = TX_WAIT_FOREVER) noexcept
  {
    if (true == IS_IN_ISR || true == IS_IN_TIMER)
    {
      timeout = 0;
    }

    const UINT status = tx_semaphore_get(&m_semaphore, timeout);
    if (status == TX_SUCCESS)
    {
      return Status::SUCCESS;
    }
    else if (status == TX_NO_INSTANCE)
    {
      return Status::TIMEOUT;
    }
    else
    {
#if (SYSTEM_ERROR_LOG_ENABLE && SEMAPHORE_ERROR_LOG_ENABLE)
      system::System_Monitor::log_error(status, "Semaphore get failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SEMAPHORE_ERROR_LOG_ENABLE) */
      return Status::ERROR;
    }
  }

  /**
   * @brief  信号量   带上限值 获取信号
   *
   * @param  ceiling 信号量上限值
   * @return Status  状态
   */
  Status O3 ceiling(uint32_t ceiling) noexcept
  {
    const UINT status = tx_semaphore_ceiling_put(&m_semaphore, ceiling);
    if (status == TX_SUCCESS)
    {
      return Status::SUCCESS;
    }
    else if (status == TX_NO_INSTANCE)
    {
      return Status::TIMEOUT;
    }
    else
    {
#if (SYSTEM_ERROR_LOG_ENABLE && SEMAPHORE_ERROR_LOG_ENABLE)
      system::System_Monitor::log_error(status, "Semaphore ceiling put failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SEMAPHORE_ERROR_LOG_ENABLE) */
      return Status::ERROR;
    }
  }

  /**
   * @brief  信号量   释放信号
   *
   * @return Status  状态
   */
  Status O3 release() noexcept
  {
    const UINT status = tx_semaphore_put(&m_semaphore);
    if (status == TX_SUCCESS)
    {
      return Status::SUCCESS;
    }
    else
    {
#if (SYSTEM_ERROR_LOG_ENABLE && SEMAPHORE_ERROR_LOG_ENABLE)
      system::System_Monitor::log_error(status, "Semaphore put failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SEMAPHORE_ERROR_LOG_ENABLE) */
      return Status::ERROR;
    }
  }

  /**
   * @brief  信号量    获取信号量剩余数量
   *
   * @return uint32_t 信号量剩余数量
   */
  uint32_t O3 available() noexcept
  {
    ULONG      count;
    const UINT status = tx_semaphore_info_get(&m_semaphore, static_cast<char**>(TX_NULL), &count, static_cast<TX_THREAD**>(TX_NULL), static_cast<ULONG*>(TX_NULL), static_cast<TX_SEMAPHORE**>(TX_NULL));
#if (SYSTEM_ERROR_LOG_ENABLE && SEMAPHORE_ERROR_LOG_ENABLE)
    system::System_Monitor::check_status(status, "Semaphore info get failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SEMAPHORE_ERROR_LOG_ENABLE) */
    return (status == TX_SUCCESS) ? count : 0;
  }

#ifdef TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO

  /**
   * @brief  信号量 获取信号量总获取次数
   *
   * @return uint32_t 总获取次数
   */
  uint32_t total_acquire() noexcept
  {
    ULONG      total;
    const UINT status = tx_semaphore_performance_info_get(&m_semaphore, static_cast<ULONG*>(TX_NULL), &total, static_cast<ULONG*>(TX_NULL), static_cast<ULONG*>(TX_NULL));
  #if (SYSTEM_ERROR_LOG_ENABLE && SEMAPHORE_ERROR_LOG_ENABLE)
    system::System_Monitor::check_status(status, "Semaphore performance info get failed");
  #endif /* (SYSTEM_ERROR_LOG_ENABLE && SEMAPHORE_ERROR_LOG_ENABLE) */
    return (status == TX_SUCCESS) ? total : 0;
  }

  /**
   * @brief  信号量 获取信号量总释放次数
   *
   * @return uint32_t 总释放次数
   */
  uint32_t total_release() noexcept
  {
    ULONG      total;
    const UINT status = tx_semaphore_performance_info_get(&m_semaphore, &total, static_cast<ULONG*>(TX_NULL), static_cast<ULONG*>(TX_NULL), static_cast<ULONG*>(TX_NULL));
  #if (SYSTEM_ERROR_LOG_ENABLE && SEMAPHORE_ERROR_LOG_ENABLE)
    system::System_Monitor::check_status(status, "Semaphore performance info get failed");
  #endif /* (SYSTEM_ERROR_LOG_ENABLE && SEMAPHORE_ERROR_LOG_ENABLE) */
    return (status == TX_SUCCESS) ? total : 0;
  }

  /**
   * @brief  信号量 获取信号量总超时时间
   *
   * @return uint32_t 总超时时间
   */
  uint32_t total_timeout() noexcept
  {
    ULONG      total;
    const UINT status = tx_semaphore_performance_info_get(&m_semaphore, static_cast<ULONG*>(TX_NULL), static_cast<ULONG*>(TX_NULL), static_cast<ULONG*>(TX_NULL), &total);
  #if (SYSTEM_ERROR_LOG_ENABLE && SEMAPHORE_ERROR_LOG_ENABLE)
    system::System_Monitor::check_status(status, "Semaphore performance info get failed");
  #endif /* (SYSTEM_ERROR_LOG_ENABLE && SEMAPHORE_ERROR_LOG_ENABLE) */
    return (status == TX_SUCCESS) ? total : 0;
  }

#endif /* TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO */

  /**
   * @brief  信号量 析构函数
   *
   */
  ~Semaphore()
  {
#if (SYSTEM_ERROR_LOG_ENABLE && SEMAPHORE_ERROR_LOG_ENABLE)
    const UINT status = tx_semaphore_delete(&m_semaphore);
    system::System_Monitor::check_status(status, "Semaphore delete failed");
#else
    tx_semaphore_delete(&m_semaphore);
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SEMAPHORE_ERROR_LOG_ENABLE) */
  }
};
} /* namespace kernel */
} /* namespace system */
} /* namespace QAQ */

#endif /* __SEMAPHORE_HPP__ */
