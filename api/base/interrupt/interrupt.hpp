#ifndef __INTERRUPT_HPP__
#define __INTERRUPT_HPP__

#include "stm32h743xx.h"
#include "message_queue.hpp"
#include "thread.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 基本对象
namespace base
{
/// @brief 命名空间 内部
namespace base_internal
{
/// @brief 名称空间 中断 内部
namespace interrupt_internal
{
/// @brief 中断管理器 线程栈大小
static constexpr uint32_t INTERRUPT_MANAGER_STACK_SIZE         = 1536;
/// @brief 中断管理器 线程优先级
static constexpr uint32_t INTERRUPT_MANAGER_PRIORITY           = 2;
/// @brief 中断管理器 消息队列大小
static constexpr uint32_t INTERRUPT_MANAGER_MASSAGE_QUEUE_SIZE = 32;
} /* namespace interrupt_internal */
} /* namespace base_internal */

/// @brief 名称空间 中断
namespace interrupt
{
/// @brief 中断处理函数参数
using Interrupt_Args_t    = void*;
/// @brief 中断处理函数
using Interrupt_Func_t    = void (*)(Interrupt_Args_t, uint8_t);
/// @brief 中断通道判断函数
using Interrupt_Meas_t    = uint8_t (*)();
/// @brief 中断通道
using Interrupt_Channel_t = IRQn_Type;

/// @brief 中断函数触发类型
enum class Interrupt_Type
{
  Direct, /* 同步触发 */
  Queue,  /* 异步线程触发 */
  Mixed,  /* 混合触发 */
  Device, /* 设备触发 */
};

/// @brief 中断管理器
class Interrupt_Manager final : private system::thread::Thread<base_internal::interrupt_internal::INTERRUPT_MANAGER_STACK_SIZE, 0, Interrupt_Manager>
{
public:
  /// @brief 中断通道数量
  static constexpr uint32_t MAX_INTERRUPTS = 149;

private:
  /// @brief 友元声明 中断设备基类
  friend class Interrupt_Device;

  /// @brief 中断处理信息结构体
  struct Interrupt_Handle
  {
    Interrupt_Type   type;         /* 触发类型 */
    Interrupt_Func_t direct_func;  /* 同步处理函数 */
    Interrupt_Func_t queue_func;   /* 异步线程处理函数 */
    Interrupt_Meas_t measure_func; /* 通道判断函数 */
    Interrupt_Args_t arg;          /* 函数参数 */
  };

  /// @brief 队列数据结构体
  struct Queue_Data
  {
    Interrupt_Channel_t irq;     /* 触发通道 */
    uint8_t             channel; /* 触发通道编号 */
  };

  /// @brief 中断管理器 队列类型
  using Interrupt_Queue_t                  = system::kernel::Message_Queue<Queue_Data, base_internal::interrupt_internal::INTERRUPT_MANAGER_MASSAGE_QUEUE_SIZE>;
  /// @brief 中断管理器 队列名称
  static constexpr const char* QUEUE_NAME  = "Interrupt Manager Queue";
  /// @brief 中断管理器 线程名称
  static constexpr const char* THREAD_NAME = "Interrupt Manager Thread";
  /// @brief 中断管理器 队列实例
  Interrupt_Queue_t            m_queue;
  /// @brief 中断管理器 中断处理信息结构体数组
  Interrupt_Handle             m_interrupts[MAX_INTERRUPTS];

  /// @brief 中断管理器 线程任务
  THREAD_TASK
  {
    Interrupt_Queue_t::Status status;
    Queue_Data                data;
    while (1)
    {
      status = m_queue.receive(data, TX_WAIT_FOREVER);
      if (status == Interrupt_Queue_t::Status::SUCCESS)
      {
        Interrupt_Handle& handle = m_interrupts[data.irq];
        if (handle.type != Interrupt_Type::Direct)
        {
          if (nullptr != handle.queue_func)
          {
            handle.queue_func(handle.arg, data.channel);
          }
        }
      }
    }
  }

private:
  /**
   * @brief 中断管理器 构造函数
   *
   */
  explicit Interrupt_Manager() : m_queue(QUEUE_NAME)
  {
    memset(m_interrupts, 0, sizeof(m_interrupts));
    this->create(THREAD_NAME, base_internal::interrupt_internal::INTERRUPT_MANAGER_PRIORITY);
    this->start();
  }

  /**
   * @brief 中断管理器 析构函数
   *
   */
  ~Interrupt_Manager() {}

  /**
   * @brief  中断管理器 注册中断处理函数
   *
   * @param  irq            中断通道
   * @param  handle         中断处理信息结构体
   * @param  priority       优先级
   * @param  subpriority    子优先级
   * @return true           成功
   * @return false          失败
   */
  bool register_handle(Interrupt_Channel_t irq, Interrupt_Handle& handle, uint32_t priority, uint32_t subpriority)
  {
    if (irq >= static_cast<Interrupt_Channel_t>(MAX_INTERRUPTS))
    {
      return false;
    }

    system::kernel::Interrupt_Guard guard;

    m_interrupts[irq].type         = handle.type;
    m_interrupts[irq].direct_func  = handle.direct_func;
    m_interrupts[irq].queue_func   = handle.queue_func;
    m_interrupts[irq].measure_func = handle.measure_func;
    m_interrupts[irq].arg          = handle.arg;

    set_interrupt_priority(irq, priority, subpriority);
    enable_interrupt(irq);

    return true;
  }

  /**
   * @brief  中断管理器 注销中断处理函数
   *
   * @param  irq       中断通道
   * @return true      成功
   * @return false     失败
   */
  bool unregister_handle(Interrupt_Channel_t irq)
  {
    if (static_cast<uint32_t>(irq) >= MAX_INTERRUPTS)
    {
      return false;
    }

    system::kernel::Interrupt_Guard guard;

    m_interrupts[irq].type         = Interrupt_Type::Direct;
    m_interrupts[irq].direct_func  = nullptr;
    m_interrupts[irq].queue_func   = nullptr;
    m_interrupts[irq].measure_func = nullptr;
    m_interrupts[irq].arg          = nullptr;

    disable_interrupt(irq);

    return true;
  }

  /**
   * @brief  中断管理器 注册中断设备
   *
   * @param  irq            中断通道
   * @param  direct_func    同步处理函数
   * @param  queue_func     异步线程处理函数
   * @param  arg            函数参数
   * @param  priority       优先级
   * @param  subpriority    子优先级
   * @return true          成功
   * @return false         失败
   */
  bool register_device(Interrupt_Channel_t irq, Interrupt_Func_t direct_func, Interrupt_Func_t queue_func, Interrupt_Args_t arg, uint32_t priority, uint32_t subpriority)
  {
    Interrupt_Handle handle = { Interrupt_Type::Device, direct_func, queue_func, nullptr, arg };
    return register_handle(irq, handle, priority, subpriority);
  }

public:
  /**
   * @brief  中断管理器 获取单例对象
   *
   * @return Interrupt_Manager& 单例对象
   */
  static Interrupt_Manager& get_instance()
  {
    static Interrupt_Manager instance;
    return instance;
  }

  /**
   * @brief 中断管理器 中断处理函数
   *
   * @note  该函数由中断服务程序调用，不应在外部调用
   * @param irq       中断通道
   */
  void irq_handler(Interrupt_Channel_t irq)
  {
    Interrupt_Handle& handle = m_interrupts[irq];

    if (handle.measure_func == nullptr)
    {
      if (handle.type == Interrupt_Type::Direct || handle.type == Interrupt_Type::Device)
      {
        handle.direct_func(handle.arg, 0);
      }
      else if (handle.type == Interrupt_Type::Queue)
      {
        Queue_Data data = { irq, 0 };
        m_queue.send(data);
      }
      else if (handle.type == Interrupt_Type::Mixed)
      {
        Queue_Data data = { irq, 0 };
        handle.direct_func(handle.arg, 0);
        m_queue.send(data);
      }
    }
    else
    {
      uint8_t channel = handle.measure_func();

      if (handle.type == Interrupt_Type::Direct || handle.type == Interrupt_Type::Device)
      {
        handle.direct_func(handle.arg, channel);
      }
      else if (handle.type == Interrupt_Type::Queue)
      {
        Queue_Data data = { irq, channel };
        m_queue.send(data);
      }
      else if (handle.type == Interrupt_Type::Mixed)
      {
        Queue_Data data = { irq, channel };
        handle.direct_func(handle.arg, channel);
        m_queue.send(data);
      }
    }
  }

  /**
   * @brief  中断管理器 注册中断处理函数 (同步或异步模式)
   *
   * @param  irq          中断通道
   * @param  function     中断处理函数
   * @param  arg          函数参数
   * @param  measure_func 通道判断函数
   * @param  type         中断触发类型
   * @param  priority     优先级
   * @param  subpriority  子优先级
   * @return true         成功
   * @return false        失败
   */
  bool register_interrupt(Interrupt_Channel_t irq, Interrupt_Func_t function, Interrupt_Args_t arg, Interrupt_Meas_t measure_func, Interrupt_Type type, uint32_t priority, uint32_t subpriority)
  {
    if (Interrupt_Type::Direct == type)
    {
      Interrupt_Handle handle = { Interrupt_Type::Direct, function, nullptr, measure_func, arg };
      return register_handle(irq, handle, priority, subpriority);
    }
    else if (Interrupt_Type::Queue == type)
    {
      Interrupt_Handle handle = { Interrupt_Type::Queue, nullptr, function, measure_func, arg };
      return register_handle(irq, handle, priority, subpriority);
    }
    else
    {
      return false;
    }
  }

  /**
   * @brief  中断管理器 注册中断处理函数 (混合模式)
   *
   * @param  irq          中断通道
   * @param  direct_func  同步中断处理函数
   * @param  queue_func   异步中断处理函数
   * @param  arg          函数参数
   * @param  measure_func 通道判断函数
   * @param  priority     优先级
   * @param  subpriority  子优先级
   * @return true         成功
   * @return false        失败
   */
  bool register_interrupt(Interrupt_Channel_t irq, Interrupt_Func_t direct_func, Interrupt_Func_t queue_func, Interrupt_Args_t arg, Interrupt_Meas_t measure_func, uint32_t priority, uint32_t subpriority)
  {
    Interrupt_Handle handle = { Interrupt_Type::Mixed, direct_func, queue_func, measure_func, arg };
    return register_handle(irq, handle, priority, subpriority);
  }

  /**
   * @brief  中断管理器 注销中断处理函数
   *
   * @param  irq      中断通道
   * @return true     成功
   * @return false    失败
   */
  bool unregister_interrupt(Interrupt_Channel_t irq)
  {
    return unregister_handle(irq);
  }

  /**
   * @brief 中断管理器 设置中断优先级
   *
   * @param irq         中断通道
   * @param priority    优先级
   * @param subpriority 子优先级
   */
  void set_interrupt_priority(Interrupt_Channel_t irq, uint32_t priority, uint32_t subpriority)
  {
    NVIC_SetPriority(irq, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), priority, subpriority));
  }

  /**
   * @brief 中断管理器 设置中断优先级
   *
   * @param irq         中断通道
   * @param priority    优先级
   */
  void set_interrupt_priority(Interrupt_Channel_t irq, uint32_t priority)
  {
    uint32_t subpriority = get_interrupt_subpriority(irq);
    NVIC_SetPriority(irq, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), priority, subpriority));
  }

  /**
   * @brief 中断管理器 设置中断子优先级
   *
   * @param irq         中断通道
   * @param subpriority 子优先级
   */
  void set_interrupt_subpriority(Interrupt_Channel_t irq, uint32_t subpriority)
  {
    uint32_t priority = get_interrupt_priority(irq);
    NVIC_SetPriority(irq, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), priority, subpriority));
  }

  /**
   * @brief 中断管理器 获取中断优先级
   *
   * @param irq 中断通道
   * @return uint32_t 优先级
   */
  const uint32_t get_interrupt_priority(Interrupt_Channel_t irq)
  {
    uint32_t priority    = 0;
    uint32_t subpriority = 0;
    NVIC_DecodePriority(NVIC_GetPriority(irq), NVIC_GetPriorityGrouping(), &priority, &subpriority);
    return priority;
  }

  /**
   * @brief 中断管理器 获取中断子优先级
   *
   * @param irq 中断通道
   * @return uint32_t 子优先级
   */
  const uint32_t get_interrupt_subpriority(Interrupt_Channel_t irq)
  {
    uint32_t priority    = 0;
    uint32_t subpriority = 0;
    NVIC_DecodePriority(NVIC_GetPriority(irq), NVIC_GetPriorityGrouping(), &priority, &subpriority);
    return subpriority;
  }

  /**
   * @brief  中断管理器 使能中断
   *
   * @param  irq      中断通道
   */
  void enable_interrupt(Interrupt_Channel_t irq)
  {
    NVIC_EnableIRQ(irq);
  }

  /**
   * @brief  中断管理器 禁止中断
   *
   * @param  irq      中断通道
   */
  void disable_interrupt(Interrupt_Channel_t irq)
  {
    NVIC_DisableIRQ(irq);
  }
};

/**
 * @brief  中断设备基类
 *
 */
class Interrupt_Device
{
protected:
  /**
   * @brief 中断设备基类 发送中断信息到队列
   *
   * @param irq     中断通道
   * @param channel 通道编号
   */
  static void send_to_queue(Interrupt_Channel_t irq, uint8_t channel)
  {
    Interrupt_Manager::get_instance().m_queue.send({ irq, channel });
  }

  /**
   * @brief 中断设备基类 注册中断设备
   *
   * @param irq         中断通道
   * @param direct_func 同步处理函数
   * @param queue_func  异步处理函数
   * @param arg         函数参数
   * @param priority    优先级
   * @param subpriority 子优先级
   * @return true       成功
   * @return false      失败
   */
  static bool register_device(Interrupt_Channel_t irq, Interrupt_Func_t direct_func, Interrupt_Func_t queue_func, Interrupt_Args_t arg, uint32_t priority, uint32_t subpriority)
  {
    return Interrupt_Manager::get_instance().register_device(irq, direct_func, queue_func, arg, priority, subpriority);
  }

  /**
   * @brief 中断设备基类 注销中断设备
   *
   * @param irq        中断通道
   * @return true      成功
   * @return false     失败
   */
  static bool unregister_device(Interrupt_Channel_t irq)
  {
    return Interrupt_Manager::get_instance().unregister_handle(irq);
  }
};
}   // namespace interrupt
}   // namespace base
} /* namespace QAQ */

extern "C" void trace_isr_enter(void);
extern "C" void trace_isr_exit(void);

/// @brief 定义中断处理函数宏
#define INTERRUPT_HANDLER(irq)                                                       \
  extern "C" void irq##_IRQHandler()                                                 \
  {                                                                                  \
    trace_isr_enter();                                                               \
    QAQ::base::interrupt::Interrupt_Manager::get_instance().irq_handler(irq##_IRQn); \
    trace_isr_exit();                                                                \
  }

#endif /* __INTERRUPT_HPP__ */
