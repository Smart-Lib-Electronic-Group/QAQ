#ifndef __DEVICE_MANAGER_HPP__
#define __DEVICE_MANAGER_HPP__

#include "device_base.hpp"
#include "thread.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 系统
namespace system
{
/// @brief 命名空间 内部
namespace system_internal
{
/// @brief 名称空间 设备 内部
namespace device_internal
{
/// @brief 设备管理器线程栈大小
static constexpr uint32_t DEVICE_MANAGER_STACK_SIZE         = 1536;
/// @brief 设备管理器线程优先级
static constexpr uint32_t DEVICE_MANAGER_PRIORITY           = 3;
/// @brief 设备管理器 消息队列大小
static constexpr uint32_t DEVICE_MANAGER_MASSAGE_QUEUE_SIZE = 32;

/// @brief 设备事件消息结构体
struct Device_Event_Message
{
  /// @brief 设备基类指针
  Device_Base* device;
  /// @brief 设备事件标志位
  uint32_t     event_bits;
};

/**
 * @brief 设备管理器
 *
 */
class Device_Manager final : private thread::Thread<DEVICE_MANAGER_STACK_SIZE, 0, Device_Manager>
{
  // 禁止拷贝和移动
  NO_COPY_MOVE(Device_Manager)

private:
  /// @brief 设备管理器 消息队列名称
  static constexpr const char* QUEUE_NAME  = "Device Manager Queue";
  /// @brief 设备管理器 线程名称
  static constexpr const char* THREAD_NAME = "Device Manager Thread";

  /// @brief 设备管理器 消息队列类型
  using Queue                              = kernel::Message_Queue<Device_Event_Message, DEVICE_MANAGER_MASSAGE_QUEUE_SIZE>;
  /// @brief 设备管理器 消息队列
  Queue m_queue;

  /// @brief 设备管理器 线程任务
  THREAD_TASK
  {
    Device_Event_Message message;
    while (true)
    {
      if (m_queue.receive(message, TX_WAIT_FOREVER) == Queue::Status::SUCCESS)
      {
        if (nullptr != message.device)
          message.device->manger_handler(message.event_bits);

        message.device     = nullptr;
        message.event_bits = 0;
      }
    }
  }

private:
  /**
   * @brief 设备管理器 构造函数
   *
   */
  explicit Device_Manager() : Thread(), m_queue(QUEUE_NAME)
  {
    create(THREAD_NAME, DEVICE_MANAGER_PRIORITY);
    start();
  }

  /**
   * @brief 析构函数
   *
   */
  ~Device_Manager() {}

public:
  /**
   * @brief  设备管理器 获取单例
   *
   * @return Device_Manager& 设备管理器单例
   */
  static Device_Manager& instance(void)
  {
    static Device_Manager instance;
    return instance;
  }

  /**
   * @brief  设备管理器 添加设备事件
   *
   * @param  device     设备基类指针
   * @param  event_bits 设备事件标志位
   * @return true       成功
   * @return false      失败
   */
  bool post_event(Device_Base* device, uint32_t event_bits)
  {
    Device_Event_Message message;
    message.device     = device;
    message.event_bits = event_bits;
    return m_queue.send(message, 0) == Queue::Status::SUCCESS;
  }
};
} /* namespace device_internal */
} /* namespace system_internal */
} /* namespace system */
} /* namespace QAQ */

#endif /* __DEVICE_MANAGER_HPP__ */
