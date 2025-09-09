#ifndef __MESSAGE_QUEUE_HPP__
#define __MESSAGE_QUEUE_HPP__

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
 * @brief  消息队列模板类
 *
 * @tparam T    元素类型
 * @tparam size 元素个数
 */
template <typename T, uint32_t size>
class Message_Queue final
{
  // 消息队列大小检查 - ThreadX 要求消息队列大小至少为 4
  static_assert(size >= 4, "Message_Queue size must be greater than or equal to 4");
  // 消息队列元素类型必须是平凡类型
  static_assert(std::is_trivial_v<T>, "Message_Queue element type must be trivial");
  // 禁止拷贝和移动
  NO_COPY_MOVE(Message_Queue)

private:
  /// @brief 消息队列默认名称
  static constexpr const char* const default_name = "Message_Queue";
  /// @brief 消息队列句柄
  TX_QUEUE                           m_queue;
  /// @brief 消息队列缓冲区
  uint8_t                            m_buffer[sizeof(T) * size] ALIGN(32);

public:
  /// @brief 消息队列 状态
  enum class Status
  {
    SUCCESS = 0, /* 成功 */
    FULL,        /* 队列已满 */
    TIMEOUT,     /* 超时 */
    ERROR,       /* 错误 */
  };

  /**
   * @brief 消息队列 构造函数
   *
   * @param name    消息队列名称
   */
  explicit O3 Message_Queue(const char* name = default_name)
  {
#if (SYSTEM_ERROR_LOG_ENABLE && MESSAGE_QUEUE_ERROR_LOG_ENABLE)
    const UINT status = tx_queue_create(&m_queue, const_cast<CHAR*>(name), sizeof(T), m_buffer, sizeof(m_buffer));
    system::System_Monitor::check_status(status, "Message_Queue create failed");
#else
    tx_queue_create(&m_queue, const_cast<CHAR*>(name), sizeof(T), m_buffer, sizeof(m_buffer));
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MESSAGE_QUEUE_ERROR_LOG_ENABLE) */
  }

  /**
   * @brief  消息队列 发送
   *
   * @param  message 要发送的消息
   * @param  timeout 发送超时时间
   * @return Status  状态
   */
  Status O3 send(const T& message, uint32_t timeout = TX_WAIT_FOREVER)
  {
    if (IS_IN_ISR || IS_IN_TIMER)
    {
      timeout = 0;
    }

    const UINT status = tx_queue_send(&m_queue, const_cast<T*>(&message), timeout);
    if (status == TX_SUCCESS)
    {
      return Status::SUCCESS;
    }
    else if (status == TX_QUEUE_FULL)
    {
      return Status::FULL;
    }
    else
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MESSAGE_QUEUE_ERROR_LOG_ENABLE)
      system::System_Monitor::log_error(status, "Message_Queue send failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MESSAGE_QUEUE_ERROR_LOG_ENABLE) */
      return Status::ERROR;
    }
  }

  /**
   * @brief  消息队列 发送到队首
   *
   * @param  message 要发送的消息
   * @param  timeout 发送超时时间
   * @return Status  状态
   */
  Status O3 send_front(const T& message, uint32_t timeout = TX_WAIT_FOREVER)
  {
    if (IS_IN_ISR || IS_IN_TIMER)
    {
      timeout = 0;
    }

    const UINT status = tx_queue_front_send(&m_queue, const_cast<T*>(&message), timeout);
    if (status == TX_SUCCESS)
    {
      return Status::SUCCESS;
    }
    else if (status == TX_QUEUE_FULL)
    {
      return Status::FULL;
    }
    else
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MESSAGE_QUEUE_ERROR_LOG_ENABLE)
      system::System_Monitor::log_error(status, "Message_Queue send_front failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MESSAGE_QUEUE_ERROR_LOG_ENABLE) */
      return Status::ERROR;
    }
  }

  /**
   * @brief  消息队列 接收
   *
   * @param  message 接收到的消息
   * @param  timeout 接收超时时间
   * @return Status  状态
   */
  Status O3 receive(T& message, uint32_t timeout = TX_WAIT_FOREVER)
  {
    if (IS_IN_ISR || IS_IN_TIMER)
    {
      timeout = 0;
    }

    const UINT status = tx_queue_receive(&m_queue, &message, timeout);
    if (status == TX_SUCCESS)
    {
      return Status::SUCCESS;
    }
    else if (status == TX_QUEUE_EMPTY)
    {
      return Status::TIMEOUT;
    }
    else
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MESSAGE_QUEUE_ERROR_LOG_ENABLE)
      system::System_Monitor::log_error(status, "Message_Queue receive failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MESSAGE_QUEUE_ERROR_LOG_ENABLE) */
      return Status::ERROR;
    }
  }

  /**
   * @brief  消息队列 清空
   *
   * @return Status  状态
   */
  Status O3 clear()
  {
    const UINT status = tx_queue_flush(&m_queue);
    if (status == TX_SUCCESS)
    {
      return Status::SUCCESS;
    }
    else
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MESSAGE_QUEUE_ERROR_LOG_ENABLE)
      system::System_Monitor::log_error(status, "Message_Queue clear failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MESSAGE_QUEUE_ERROR_LOG_ENABLE) */
      return Status::ERROR;
    }
  }

  /**
   * @brief  消息队列  剩余空间
   *
   * @return uint32_t 剩余空间大小
   */
  uint32_t O3 available() const
  {
    return m_queue.tx_queue_available_storage / sizeof(T);
  }

  /**
   * @brief  消息队列  当前存有的消息数量
   *
   * @return uint32_t 消息数量
   */
  uint32_t O3 enqueued() const
  {
    return m_queue.tx_queue_enqueued / sizeof(T);
  }

  /**
   * @brief  消息队列  容量
   *
   * @return uint32_t 容量大小
   */
  uint32_t O3 capacity() const
  {
    return size;
  }

  /**
   * @brief  消息队列 是否为空
   *
   * @return true    为空
   * @return false   不为空
   */
  bool O3 empty() const
  {
    return m_queue.tx_queue_enqueued == 0;
  }

  /**
   * @brief  消息队列 是否已满
   *
   * @return true    已满
   * @return false   不满
   */
  bool O3 full() const
  {
    return m_queue.tx_queue_available_storage == 0;
  }

#ifdef TX_QUEUE_ENABLE_PERFORMANCE_INFO

  /**
   * @brief  消息队列 发送总次数
   *
   * @return ULONG   发送总次数
   */
  uint32_t O3 total_send() const
  {
    ULONG      count;
    const UINT status = tx_queue_performance_info_get(&m_queue, &count, TX_NULL, TX_NULL, TX_NULL, TX_NULL, TX_NULL);
  #if (SYSTEM_ERROR_LOG_ENABLE && MESSAGE_QUEUE_ERROR_LOG_ENABLE)
    system::System_Monitor::check_status(status, "Message_Queue get total_send failed");
  #endif /* (SYSTEM_ERROR_LOG_ENABLE && MESSAGE_QUEUE_ERROR_LOG_ENABLE) */
    return status == TX_SUCCESS ? count : 0;
  }

  /**
   * @brief  消息队列 接收总次数
   *
   * @return ULONG   接收总次数
   */
  uint32_t O3 total_recv() const
  {
    ULONG      count;
    const UINT status = tx_queue_performance_info_get(&m_queue, TX_NULL, &count, TX_NULL, TX_NULL, TX_NULL, TX_NULL);
  #if (SYSTEM_ERROR_LOG_ENABLE && MESSAGE_QUEUE_ERROR_LOG_ENABLE)
    system::System_Monitor::check_status(status, "Message_Queue get total_recv failed");
  #endif /* (SYSTEM_ERROR_LOG_ENABLE && MESSAGE_QUEUE_ERROR_LOG_ENABLE) */
    return status == TX_SUCCESS ? count : 0;
  }

#endif /* TX_QUEUE_ENABLE_PERFORMANCE_INFO */

  /**
   * @brief  消息队列 析构函数
   *
   */
  ~Message_Queue()
  {
#if (SYSTEM_ERROR_LOG_ENABLE && MESSAGE_QUEUE_ERROR_LOG_ENABLE)
    const UINT status = tx_queue_delete(&m_queue);
    system::System_Monitor::check_status(status, "Message_Queue delete failed");
#else
    tx_queue_delete(&m_queue);
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MESSAGE_QUEUE_ERROR_LOG_ENABLE) */
  }
};
} /* namespace kernel */
} /* namespace system */
} /* namespace QAQ */

#endif /* __MESSAGE_QUEUE_HPP__ */
