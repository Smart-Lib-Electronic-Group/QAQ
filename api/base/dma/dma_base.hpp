#ifndef __DMA_BASE_HPP__
#define __DMA_BASE_HPP__

#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_rcc.h"
#include "stm32h7xx_ll_dma.h"
#include "interrupt.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 基本对象
namespace base
{
/// @brief 名称空间 DMA
namespace dma
{
/// @brief 回调函数参数类型
using Dma_Callback_Args_t = void*;
/// @brief 回调函数类型
using Dma_Callback_Func_t = void (*)(Dma_Callback_Args_t);

/**
 * @brief DMA 数据传输方向
 */
enum class Dma_Direction : uint8_t
{
  Memory_To_Memory,     /* 内存到内存 */
  Peripheral_To_Memory, /* 外设到内存 */
  Memory_To_Peripheral, /* 内存到外设 */
};

/**
 * @brief DMA 模式
 */
enum class Dma_Mode : uint8_t
{
  Normal,        /* 正常模式 */
  Cyclic,        /* 循环模式 */
  Double_Buffer, /* 双缓冲模式 */
};

/**
 * @brief DMA 优先级
 */
enum class Dma_Priority : uint8_t
{
  Low,       /* 低优先级 */
  Medium,    /* 中等优先级 */
  High,      /* 高优先级 */
  Very_High, /* 非常高优先级 */
};

/**
 * @brief DMA 数据位大小
 */
enum class Dma_Data_Size : uint8_t
{
  Byte,      /*  8bit 比特 */
  Half_Word, /* 16bit 半字节 */
  Word       /* 32bit 字节 */
};

/**
 * @brief DMA 错误码
 */
enum class Dma_Error_Code : uint8_t
{
  OK,                   /* 正常 */
  NO_AVAILABLE_CHANNEL, /* 没有可用的通道 */
  CHANNEL_NOT_OPEN,     /* 通道未打开 */
  CHANNEL_NOT_STOPPED,  /* 通道未停止 */
  ALREADY_OPENED,       /* 通道已打开 */
  ERROR,                /* 错误 */
};

/**
 * @brief  DMA 通道配置 模版类
 *
 * @tparam direction      DMA 方向
 * @tparam mode           DMA 模式
 * @tparam priority       DMA 优先级
 * @tparam src_increment  源数据地址自增
 * @tparam dst_increment  目标数据地址自增
 * @tparam src_data_size  源数据位大小
 * @tparam dst_data_size  目标数据位大小
 */
template <Dma_Direction direction, Dma_Mode mode = Dma_Mode::Normal, Dma_Priority priority = Dma_Priority::Medium, bool src_increment = false, bool dst_increment = false, Dma_Data_Size src_data_size = Dma_Data_Size::Byte, Dma_Data_Size dst_data_size = Dma_Data_Size::Byte>
class Dma_Config
{
  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(Dma_Config)

public:
  /**
   * @brief  DMA 通道配置 是否为双缓冲模式
   *
   * @return true  是
   * @return false 否
   */
  static constexpr bool is_double_buffer(void) noexcept
  {
    return (Dma_Mode::Double_Buffer == mode);
  }

  /**
   * @brief  DMA 通道配置 获取传输方向 LL库编码
   *
   * @return uint32_t 传输方向 LL库编码
   */
  static constexpr uint32_t get_direction(void) noexcept
  {
    if constexpr (Dma_Direction::Memory_To_Memory == direction)
    {
      return LL_DMA_DIRECTION_MEMORY_TO_MEMORY;
    }
    else if constexpr (Dma_Direction::Peripheral_To_Memory == direction)
    {
      return LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
    }
    else if constexpr (Dma_Direction::Memory_To_Peripheral == direction)
    {
      return LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    }
  }

  /**
   * @brief  DMA 通道配置 获取DMA模式 LL库编码
   *
   * @return uint32_t DMA模式 LL库编码
   */
  static constexpr uint32_t get_mode(void) noexcept
  {
    if constexpr (Dma_Mode::Normal == mode)
    {
      return LL_DMA_MODE_NORMAL;
    }
    else if constexpr (Dma_Mode::Cyclic == mode)
    {
      return LL_DMA_MODE_CIRCULAR;
    }
    else if constexpr (Dma_Mode::Double_Buffer == mode)
    {
      return LL_DMA_MODE_CIRCULAR;
    }
  }

  /**
   * @brief  DMA 通道配置 获取DMA优先级 LL库编码
   *
   * @return uint32_t DMA优先级 LL库编码
   */
  static constexpr uint32_t get_priority(void) noexcept
  {
    if constexpr (Dma_Priority::Low == priority)
    {
      return LL_DMA_PRIORITY_LOW;
    }
    else if constexpr (Dma_Priority::Medium == priority)
    {
      return LL_DMA_PRIORITY_MEDIUM;
    }
    else if constexpr (Dma_Priority::High == priority)
    {
      return LL_DMA_PRIORITY_HIGH;
    }
    else if constexpr (Dma_Priority::Very_High == priority)
    {
      return LL_DMA_PRIORITY_VERYHIGH;
    }
  }

  /**
   * @brief  DMA 通道配置 获取DMA外设地址自增模式 LL库编码
   *
   * @return uint32_t DMA外设地址自增模式 LL库编码
   */
  static constexpr uint32_t get_periph_increment(void) noexcept
  {
    if constexpr (Dma_Direction::Memory_To_Peripheral == direction)
    {
      if constexpr (dst_increment)
      {
        return LL_DMA_PERIPH_INCREMENT;
      }
      else
      {
        return LL_DMA_PERIPH_NOINCREMENT;
      }
    }
    else
    {
      if constexpr (src_increment)
      {
        return LL_DMA_PERIPH_INCREMENT;
      }
      else
      {
        return LL_DMA_PERIPH_NOINCREMENT;
      }
    }
  }

  /**
   * @brief  DMA 通道配置 获取DMA内存地址自增模式 LL库编码
   *
   * @return DMA内存地址自增模式 LL库编码
   */
  static constexpr uint32_t get_memory_increment(void) noexcept
  {
    if constexpr (Dma_Direction::Memory_To_Peripheral == direction)
    {
      if constexpr (src_increment)
      {
        return LL_DMA_MEMORY_INCREMENT;
      }
      else
      {
        return LL_DMA_MEMORY_NOINCREMENT;
      }
    }
    else
    {
      if constexpr (dst_increment)
      {
        return LL_DMA_MEMORY_INCREMENT;
      }
      else
      {
        return LL_DMA_MEMORY_NOINCREMENT;
      }
    }
  }

  /**
   * @brief  DMA 通道配置 获取DMA外设数据位大小 LL库编码
   *
   * @return uint32_t DMA外设数据位大小 LL库编码
   */
  static constexpr uint32_t get_periph_data_size(void) noexcept
  {
    if constexpr (Dma_Direction::Memory_To_Peripheral == direction)
    {
      if constexpr (Dma_Data_Size::Byte == dst_data_size)
      {
        return LL_DMA_PDATAALIGN_BYTE;
      }
      else if constexpr (Dma_Data_Size::Half_Word == dst_data_size)
      {
        return LL_DMA_PDATAALIGN_HALFWORD;
      }
      else if constexpr (Dma_Data_Size::Word == dst_data_size)
      {
        return LL_DMA_PDATAALIGN_WORD;
      }
    }
    else
    {
      if constexpr (Dma_Data_Size::Byte == src_data_size)
      {
        return LL_DMA_PDATAALIGN_BYTE;
      }
      else if constexpr (Dma_Data_Size::Half_Word == src_data_size)
      {
        return LL_DMA_PDATAALIGN_HALFWORD;
      }
      else if constexpr (Dma_Data_Size::Word == src_data_size)
      {
        return LL_DMA_PDATAALIGN_WORD;
      }
    }
  }

  /**
   * @brief  DMA 通道配置 获取DMA内存数据位大小 LL库编码
   *
   * @return uint32_t DMA内存数据位大小 LL库编码
   */
  static constexpr uint32_t get_memory_data_size(void) noexcept
  {
    if constexpr (Dma_Direction::Memory_To_Peripheral == direction)
    {
      if constexpr (Dma_Data_Size::Byte == src_data_size)
      {
        return LL_DMA_MDATAALIGN_BYTE;
      }
      else if constexpr (Dma_Data_Size::Half_Word == src_data_size)
      {
        return LL_DMA_MDATAALIGN_HALFWORD;
      }
      else if constexpr (Dma_Data_Size::Word == src_data_size)
      {
        return LL_DMA_MDATAALIGN_WORD;
      }
    }
    else
    {
      if constexpr (Dma_Data_Size::Byte == dst_data_size)
      {
        return LL_DMA_MDATAALIGN_BYTE;
      }
      else if constexpr (Dma_Data_Size::Half_Word == dst_data_size)
      {
        return LL_DMA_MDATAALIGN_HALFWORD;
      }
      else if constexpr (Dma_Data_Size::Word == dst_data_size)
      {
        return LL_DMA_MDATAALIGN_WORD;
      }
    }
  }
};
}   // namespace dma

/// @brief 命名空间 内部
namespace base_internal
{
/// @brief 名称空间 DMA 内部
namespace dma_internal
{
/**
 * @brief DMA 基类
 */
class Dma_Base : public interrupt::Interrupt_Device
{
  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(Dma_Base)

protected:
  /// @brief DMA 通道 类型声明
  using Dma_Reg        = DMA_TypeDef*;
  /// @brief DMA 通道 类型声明
  using Dma_Const_Reg  = const DMA_TypeDef*;
  /// @brief DMA 中断类型 声明
  using Interrupt_Type = interrupt::Interrupt_Type;
  /// @brief DMA 错误码 类型声明
  using Dma_Error_Code = dma::Dma_Error_Code;

  /// @brief DMA 状态枚举
  enum class State : uint8_t
  {
    CLOSEED, /* DMA 关闭状态 */
    OPENED,  /* DMA 开启状态 */
    WORKING, /* DMA 工作状态 */
    STOPPED, /* DMA 暂停状态 */
  };

private:
  /// @brief DMA 回调函数 数据结构体
  struct Dma_Callback_Data
  {
    dma::Dma_Callback_Func_t direct_function; /* 同步回调函数 - 中断服务函数内执行 */
    dma::Dma_Callback_Func_t queue_function;  /* 异步回调函数 - 管理器线程内执行 */
    dma::Dma_Callback_Args_t arg;             /* 回调函数参数 */
    Interrupt_Type           type;            /* 中断函数触发类型 */
  };

  /// @brief DMA 最大通道数
  static constexpr uint8_t MAX_CHANNEL                                              = 16;

  /// @brief DMA 中断通道
  static constexpr interrupt::Interrupt_Channel_t dma_internal_channel[MAX_CHANNEL] = { DMA1_Stream0_IRQn, DMA1_Stream1_IRQn, DMA1_Stream2_IRQn, DMA1_Stream3_IRQn, DMA1_Stream4_IRQn, DMA1_Stream5_IRQn, DMA1_Stream6_IRQn, DMA1_Stream7_IRQn, DMA2_Stream0_IRQn, DMA2_Stream1_IRQn, DMA2_Stream2_IRQn, DMA2_Stream3_IRQn, DMA2_Stream4_IRQn, DMA2_Stream5_IRQn, DMA2_Stream6_IRQn, DMA2_Stream7_IRQn };

  /// @brief DMA 清除传输完成标志函数
  static constexpr void (*dma_clear_tc_flag[MAX_CHANNEL])(Dma_Reg)                  = { LL_DMA_ClearFlag_TC0, LL_DMA_ClearFlag_TC1, LL_DMA_ClearFlag_TC2, LL_DMA_ClearFlag_TC3, LL_DMA_ClearFlag_TC4, LL_DMA_ClearFlag_TC5, LL_DMA_ClearFlag_TC6, LL_DMA_ClearFlag_TC7, LL_DMA_ClearFlag_TC0, LL_DMA_ClearFlag_TC1, LL_DMA_ClearFlag_TC2, LL_DMA_ClearFlag_TC3, LL_DMA_ClearFlag_TC4, LL_DMA_ClearFlag_TC5, LL_DMA_ClearFlag_TC6, LL_DMA_ClearFlag_TC7 };

  /// @brief DMA 清除传输错误标志函数
  static constexpr void (*dma_clear_te_flag[MAX_CHANNEL])(Dma_Reg)                  = { LL_DMA_ClearFlag_TE0, LL_DMA_ClearFlag_TE1, LL_DMA_ClearFlag_TE2, LL_DMA_ClearFlag_TE3, LL_DMA_ClearFlag_TE4, LL_DMA_ClearFlag_TE5, LL_DMA_ClearFlag_TE6, LL_DMA_ClearFlag_TE7, LL_DMA_ClearFlag_TE0, LL_DMA_ClearFlag_TE1, LL_DMA_ClearFlag_TE2, LL_DMA_ClearFlag_TE3, LL_DMA_ClearFlag_TE4, LL_DMA_ClearFlag_TE5, LL_DMA_ClearFlag_TE6, LL_DMA_ClearFlag_TE7 };

  /// @brief DMA 清除半传输完成标志函数
  static constexpr void (*dma_clear_ht_flag[MAX_CHANNEL])(Dma_Reg)                  = { LL_DMA_ClearFlag_HT0, LL_DMA_ClearFlag_HT1, LL_DMA_ClearFlag_HT2, LL_DMA_ClearFlag_HT3, LL_DMA_ClearFlag_HT4, LL_DMA_ClearFlag_HT5, LL_DMA_ClearFlag_HT6, LL_DMA_ClearFlag_HT7, LL_DMA_ClearFlag_HT0, LL_DMA_ClearFlag_HT1, LL_DMA_ClearFlag_HT2, LL_DMA_ClearFlag_HT3, LL_DMA_ClearFlag_HT4, LL_DMA_ClearFlag_HT5, LL_DMA_ClearFlag_HT6, LL_DMA_ClearFlag_HT7 };

  /// @brief DMA 清除DMA错误标志函数
  static constexpr void (*dma_clear_dme_flag[MAX_CHANNEL])(Dma_Reg)                 = { LL_DMA_ClearFlag_DME0, LL_DMA_ClearFlag_DME1, LL_DMA_ClearFlag_DME2, LL_DMA_ClearFlag_DME3, LL_DMA_ClearFlag_DME4, LL_DMA_ClearFlag_DME5, LL_DMA_ClearFlag_DME6, LL_DMA_ClearFlag_DME7, LL_DMA_ClearFlag_DME0, LL_DMA_ClearFlag_DME1, LL_DMA_ClearFlag_DME2, LL_DMA_ClearFlag_DME3, LL_DMA_ClearFlag_DME4, LL_DMA_ClearFlag_DME5, LL_DMA_ClearFlag_DME6, LL_DMA_ClearFlag_DME7 };

  /// @brief DMA 清除FIFO错误标志函数
  static constexpr void (*dma_clear_fe_flag[MAX_CHANNEL])(Dma_Reg)                  = { LL_DMA_ClearFlag_FE0, LL_DMA_ClearFlag_FE1, LL_DMA_ClearFlag_FE2, LL_DMA_ClearFlag_FE3, LL_DMA_ClearFlag_FE4, LL_DMA_ClearFlag_FE5, LL_DMA_ClearFlag_FE6, LL_DMA_ClearFlag_FE7, LL_DMA_ClearFlag_FE0, LL_DMA_ClearFlag_FE1, LL_DMA_ClearFlag_FE2, LL_DMA_ClearFlag_FE3, LL_DMA_ClearFlag_FE4, LL_DMA_ClearFlag_FE5, LL_DMA_ClearFlag_FE6, LL_DMA_ClearFlag_FE7 };

  /// @brief DMA 获取传输完成标志函数
  static constexpr uint32_t (*dma_is_active_tc_flag[MAX_CHANNEL])(Dma_Const_Reg)    = { LL_DMA_IsActiveFlag_TC0, LL_DMA_IsActiveFlag_TC1, LL_DMA_IsActiveFlag_TC2, LL_DMA_IsActiveFlag_TC3, LL_DMA_IsActiveFlag_TC4, LL_DMA_IsActiveFlag_TC5, LL_DMA_IsActiveFlag_TC6, LL_DMA_IsActiveFlag_TC7, LL_DMA_IsActiveFlag_TC0, LL_DMA_IsActiveFlag_TC1, LL_DMA_IsActiveFlag_TC2, LL_DMA_IsActiveFlag_TC3, LL_DMA_IsActiveFlag_TC4, LL_DMA_IsActiveFlag_TC5, LL_DMA_IsActiveFlag_TC6, LL_DMA_IsActiveFlag_TC7 };

  /// @brief DMA 回调数据结构体数组
  static inline Dma_Callback_Data m_callback_data[MAX_CHANNEL]                      = { 0 };
  /// @brief DMA 通道激活标志
  static inline uint16_t          m_activate_channel_flag                           = 0;

  /// @brief DMA 当前通道编号
  int8_t   m_channel                                                                = -1;
  /// @brief DMA 当前数据流 LL库编码
  uint32_t m_stream                                                                 = 0;
  /// @brief DMA 当前通道
  Dma_Reg  m_dma                                                                    = nullptr;

protected:
  /// @brief DMA 状态
  State    m_state       = State::CLOSEED;
  /// @brief DMA 缓存大小
  uint32_t m_memory_size = 0;

private:
  /**
   * @brief DMA 基类 更新通道信息
   *
   * @param channel 通道编号
   */
  void update_channel(int8_t channel)
  {
    if (channel >= 0 && channel < MAX_CHANNEL / 2)
    {
      m_dma    = DMA1;
      m_stream = LL_DMA_STREAM_0 + channel;
    }
    else if (channel >= MAX_CHANNEL / 2 && channel < MAX_CHANNEL)
    {
      m_dma    = DMA2;
      m_stream = LL_DMA_STREAM_0 + (channel - MAX_CHANNEL / 2);
    }
  }

  /**
   * @brief  DMA 基类 同步中断回调函数
   *
   * @tparam channel 通道编号
   * @param  arg     入口参数
   */
  template <uint8_t channel>
  static void irq_direct_handler(interrupt::Interrupt_Args_t arg, uint8_t)
  {
    static_assert(channel < MAX_CHANNEL, "DMA channel number error");

    Dma_Base* dma = static_cast<Dma_Base*>(arg);

    if (dma_is_active_tc_flag[channel](dma->m_dma) && LL_DMA_IsEnabledIT_TC(dma->m_dma, channel % 8))
    {
      if (Interrupt_Type::Queue != m_callback_data[channel].type)
      {
        if (m_callback_data[channel].direct_function)
        {
          m_callback_data[channel].direct_function(m_callback_data[channel].arg);
        }
      }

      if (Interrupt_Type::Queue == m_callback_data[channel].type || Interrupt_Type::Mixed == m_callback_data[channel].type)
      {
        dma->send_to_queue(dma_internal_channel[channel], 0);
      }

      dma_clear_tc_flag[channel](dma->m_dma);
    }
  }

  /**
   * @brief  DMA 基类 异步中断回调函数
   *
   * @tparam channel 通道编号
   * @param  arg     入口参数
   */
  template <uint8_t channel>
  static void irq_queue_handler(interrupt::Interrupt_Args_t arg, uint8_t)
  {
    static_assert(channel < MAX_CHANNEL, "DMA channel number error");

    Dma_Base* dma = static_cast<Dma_Base*>(arg);

    if (dma->m_callback_data[channel].queue_function)
    {
      dma->m_callback_data[channel].queue_function(dma->m_callback_data[channel].arg);
    }
  }

  /**
   * @brief  DMA 基类 获取中断通道
   *
   * @param  channel              通道编号
   * @return Interrupt_Channel_t  中断通道
   */
  static interrupt::Interrupt_Channel_t get_interrupt_channel(uint8_t channel)
  {
    return dma_internal_channel[channel];
  }

  /**
   * @brief  DMA 基类 获取同步中断回调函数
   *
   * @param  channel          通道编号
   * @return Interrupt_Func_t 同步中断回调函数
   */
  interrupt::Interrupt_Func_t get_direct_func(uint8_t channel)
  {
    static constexpr interrupt::Interrupt_Func_t func[] = { irq_direct_handler<0>, irq_direct_handler<1>, irq_direct_handler<2>, irq_direct_handler<3>, irq_direct_handler<4>, irq_direct_handler<5>, irq_direct_handler<6>, irq_direct_handler<7>, irq_direct_handler<8>, irq_direct_handler<9>, irq_direct_handler<10>, irq_direct_handler<11>, irq_direct_handler<12>, irq_direct_handler<13>, irq_direct_handler<14>, irq_direct_handler<15> };

    if (channel < 16)
    {
      return func[channel];
    }
    else
    {
      return nullptr;
    }
  }

  /**
   * @brief  DMA 基类 获取异步中断回调函数
   *
   * @param  channel          通道编号
   * @return Interrupt_Func_t 异步中断回调函数
   */
  interrupt::Interrupt_Func_t get_queue_func(uint8_t channel)
  {
    static constexpr interrupt::Interrupt_Func_t func[] = { irq_queue_handler<0>, irq_queue_handler<1>, irq_queue_handler<2>, irq_queue_handler<3>, irq_queue_handler<4>, irq_queue_handler<5>, irq_queue_handler<6>, irq_queue_handler<7>, irq_queue_handler<8>, irq_queue_handler<9>, irq_queue_handler<10>, irq_queue_handler<11>, irq_queue_handler<12>, irq_queue_handler<13>, irq_queue_handler<14>, irq_queue_handler<15> };

    if (channel < 16)
    {
      return func[channel];
    }
    else
    {
      return nullptr;
    }
  }

protected:
  /**
   * @brief  DMA 基类 构造函数
   */
  explicit Dma_Base() {}

  /**
   * @brief  DMA 基类 分配通道
   *
   * @return true  分配成功
   * @return false 分配失败
   */
  bool distribution_channel(void)
  {
    bool ret = false;

    if (m_channel == -1)
    {
      for (uint8_t i = 0; i < MAX_CHANNEL; i++)
      {
        if ((m_activate_channel_flag & (1 << i)) == 0)
        {
          m_channel                = i;
          m_activate_channel_flag |= (1 << i);
          break;
        }
      }

      if (m_channel != -1)
      {
        update_channel(m_channel);
        ret = true;
      }
    }
    else
    {
      ret = true;
    }

    return ret;
  }

  /**
   * @brief  DMA 基类 释放通道
   */
  void return_channel(void)
  {
    if (m_channel != -1)
    {
      m_activate_channel_flag &= ~(1 << m_channel);
      m_channel                = -1;
      m_dma                    = nullptr;
      m_stream                 = 0;
    }
    else
    {
      m_dma    = nullptr;
      m_stream = 0;
    }
  }

  /**
   * @brief  DMA 基类 获取当前DMA通道
   *
   * @return Dma_Reg  DMA通道
   */
  Dma_Reg get_dma(void)
  {
    return m_dma;
  }

  /**
   * @brief  DMA 基类 获取当前数据流 LL库编码
   *
   * @return uint32_t 数据流 LL库编码
   */
  uint32_t get_stream(void)
  {
    return m_stream;
  }

  /**
   * @brief  DMA 基类 注册中断
   *
   * @param  direct_function 同步中断回调函数
   * @param  queue_function  异步中断回调函数
   * @param  arg             中断回调函数参数
   * @param  type            中断触发类型
   * @param  priority        中断优先级
   * @param  subpriority     中断子优先级
   * @return true            注册成功
   * @return false           注册失败
   */
  bool register_interrupt(dma::Dma_Callback_Func_t direct_function, dma::Dma_Callback_Func_t queue_function, dma::Dma_Callback_Args_t arg, Interrupt_Type type, uint32_t priority, uint32_t subpriority)
  {
    bool ret = false;
    if (-1 != m_channel)
    {
      m_callback_data[m_channel].direct_function = direct_function;
      m_callback_data[m_channel].queue_function  = queue_function;
      m_callback_data[m_channel].arg             = arg;
      m_callback_data[m_channel].type            = type;

      ret                                        = register_device(get_interrupt_channel(m_channel), get_direct_func(m_channel), get_queue_func(m_channel), this, priority, subpriority);
    }
    return ret;
  }

  /**
   * @brief  DMA 基类 注销中断
   */
  void unregister_interrupt(void)
  {
    unregister_device(get_interrupt_channel(m_channel));
  }

  /**
   * @brief  DMA 基类 清除中断标志位
   */
  void clear_interrupt_flag(void)
  {
    if (m_channel >= 0 && m_channel < MAX_CHANNEL)
    {
      dma_clear_tc_flag[m_channel](m_dma);
      dma_clear_ht_flag[m_channel](m_dma);
      dma_clear_te_flag[m_channel](m_dma);
      dma_clear_fe_flag[m_channel](m_dma);
      dma_clear_dme_flag[m_channel](m_dma);
    }
  }

  /**
   * @brief  DMA 基类 析构函数
   */
  virtual ~Dma_Base() {}

public:
  /**
   * @brief  DMA 类 获取传输大小
   *
   * @return uint32_t DMA 传输大小
   */
  uint32_t get_transferred_size(void)
  {
    uint32_t transferred_size = 0;

    if (State::WORKING == m_state || State::STOPPED == m_state)
    {
      transferred_size = m_memory_size - LL_DMA_GetDataLength(get_dma(), get_stream());
    }

    return transferred_size;
  }

  void set

  /**
   * @brief  DMA 类 关闭传输
   *
   * @return Dma_Error_Code DMA 错误码
   */
  Dma_Error_Code stop(void)
  {
    Dma_Error_Code error_code = Dma_Error_Code::OK;

    if (State::CLOSEED == m_state)
    {
      error_code = Dma_Error_Code::CHANNEL_NOT_OPEN;
    }
    else
    {
      LL_DMA_DisableIT_TC(get_dma(), get_stream());
      LL_DMA_DisableStream(get_dma(), get_stream());

      while (LL_DMA_IsEnabledStream(get_dma(), get_stream()))
      {
      }

      clear_interrupt_flag();

      m_state = State::STOPPED;
    }

    return error_code;
  }

  /**
   * @brief  DMA 类 恢复传输
   *
   * @return Dma_Error_Code  DMA 错误码
   */
  Dma_Error_Code resume(void)
  {
    Dma_Error_Code error_code = Dma_Error_Code::OK;

    if (State::STOPPED != m_state)
    {
      error_code = Dma_Error_Code::CHANNEL_NOT_STOPPED;
    }
    else
    {
      LL_DMA_EnableIT_TC(get_dma(), get_stream());
      LL_DMA_EnableStream(get_dma(), get_stream());

      m_state = State::WORKING;
    }

    return error_code;
  }

  /**
   * @brief  DMA 类 关闭通道
   *
   * @return Dma_Error_Code  DMA 错误码
   */
  Dma_Error_Code close(void)
  {
    Dma_Error_Code error_code = Dma_Error_Code::OK;

    if (State::CLOSEED == m_state)
    {
      error_code = Dma_Error_Code::CHANNEL_NOT_OPEN;
    }
    else
    {
      LL_DMA_DisableIT_TC(get_dma(), get_stream());
      LL_DMA_DisableStream(get_dma(), get_stream());

      while (LL_DMA_IsEnabledStream(get_dma(), get_stream()))
      {
      }

      clear_interrupt_flag();

      unregister_interrupt();
      return_channel();

      m_memory_size = 0;
      m_state       = State::CLOSEED;
    }

    return error_code;
  }
};
} /* namespace dma_internal */
} /* namespace base_internal */
} /* namespace base */
} /* namespace QAQ */

/* ------- 注册中断函数 ------- */
INTERRUPT_HANDLER(DMA1_Stream0)
INTERRUPT_HANDLER(DMA1_Stream1)
INTERRUPT_HANDLER(DMA1_Stream2)
INTERRUPT_HANDLER(DMA1_Stream3)
INTERRUPT_HANDLER(DMA1_Stream4)
INTERRUPT_HANDLER(DMA1_Stream5)
INTERRUPT_HANDLER(DMA1_Stream6)
INTERRUPT_HANDLER(DMA1_Stream7)
INTERRUPT_HANDLER(DMA2_Stream0)
INTERRUPT_HANDLER(DMA2_Stream1)
INTERRUPT_HANDLER(DMA2_Stream2)
INTERRUPT_HANDLER(DMA2_Stream3)
INTERRUPT_HANDLER(DMA2_Stream4)
INTERRUPT_HANDLER(DMA2_Stream5)
INTERRUPT_HANDLER(DMA2_Stream6)
INTERRUPT_HANDLER(DMA2_Stream7)

#endif /* __DMA_BASE_HPP__ */
