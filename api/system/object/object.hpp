#ifndef __OBJECT_HPP__
#define __OBJECT_HPP__

#include "object_base.hpp"
#include "message_queue.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 系统
namespace system
{
/**
 * @brief  对象类
 *
 * @tparam Queue_Size                   消息队列长度
 * @tparam is_enable_signal_post_notify 是否启用信号通知
 */
template <uint32_t Queue_Size = 0, bool is_enable_signal_post_notify = false>
class Object;

/**
 * @brief  对象类 普通模式特化
 *
 */
template <>
class Object<0, false> : public system_internal::object_internal::Object_Base
{
  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(Object)

protected:
  /**
   * @brief 对象类 构造函数
   *
   */
  explicit Object(const char* name = "Object") {}

  /**
   * @brief 对象类 析构函数
   *
   */
  virtual ~Object() {}
};

/**
 * @brief  对象类 普通模式特化
 *
 */
template <>
class Object<0, true> : public system_internal::object_internal::Object_Base
{
  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(Object)

protected:
  /**
   * @brief 对象类 构造函数
   *
   */
  explicit Object(const char* name = "Object") {}

  /**
   * @brief 对象类 发送信号通知
   *
   */
  virtual void signal_post_notify(void) {};

  /**
   * @brief 对象类 析构函数
   *
   */
  virtual ~Object() {}
};

/**
 * @brief  对象类 消息队列模式特化
 *
 * @tparam Queue_Size 消息队列长度
 */
template <uint32_t Queue_Size>
class Object<Queue_Size, false> : public system_internal::object_internal::Object_Base
{
  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(Object)

private:
  /// @brief 信号数据类型
  using Signal_Data  = system_internal::signal_internal::Signal_Data_Base;
  /// @brief 消息队列类型
  using Signal_Queue = kernel::Message_Queue<Signal_Data*, Queue_Size>;
  /// @brief 信号队列
  Signal_Queue m_signal_queue;

  /**
   * @brief 对象类 清除信号
   *
   */
  void clear_signal(void) noexcept
  {
    while (m_signal_queue.enqueued() > 0)
    {
      Signal_Data* package = nullptr;
      m_signal_queue.receive(package, TX_NO_WAIT);
      if (package != nullptr)
      {
        package->destroy();
      }
    }
  }

  /**
   * @brief 对象类 发送信号
   *
   * @param package 信号数据包
   * @return true   成功发送
   * @return false  发送失败
   */
  bool post_signal(Signal_Data* package) noexcept override
  {
    return m_signal_queue.send(package, TX_NO_WAIT) == Signal_Queue::Status::SUCCESS;
  }

  /**
   * @brief  对象类 是否有信号队列
   *
   * @return true    有信号队列
   * @return false   没有信号队列
   */
  bool has_signal_queue(void) const noexcept override
  {
    return true;
  }

protected:
  /**
   * @brief 对象类 构造函数
   *
   */
  explicit Object(const char* name = "Object") : m_signal_queue((std::string(name) + " Signal Queue").c_str()) {}

  /**
   * @brief 对象类     处理信号
   *
   * @param timeout   超时时间
   * @return uint32_t 已处理信号数量
   */
  uint32_t process_signal(uint32_t timeout = TX_WAIT_FOREVER) noexcept
  {
    using Status         = typename Signal_Queue::Status;
    Signal_Data* package = nullptr;
    uint32_t     count   = 0;
    while (1)
    {
      Status status = m_signal_queue.receive(package, timeout);
      if (status == Status::SUCCESS)
      {
        if (package != nullptr)
        {
          package->execute();
          package->destroy();
          package = nullptr;
          count++;
        }
        else
        {
          break;
        }
      }
      else
      {
        break;
      }
    }
    return count;
  }

  /**
   * @brief 对象类 析构函数
   *
   */
  virtual ~Object()
  {
    clear_signal();
  }
};

/**
 * @brief  对象类 消息队列模式特化
 *
 * @tparam Queue_Size                   消息队列长度
 */
template <uint32_t Queue_Size>
class Object<Queue_Size, true> : public system_internal::object_internal::Object_Base
{
  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(Object)

private:
  /// @brief 信号数据类型
  using Signal_Data  = system_internal::signal_internal::Signal_Data_Base;
  /// @brief 消息队列类型
  using Signal_Queue = kernel::Message_Queue<Signal_Data*, Queue_Size>;
  /// @brief 信号队列
  Signal_Queue m_signal_queue;

  /**
   * @brief 对象类 清除信号
   *
   */
  void clear_signal(void) noexcept
  {
    while (m_signal_queue.enqueued() > 0)
    {
      Signal_Data* package = nullptr;
      m_signal_queue.receive(package, TX_NO_WAIT);
      if (package != nullptr)
      {
        package->destroy();
      }
    }
  }

  /**
   * @brief 对象类 发送信号
   *
   * @param package 信号数据包
   * @return true   成功发送
   * @return false  发送失败
   */
  bool post_signal(Signal_Data* package) noexcept override
  {
    signal_post_notify();
    return m_signal_queue.send(package, TX_NO_WAIT) == Signal_Queue::Status::SUCCESS;
  }

  /**
   * @brief  对象类 是否有信号队列
   *
   * @return true    有信号队列
   * @return false   没有信号队列
   */
  bool has_signal_queue(void) const noexcept override
  {
    return true;
  }

protected:
  /**
   * @brief 对象类 构造函数
   *
   */
  explicit Object(const char* name = "Object") : m_signal_queue((std::string(name) + " Signal Queue").c_str()) {}

  /**
   * @brief 对象类 发送信号通知 纯虚函数
   *
   */
  virtual void signal_post_notify(void) = 0;

  /**
   * @brief 对象类     处理信号
   *
   * @param timeout   超时时间
   * @return uint32_t 已处理信号数量
   */
  uint32_t process_signal(uint32_t timeout = TX_WAIT_FOREVER) noexcept
  {
    using Status         = typename Signal_Queue::Status;
    Signal_Data* package = nullptr;
    uint32_t     count   = 0;
    while (1)
    {
      Status status = m_signal_queue.receive(package, timeout);
      if (status == Status::SUCCESS)
      {
        if (package != nullptr)
        {
          package->execute();
          package->destroy();
          package = nullptr;
          count++;
        }
        else
        {
          break;
        }
      }
      else
      {
        break;
      }
    }
    return count;
  }

  /**
   * @brief 对象类 析构函数
   *
   */
  virtual ~Object()
  {
    clear_signal();
  }
};
} /* namespace system */
} /* namespace QAQ */

#endif /* __OBJECT_HPP__ */
