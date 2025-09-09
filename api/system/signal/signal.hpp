#ifndef __SIGNAL_HPP__
#define __SIGNAL_HPP__

#include "signal_manager.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 系统
namespace system
{
/// @brief 名称空间 信号
namespace signal
{
/**
 * @brief  信号类
 *
 * @tparam Args 信号参数类型
 */
template <typename... Args>
class Signal : public system_internal::signal_internal::Signal_Base
{
  // 禁止拷贝和移动
  NO_COPY_MOVE(Signal)

public:
  /**
   * @brief  信号类 构造函数
   *
   */
  explicit Signal() {}

  /**
   * @brief  信号类 连接 (成员函数)
   *
   * @tparam Receiver           接收对象类型
   * @tparam Handler            连接函数类型
   * @param  receiver           接收对象指针
   * @param  handler            连接函数指针
   * @param  type               连接类型
   * @return Signal_Error_Code  连接结果
   */
  template <typename Receiver, typename Handler>
  INLINE Signal_Error_Code O3 connect(Receiver* receiver, Handler handler, Connection_Type type = Connection_Type::Auto_Connection)
  {
    /// @note 类型检查 接收对象是否符合要求
    static_assert(std::is_base_of_v<system_internal::object_internal::Object_Base, Receiver>, "Receiver must be a subclass of Object_Base");
    using function_type = void (Receiver::*)(Args...);
    /// @note 类型检查 连接函数是否符合要求
    static_assert(std::is_same_v<function_type, std::decay_t<Handler>>, "Handler type mismatch");

    return system_internal::signal_internal::Signal_Manager::instance().connect(this, receiver, handler, type);
  }

  /**
   * @brief  信号类 连接 (全局函数)
   *
   * @tparam Handler            连接函数类型
   * @param  handler            连接函数指针
   * @return Signal_Error_Code  连接结果
   */
  template <typename Handler>
  INLINE Signal_Error_Code O3 connect(Handler handler)
  {
    using function_type = void (*)(Args...);
    /// @note 类型检查 连接函数是否符合要求
    static_assert(std::is_same_v<function_type, std::decay_t<Handler>>, "Handler type mismatch");

    return system_internal::signal_internal::Signal_Manager::instance().connect(this, handler);
  }

  /**
   * @brief  信号类 断开连接 (成员函数)
   *
   * @tparam Receiver 接收对象类型
   * @tparam Handler  连接函数类型
   * @param  receiver 接收对象指针
   * @param  handler  连接函数指针
   * @return uint32_t 移除连接数量
   */
  template <typename Receiver, typename Handler>
  INLINE uint32_t O3 disconnect(Receiver* receiver, Handler handler)
  {
    /// @note 类型检查 接收对象是否符合要求
    static_assert(std::is_base_of_v<system_internal::object_internal::Object_Base, Receiver>, "Receiver must be a subclass of Object_Base");
    using function_type = void (Receiver::*)(Args...);
    /// @note 类型检查 连接函数是否符合要求
    static_assert(std::is_same_v<function_type, std::decay_t<Handler>>, "Handler type mismatch");

    return system_internal::signal_internal::Signal_Manager::instance().disconnect(this, receiver, handler);
  }

  /**
   * @brief  信号类 断开连接 (全局函数)
   *
   * @tparam Handler  连接函数类型
   * @param  handler  连接函数指针
   * @return uint32_t 移除连接数量
   */
  template <typename Handler>
  INLINE uint32_t O3 disconnect(Handler handler)
  {
    using function_type = void (*)(Args...);
    /// @note 类型检查 连接函数是否符合要求
    static_assert(std::is_same_v<function_type, std::decay_t<Handler>>, "Handler type mismatch");

    return system_internal::signal_internal::Signal_Manager::instance().disconnect(this, handler);
  }

  /**
   * @brief  信号类 发送信号
   *
   * @param  args               信号参数(左值引用)
   * @param  timeout            超时时间
   * @return Signal_Error_Code  发送结果
   */
  INLINE Signal_Error_Code O3 emit(Args&... args, uint32_t timeout = TX_WAIT_FOREVER)
  {
    system_internal::signal_internal::Signal_Semaphore* sem = nullptr;
    Signal_Error_Code                            ret = system_internal::signal_internal::Signal_Manager::instance().emit(&sem, this, std::forward<Args>(args)...);

    if (sem)
    {
      while (sem->used != sem->total)
      {
        if (sem->semaphore.acquire(timeout) != kernel::Semaphore::Status::SUCCESS)
        {
          sem->is_timeout = true;
          return Signal_Error_Code::EMIT_TIMEOUT;
        }
        else
        {
          sem->used++;
        }
      }

      system_internal::signal_internal::Signal_Manager::instance().deallocate_semaphore(sem);
    }

    return ret;
  }

  /**
   * @brief  信号类 发送信号
   *
   * @param  args               信号参数(右值引用)
   * @param  timeout            超时时间
   * @return Signal_Error_Code  发送结果
   */
  INLINE Signal_Error_Code O3 emit(Args&&... args, uint32_t timeout = TX_WAIT_FOREVER)
  {
    system_internal::signal_internal::Signal_Semaphore* sem = nullptr;
    Signal_Error_Code                            ret = system_internal::signal_internal::Signal_Manager::instance().emit(&sem, this, std::forward<Args>(args)...);

    if (sem)
    {
      while (sem->used != sem->total)
      {
        if (sem->semaphore.acquire(timeout) != kernel::Semaphore::Status::SUCCESS)
        {
          sem->is_timeout = true;
          return Signal_Error_Code::EMIT_TIMEOUT;
        }
        else
        {
          sem->used++;
        }
      }

      system_internal::signal_internal::Signal_Manager::instance().deallocate_semaphore(sem);
    }

    return ret;
  }

  /**
   * @brief  信号类 发送信号 (重载)
   *
   * @param  args               信号参数(左值引用)
   * @param  timeout            超时时间
   * @return Signal_Error_Code  发送结果
   */
  INLINE Signal_Error_Code O3 operator()(Args&... args, uint32_t timeout = TX_WAIT_FOREVER)
  {
    system_internal::signal_internal::Signal_Semaphore* sem = nullptr;
    Signal_Error_Code                            ret = system_internal::signal_internal::Signal_Manager::instance().emit(&sem, this, std::forward<Args>(args)...);

    if (sem)
    {
      while (1)
      {
        if (sem->semaphore.acquire(timeout) != kernel::Semaphore::Status::SUCCESS)
        {
          sem->is_timeout = true;
          return Signal_Error_Code::EMIT_TIMEOUT;
        }
        else
        {
          if (sem->total == sem->used)
            break;
        }
      }

      system_internal::signal_internal::Signal_Manager::instance().deallocate_semaphore(sem);
    }

    return ret;
  }

  /**
   * @brief  信号类 发送信号 (重载)
   *
   * @param  args               信号参数(右值引用)
   * @param  timeout            超时时间
   * @return Signal_Error_Code  发送结果
   */
  INLINE Signal_Error_Code O3 operator()(Args&&... args, uint32_t timeout = TX_WAIT_FOREVER)
  {
    system_internal::signal_internal::Signal_Semaphore* sem = nullptr;
    Signal_Error_Code                            ret = system_internal::signal_internal::Signal_Manager::instance().emit(&sem, this, std::forward<Args>(args)...);

    if (sem)
    {
      while (1)
      {
        if (sem->semaphore.acquire(timeout) != kernel::Semaphore::Status::SUCCESS)
        {
          sem->is_timeout = true;
          return Signal_Error_Code::EMIT_TIMEOUT;
        }
        else
        {
          if (sem->total == sem->used)
            break;
        }
      }

      system_internal::signal_internal::Signal_Manager::instance().deallocate_semaphore(sem);
    }

    return ret;
  }

  /**
   * @brief  信号类 析构函数
   *
   */
  virtual ~Signal()
  {
    system_internal::signal_internal::Signal_Manager::instance().disconnect_signal(this);
  }
};
} /* namespace signal */
} /* namespace system */
} /* namespace QAQ */

#endif /* __SIGNAL_HPP__ */
