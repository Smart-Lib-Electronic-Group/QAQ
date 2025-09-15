#ifndef __OBJECT_BASE_HPP__
#define __OBJECT_BASE_HPP__

#include "signal_base.hpp"
#include "thread_base.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 系统
namespace system
{
/// @brief 名称空间 内部
namespace system_internal
{
/// @brief 名称空间 信号 内部
namespace signal_internal
{
class Signal_Manager;
struct Signal_Data_Base;
} /* namespace signal_internal */

namespace object_internal
{
class Object_Base;
} /* namespace object_internal */

namespace thread_internal
{
/**
 * @brief  获取当前线程对象指针
 *
 * @return object_internal::Object_Base* 线程对象指针
 */
object_internal::Object_Base* get_thread_object(void) noexcept
{
  return tx_thread_identify() ? static_cast<object_internal::Object_Base*>(tx_thread_identify()->tx_thread_user_data[0]) : nullptr;
}
} /* namespace thread_internal */

/// @brief 名称空间 对象内部
namespace object_internal
{
/**
 * @brief  对象基类
 *
 */
class Object_Base
{
  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(Object_Base)

private:
  /// @brief 友元声明 信号管理器
  friend class signal_internal::Signal_Manager;

  /// @brief 从属线程
  Object_Base*     m_affinity_thread = nullptr;
  /// @brief 是否合法
  std::atomic_bool m_valid;

private:
  /**
   * @brief 对象基类 发送信号
   *
   * @param package 信号数据包
   * @return true   成功发送
   * @return false  失败发送
   */
  virtual bool post_signal(signal_internal::Signal_Data_Base* package) noexcept
  {
    return false;
  }

  /**
   * @brief  对象基类 是否有信号队列
   *
   * @return true    有信号队列
   * @return false   没有信号队列
   */
  virtual bool has_signal_queue(void) const noexcept
  {
    return false;
  }

protected:
  /**
   * @brief 对象基类 构造函数
   *
   */
  explicit Object_Base()
  {
    m_valid = true;
  }

  /**
   * @brief 对象基类 析构函数
   *
   */
  virtual ~Object_Base()
  {
    m_valid           = false;
    m_affinity_thread = nullptr;
    signal_internal::__signal_manager_base->disconnect_receiver(this);
  }

public:
  /**
   * @brief 对象基类 设置从属线程
   *
   * @param affinity_thread 从属线程
   */
  void set_affinity_thread(Object_Base* affinity_thread) noexcept
  {
    m_affinity_thread = affinity_thread;
  }

  /**
   * @brief  对象基类 获取从属线程
   *
   * @return Object_Base* 从属线程
   */
  Object_Base* get_affinity_thread(void) const noexcept
  {
    return m_affinity_thread;
  }

  /**
   * @brief  对象基类 是否有从属线程
   *
   * @return true    有从属线程
   * @return false   没有从属线程
   */
  bool has_affinity_thread(void) const noexcept
  {
    return m_affinity_thread != nullptr;
  }

  /**
   * @brief  对象基类 是否是从属线程
   *
   * @param  thread  从属线程
   * @return true    是从属线程
   * @return false   不是从属线程
   */
  bool is_affinity_thread(Object_Base* thread) const noexcept
  {
    return m_affinity_thread == thread;
  }

  /**
   * @brief  对象基类 是否有效
   *
   * @return true    有效
   * @return false   无效
   */
  bool is_valid(void) const noexcept
  {
    return m_valid;
  }
};
} /* namespace object_internal */
} /* namespace system_internal */
} /* namespace system */
} /* namespace QAQ */

#endif /* __OBJECT_BASE_HPP__ */
