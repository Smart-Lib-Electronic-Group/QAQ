#ifndef __THREAD_HPP__
#define __THREAD_HPP__

#include <functional>
#include "object.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 系统
namespace system
{
/// @brief 名称空间 线程
namespace thread
{
/**
 * @brief 全局函数 延时函数(ms)
 *
 * @param ms 延时时间(ms)
 */
void sleep(uint32_t ms) noexcept
{
  tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 1000 * ms);
}

/**
 * @brief 全局函数 线程强制调度
 *
 */
void yield(void) noexcept
{
  tx_thread_relinquish();
}

/// @brief 线程 状态
enum class Thread_Status : uint8_t
{
  NOT_INIT,  /* 未初始化 */
  SUSPENDED, /* 已挂起 */
  RUNNING,   /* 运行中 */
  EXITED,    /* 已退出 */
};

/// @brief 线程 错误码
enum class Thread_Error_Code : uint8_t
{
  SUCCESS,      /* 成功 */
  NOT_INIT,     /* 未初始化 */
  EXITED,       /* 已退出 */
  INIT_FAILED,  /* 初始化失败 */
  ALREADY_INIT, /* 已初始化 */
  ERROR,        /* 错误 */
};
} /* namespace thread */

/// @brief 命名空间 内部
namespace system_internal
{
/// @brief 命名空间 线程内部
namespace thread_internal
{
/**
 * @brief 判断栈对齐与大小合法性判断
 *
 * @param  size   栈大小
 * @return true   合法
 * @return false  不合法
 */
constexpr bool is_stack_valid(uint32_t size) noexcept
{
  return (size >= TX_MINIMUM_STACK) && (size % 8 == 0);
}

/**
 * @brief 栈溢出错误处理函数
 *
 * @param ptr 线程指针
 */
void thread_stack_error_handler(TX_THREAD* ptr) noexcept
{
#if (SYSTEM_ERROR_LOG_ENABLE && THREAD_ERROR_LOG_ENABLE)
  QAQ_ERROR_LOG(TX_STACK_FILL, "Stack overflow detected");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && THREAD_ERROR_LOG_ENABLE) */
  tx_thread_terminate(ptr);
}

/**
 * @brief 线程入口函数模板
 *
 * @tparam T    线程类型
 * @param  arg  线程参数
 */
template <typename T>
void thread_entry(ULONG arg)
{
  auto* thread = reinterpret_cast<T*>(arg);
  thread->run();
  thread->m_status = thread::Thread_Status::EXITED;
}

/**
 * @brief  线程基类 - CRTP基类模板
 *
 * @tparam Queue_Size                   信号队列大小
 * @tparam is_enable_signal_post_notify 是否启用信号通知
 * @tparam Derived                      派生类类型
 */
template <uint32_t Queue_Size, bool is_enable_signal_post_notify, typename Derived>
class Thread_Crtp_Base : public Object<Queue_Size, is_enable_signal_post_notify>, public Thread_Base
{
  // 禁止拷贝与移动
  QAQ_NO_COPY_MOVE(Thread_Crtp_Base)

  /// @brief 线程入口函数
  template <typename T>
  friend void thread_entry(ULONG arg);

private:
  using Thread_Status     = thread::Thread_Status;
  using Thread_Error_Code = thread::Thread_Error_Code;

  /// @brief 线程句柄
  TX_THREAD     m_thread;
  /// @brief 栈指针
  UCHAR*        m_stack_ptr;
  /// @brief 栈大小
  ULONG         m_stack_size;
  /// @brief 线程状态
  Thread_Status m_status = Thread_Status::NOT_INIT;

protected:
  /**
   * @brief 线程基类 构造函数
   *
   * @param stack 栈指针
   * @param size  栈大小
   */
  explicit Thread_Crtp_Base(UCHAR* stack, ULONG size) noexcept : m_stack_ptr(stack), m_stack_size(size)
  {
    this->set_affinity_thread(this);
  }

  /**
   * @brief  线程基类 析构函数
   *
   */
  virtual ~Thread_Crtp_Base() noexcept
  {
    stop();
#if (SYSTEM_ERROR_LOG_ENABLE && THREAD_ERROR_LOG_ENABLE)
    const UINT status = tx_thread_delete(&m_thread);
    system::System_Monitor::check_status(status, "Thread delete failed");
#else
    tx_thread_delete(&m_thread);
#endif /* (SYSTEM_ERROR_LOG_ENABLE && THREAD_ERROR_LOG_ENABLE) */
  }

public:
  /**
   * @brief  线程基类 线程创建
   *
   * @param  name               线程名(使用宏定义或静态数组定义)
   * @param  priority           线程优先级
   * @param  preempt_threshold  线程抢占阈值(默认关闭)
   * @param  time_slice         时间片轮转周期数(默认关闭)
   * @return Thread_Error_Code  线程错误码
   */
  Thread_Error_Code create(const char* name, ULONG priority, ULONG preempt_threshold = 0, ULONG time_slice = TX_NO_TIME_SLICE) noexcept
  {
    Thread_Error_Code error_code = Thread_Error_Code::SUCCESS;

    if (Thread_Status::NOT_INIT == m_status)
    {
      const UINT status = tx_thread_create(&m_thread, const_cast<CHAR*>(name), system_internal::thread_internal::thread_entry<Derived>, reinterpret_cast<ULONG>(reinterpret_cast<Derived*>(this)), m_stack_ptr, m_stack_size, priority, preempt_threshold == 0 ? priority : preempt_threshold, time_slice, TX_DONT_START);

      if (TX_SUCCESS != status)
      {
#if (SYSTEM_ERROR_LOG_ENABLE && THREAD_ERROR_LOG_ENABLE)
        QAQ_ERROR_LOG(status, "Thread creation failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && THREAD_ERROR_LOG_ENABLE) */
        error_code = Thread_Error_Code::INIT_FAILED;
      }
      else
      {
        m_status                        = Thread_Status::SUSPENDED;
        m_thread.tx_thread_user_data[0] = this;
        tx_thread_stack_error_notify(thread_stack_error_handler);
      }
    }
    else
    {
      error_code = Thread_Error_Code::ALREADY_INIT;
    }

    return error_code;
  }

  /**
   * @brief  线程基类 线程启动
   *
   * @return Thread_Error_Code 线程错误码
   */
  Thread_Error_Code start(void) noexcept
  {
    Thread_Error_Code error_code = Thread_Error_Code::SUCCESS;

    if (Thread_Status::NOT_INIT == m_status)
    {
      error_code = Thread_Error_Code::NOT_INIT;
    }
    else if (Thread_Status::EXITED == m_status)
    {
      error_code = reset();

      if (Thread_Error_Code::SUCCESS == error_code)
      {
        error_code = resume();
      }
    }
    else
    {
      error_code = resume();
    }

    return error_code;
  }

  /**
   * @brief  线程基类 线程启动
   *
   * @param  priority          线程优先级
   * @return Thread_Error_Code 线程错误码
   */
  Thread_Error_Code start(ULONG priority) noexcept
  {
    Thread_Error_Code error_code = Thread_Error_Code::SUCCESS;

    if (Thread_Status::NOT_INIT == m_status)
    {
      error_code = Thread_Error_Code::NOT_INIT;
    }
    else
    {
      set_priority(priority);
      error_code = start();
    }

    return error_code;
  }

  /**
   * @brief  线程基类 线程挂起
   *
   * @return Thread_Error_Code 线程错误码
   */
  Thread_Error_Code suspend(void) noexcept
  {
    Thread_Error_Code error_code = Thread_Error_Code::SUCCESS;

    if (Thread_Status::NOT_INIT == m_status)
    {
      error_code = Thread_Error_Code::NOT_INIT;
    }
    else if (Thread_Status::EXITED == m_status)
    {
      error_code = Thread_Error_Code::EXITED;
    }
    else if (Thread_Status::RUNNING == m_status)
    {
      const UINT status = tx_thread_suspend(&m_thread);
      if (TX_SUCCESS != status)
      {
#if (SYSTEM_ERROR_LOG_ENABLE && THREAD_ERROR_LOG_ENABLE)
        QAQ_ERROR_LOG(status, "Thread suspend failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && THREAD_ERROR_LOG_ENABLE) */
        error_code = Thread_Error_Code::ERROR;
      }
      else
      {
        m_status = Thread_Status::SUSPENDED;
      }
    }

    return error_code;
  }

  /**
   * @brief  线程基类 线程恢复
   *
   * @return Thread_Error_Code 线程错误码
   */
  Thread_Error_Code resume(void) noexcept
  {
    Thread_Error_Code error_code = Thread_Error_Code::SUCCESS;

    if (Thread_Status::NOT_INIT == m_status)
    {
      error_code = Thread_Error_Code::NOT_INIT;
    }
    else if (Thread_Status::EXITED == m_status)
    {
      error_code = Thread_Error_Code::EXITED;
    }
    else if (Thread_Status::SUSPENDED == m_status)
    {
      const UINT status = tx_thread_resume(&m_thread);
      if (TX_SUCCESS != status)
      {
#if (SYSTEM_ERROR_LOG_ENABLE && THREAD_ERROR_LOG_ENABLE)
        QAQ_ERROR_LOG(status, "Thread resume failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && THREAD_ERROR_LOG_ENABLE) */
        error_code = Thread_Error_Code::ERROR;
      }
      else
      {
        m_status = Thread_Status::RUNNING;
      }
    }

    return error_code;
  }

  /**
   * @brief  线程基类 线程停止
   *
   * @return Thread_Error_Code 线程错误码
   */
  Thread_Error_Code stop(void) noexcept
  {
    Thread_Error_Code error_code = Thread_Error_Code::SUCCESS;

    if (Thread_Status::NOT_INIT == m_status)
    {
      error_code = Thread_Error_Code::NOT_INIT;
    }
    else if (Thread_Status::EXITED != m_status)
    {
      const UINT status = tx_thread_terminate(&m_thread);
      if (TX_SUCCESS != status)
      {
#if (SYSTEM_ERROR_LOG_ENABLE && THREAD_ERROR_LOG_ENABLE)
        QAQ_ERROR_LOG(status, "Thread stop failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && THREAD_ERROR_LOG_ENABLE) */
        error_code = Thread_Error_Code::ERROR;
      }
      else
      {
        m_status = Thread_Status::EXITED;
      }
    }

    return error_code;
  }

  /**
   * @brief  线程基类 线程重置
   *
   * @return Thread_Error_Code 线程错误码
   */
  Thread_Error_Code reset(void) noexcept
  {
    Thread_Error_Code error_code = Thread_Error_Code::SUCCESS;

    if (Thread_Status::NOT_INIT == m_status)
    {
      error_code = Thread_Error_Code::NOT_INIT;
    }
    else
    {
      if (Thread_Status::EXITED != m_status)
      {
        error_code = stop();
      }

      if (Thread_Error_Code::SUCCESS == error_code)
      {
        const UINT status = tx_thread_reset(&m_thread);
        if (TX_SUCCESS != status)
        {
#if (SYSTEM_ERROR_LOG_ENABLE && THREAD_ERROR_LOG_ENABLE)
          QAQ_ERROR_LOG(status, "Thread reset failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && THREAD_ERROR_LOG_ENABLE) */
          error_code = Thread_Error_Code::ERROR;
        }
        else
        {
          m_status = Thread_Status::SUSPENDED;
        }
      }
    }

    return error_code;
  }

  /**
   * @brief  线程基类 线程休眠
   *
   * @param  ms                 休眠时间(ms)
   */
  Derived& sleep(uint32_t ms) noexcept
  {
    system::thread::sleep(ms);
    return reinterpret_cast<Derived&>(*this);
  }

  /**
   * @brief  线程基类 线程优先级设置
   *
   * @param  priority           线程优先级
   * @return Derived&           线程对象
   */
  Derived& set_priority(ULONG priority) noexcept
  {
#if (SYSTEM_ERROR_LOG_ENABLE && THREAD_ERROR_LOG_ENABLE)
    const UINT status = tx_thread_priority_change(&m_thread, priority, NULL);
    system::System_Monitor::check_status(status, "Thread priority change failed");
#else
    tx_thread_priority_change(&m_thread, priority, NULL);
#endif /* (SYSTEM_ERROR_LOG_ENABLE && THREAD_ERROR_LOG_ENABLE) */
    return reinterpret_cast<Derived&>(*this);
  }

  /**
   * @brief  线程基类 线程优先级获取
   *
   * @return uint32_t 线程优先级
   */
  uint32_t get_priority(void) const noexcept
  {
    return m_thread.tx_thread_priority;
  }

  /**
   * @brief  线程基类 线程栈大小获取
   *
   * @return uint32_t             栈大小
   */
  uint32_t get_stack_size(void) const noexcept
  {
    return m_thread.tx_thread_stack_size;
  }

  /**
   * @brief  线程基类 线程栈已使用大小获取
   *
   * @return uint32_t             栈已使用大小
   */
  uint32_t get_stack_used(void) const noexcept
  {
    return static_cast<uintptr_t*>(m_thread.tx_thread_stack_end) - static_cast<uintptr_t*>(m_thread.tx_thread_stack_ptr);
  }

  /**
   * @brief  线程基类 线程栈剩余大小获取
   *
   * @return uint32_t             栈剩余大小
   */
  uint32_t get_stack_available(void) const noexcept
  {
    return static_cast<uintptr_t*>(m_thread.tx_thread_stack_start) - static_cast<uintptr_t*>(m_thread.tx_thread_stack_ptr);
  }

  /**
   * @brief  线程基类 线程栈峰值用量获取
   *
   * @return uint32_t             栈峰值用量
   */
  uint32_t get_stack_max_used(void) const noexcept
  {
    return static_cast<uintptr_t*>(m_thread.tx_thread_stack_end) - static_cast<uintptr_t*>(m_thread.tx_thread_stack_highest_ptr);
  }

  /**
   * @brief  线程基类 线程栈用量百分比获取
   *
   * @return float              栈用量百分比
   */
  float get_stack_used_percentage(void) const noexcept
  {
    return static_cast<float>(get_stack_used()) / static_cast<float>(get_stack_size()) * 100.0f;
  }

  /**
   * @brief  线程基类 线程栈剩余百分比获取
   *
   * @return float              栈剩余百分比
   */
  float get_stack_available_percentage(void) const noexcept
  {
    return static_cast<float>(get_stack_available()) / static_cast<float>(get_stack_size()) * 100.0f;
  }

  /**
   * @brief  线程基类 线程强制调度
   *
   * @return Derived&           线程对象
   */
  Derived& yield(void) noexcept
  {
    system::thread::yield();
    return reinterpret_cast<Derived&>(*this);
  }
};
} /* namespace thread_internal */
} /* namespace system_internal */

/// @brief 名称空间 线程
namespace thread
{
/**
 * @brief  线程类模板
 *
 * @tparam Stack_Size                   栈大小
 * @tparam Queue_Size                   消息队列大小 (为0时则无队列)
 * @tparam Derived                      派生类类型
 * @tparam is_enable_signal_post_notify 是否启用信号通知
 */
template <uint32_t Stack_Size, uint32_t Queue_Size, typename Derived = void, bool is_enable_signal_post_notify = false>
class Thread;

/**
 * @brief  线程类模板 函数特化
 *
 * @tparam Stack_Size                   栈大小
 * @tparam Queue_Size                   消息队列大小 (为0时则无队列)
 * @note   线程类模板的函数特化版,用于创建线程对象,其本质为定时调用函数表达式,默认循环时间为1000ms
 */
template <uint32_t Stack_Size, uint32_t Queue_Size>
class Thread<Stack_Size, Queue_Size, void, false> : public system_internal::thread_internal::Thread_Crtp_Base<Queue_Size, false, Thread<Stack_Size, Queue_Size, void, false>>
{
  // 栈大小检查
  static_assert(system_internal::thread_internal::is_stack_valid(Stack_Size), "Stack size must be 8-byte aligned and >= TX_MINIMUM_STACK");
  // 禁止拷贝与移动
  QAQ_NO_COPY_MOVE(Thread)

  /// @brief 友元函数 线程入口函数模板
  template <typename T>
  friend void system_internal::thread_internal::thread_entry(ULONG arg);

private:
  /// @brief Lambda表达式
  std::function<void()> m_lambda;
  /// @brief 栈空间 - 8字节对齐
  UCHAR                 m_stack[Stack_Size] QAQ_ALIGN(8);
  /// @brief 循环时间间隔(ms)
  uint32_t              m_loop_time;

private:
  /**
   * @brief 线程类 线程函数
   *
   */
  void run(void)
  {
    while (true)
    {
      if constexpr (Queue_Size == 0)
      {
        m_lambda();
      }
      else
      {
        Object<Queue_Size, false>::process_signal(1);
        m_lambda();
      }
      sleep(m_loop_time);
    }
  }

public:
  template <typename Function, typename... Args>
  explicit Thread(Function&& callable, Args&&... args) : system_internal::thread_internal::Thread_Crtp_Base<Queue_Size, false, Thread<Stack_Size, Queue_Size, void, false>>(m_stack, sizeof(m_stack)), m_loop_time(1000)
  {
    m_lambda = [&]()
    {
      callable(std::forward<Args>(args)...);
    };
  }

  template <typename Function, typename... Args>
  Thread& set_function(Function&& callable, Args&&... args) noexcept
  {
    this->stop();
    m_lambda = [&]()
    {
      callable(std::forward<Args>(args)...);
    };
    this->reset();
    return *this;
  }

  /**
   * @brief  线程类 循环时间间隔设置
   *
   * @param  loop_time  循环时间间隔(ms)
   * @return Thread&    线程对象
   * @note   设置后,线程将重新启动
   */
  Thread& set_loop_time(uint32_t loop_time) noexcept
  {
    this->stop();
    m_loop_time = loop_time;
    this->reset();
    return *this;
  }

  /**
   * @brief  线程类 循环时间间隔获取
   *
   * @return uint32_t 循环时间间隔(ms)
   */
  uint32_t get_loop_time(void) const noexcept
  {
    return m_loop_time;
  }

  /**
   * @brief  线程类 析构函数
   *
   */
  virtual ~Thread() {}
};

/**
 * @brief  线程类模板 CRTP 特化版

 * @tparam Stack_Size                   栈大小
 * @tparam Queue_Size                   消息队列大小 (为0时则无队列)
 * @tparam Derived                      派生类类型
 * @tparam is_enable_signal_post_notify 是否启用信号通知
 * @note   线程类模板的CRTP特化版,用于创建线程对象,派生类需实现run()函数
 */
template <uint32_t Stack_Size, uint32_t Queue_Size, typename Derived, bool is_enable_signal_post_notify>
class Thread : public system_internal::thread_internal::Thread_Crtp_Base<Queue_Size, is_enable_signal_post_notify, Derived>
{
  // 栈大小检查
  static_assert(system_internal::thread_internal::is_stack_valid(Stack_Size), "Stack size must be 8-byte aligned and >= TX_MINIMUM_STACK");

  // 禁止拷贝与移动
  QAQ_NO_COPY_MOVE(Thread)

private:
  /// @brief 栈空间 - 8字节对齐
  UCHAR m_stack[Stack_Size] QAQ_ALIGN(8);

public:
  /**
   * @brief  线程类 构造函数
   *
   */
  explicit Thread() : system_internal::thread_internal::Thread_Crtp_Base<Queue_Size, is_enable_signal_post_notify, Derived>(m_stack, sizeof(m_stack)) {}

  /**
   * @brief  线程类 析构函数
   *
   */
  virtual ~Thread() {}
};

/// @brief 友元函数声明宏 线程入口函数模板
#define THREAD_TASK                                                                   \
  template <typename T>                                                               \
  friend void QAQ::system::system_internal::thread_internal::thread_entry(ULONG arg); \
                                                                                      \
private:                                                                              \
  void run(void)
} /* namespace thread */
} /* namespace system */
} /* namespace QAQ */

#endif /* __THREAD_HPP__ */
