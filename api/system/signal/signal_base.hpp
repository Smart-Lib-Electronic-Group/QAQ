#ifndef __SIGNAL_BASE_HPP__
#define __SIGNAL_BASE_HPP__

#include "system_include.hpp"
#include "semaphore.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 系统
namespace system
{
/// @brief 名称空间 信号
namespace signal
{
/// @brief 信号连接模式
enum class Connection_Type : uint8_t
{
  Auto_Connection,           /* 自动选择投递方式 */
  Direct_Connection,         /* 直接执行 */
  Object_Queue_Connection,   /* 投递到对象消息队列 */
  Thread_Queue_Connection,   /* 投递到线程消息队列 */
  Blocking_Queue_Connection, /* 阻塞等待 */
};

/// @brief 信号错误代码
enum class Signal_Error_Code : uint8_t
{
  SUCCESS = 0x40,                   /* 成功 */
  TYPE_ERROR,                       /* 类型错误 */
  QUEUE_FULL,                       /* 队列满 */
  NULL_POINTER,                     /* 空指针 */
  OUT_OF_MEMORY,                    /* 内存不足 */
  ALREADY_CONNECTED,                /* 连接已存在 */
  OBJECT_DESTROYED,                 /* 对象已销毁 */
  EMIT_TIMEOUT,                     /* 发送超时 */
  RECEIVE_NO_FIND,                  /* 无法找到接收对象 */
  RECEIVE_NO_QUEUE,                 /* 接收对象无消息队列 */
  RECEIVE_NO_AFFINITY_THREAD,       /* 接收对象无关联线程 */
  RECEIVE_AFFINITY_THREAD_NO_QUEUE, /* 接收对象关联线程无消息队列 */
};
} /* namespace signal */
/// @brief 名称空间 内部
namespace system_internal
{
/// @brief 名称空间 对象 内部
namespace object_internal
{
class Object_Base;
} /* namespace object_internal */

/// @brief 名称空间 信号 内部
namespace signal_internal
{
class Signal_Data_Base;

/**
 * @brief 信号基类
 *
 */
class Signal_Base
{
  // 禁止拷贝和移动
  NO_COPY_MOVE(Signal_Base)

public:
  /**
   * @brief 信号基类 构造函数
   *
   */
  Signal_Base() {}

  /**
   * @brief 信号基类 析构函数
   *
   */
  virtual ~Signal_Base() {}
};

/**
 * @brief 信号管理器基类
 *
 */
class Signal_Manager_Base
{
  /// @brief 友元声明 对象基类
  friend class object_internal::Object_Base;
  /// @brief 友元声明 信号数据基础结构体
  friend class Signal_Data_Base;

  // 禁止拷贝和移动
  NO_COPY_MOVE(Signal_Manager_Base)

private:
  virtual void deallocate(void* ptr, uint32_t size) {}
  virtual void deallocate_semaphore(void* ptr) {}
  virtual void disconnect_signal(Signal_Base* signal) {}
  virtual void disconnect_receiver(object_internal::Object_Base* receiver) {}

protected:
  Signal_Manager_Base() {}
  virtual ~Signal_Manager_Base() {}
};

extern Signal_Manager_Base* __signal_manager_base;

/**
 * @brief 信号数据基础结构体 纯虚类
 *
 */
struct Signal_Data_Base
{
protected:
  /**
   * @brief 信号数据基类 内存释放接口
   *
   * @param ptr  数据指针
   * @param size 数据大小
   */
  void deallocate(void* ptr, uint32_t size) noexcept
  {
    __signal_manager_base->deallocate(ptr, size);
  }

  /**
   * @brief 信号数据基类 信号量内存释放接口
   *
   * @param ptr 信号信号量指针
   */
  void deallocate_semaphore(void* ptr) noexcept
  {
    __signal_manager_base->deallocate_semaphore(ptr);
  }

public:
  Signal_Data_Base() {}
  virtual bool execute(void) = 0;
  virtual void destroy(void) = 0;
  virtual ~Signal_Data_Base() {}
};
} /* namespace signal_internal */
} /* namespace system_internal */
} /* namespace system */
} /* namespace QAQ */

#endif /* __SIGNAL_BASE_HPP__ */
