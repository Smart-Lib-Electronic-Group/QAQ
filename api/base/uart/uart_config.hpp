#ifndef __UART_CONFIG_HPP__
#define __UART_CONFIG_HPP__

#include "stm32h7xx_ll_usart.h"
#include "interrupt.hpp"
#include "gpio.hpp"
#include "dma.hpp"
#include "uart_base.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 基本对象
namespace base
{
/// @brief 命名空间 内部
namespace base_internal
{
/// @brief 名称空间 串口 内部
namespace uart_internal
{
/**
 * @brief  Uart 获取句柄
 *
 * @tparam Port           Uart 端口号
 * @return USART_TypeDef* Uart 句柄
 */
template <uint8_t Port>
static constexpr USART_TypeDef* get_handle(void)
{
  if constexpr (1 == Port)
  {
    return USART1;
  }
  else if constexpr (2 == Port)
  {
    return USART2;
  }
  else if constexpr (3 == Port)
  {
    return USART3;
  }
  else if constexpr (4 == Port)
  {
    return UART4;
  }
  else if constexpr (5 == Port)
  {
    return UART5;
  }
  else if constexpr (6 == Port)
  {
    return USART6;
  }
  else if constexpr (7 == Port)
  {
    return UART7;
  }
  else if constexpr (8 == Port)
  {
    return UART8;
  }
  else
  {
    return nullptr;
  }
}

/**
 * @brief  Uart 获取中断通道
 *
 * @tparam Port                 Uart 端口号
 * @return Interrupt_Channel_t  中断通道
 */
template <uint8_t Port>
static constexpr interrupt::Interrupt_Channel_t get_interrupt_channel(void)
{
  if constexpr (1 == Port)
  {
    return USART1_IRQn;
  }
  else if constexpr (2 == Port)
  {
    return USART2_IRQn;
  }
  else if constexpr (3 == Port)
  {
    return USART3_IRQn;
  }
  else if constexpr (4 == Port)
  {
    return UART4_IRQn;
  }
  else if constexpr (5 == Port)
  {
    return UART5_IRQn;
  }
  else if constexpr (6 == Port)
  {
    return USART6_IRQn;
  }
  else if constexpr (7 == Port)
  {
    return UART7_IRQn;
  }
  else if constexpr (8 == Port)
  {
    return UART8_IRQn;
  }
  else
  {
    return interrupt::Interrupt_Manager::MAX_INTERRUPTS + 1;
  }
}

/**
 * @brief  Uart 获取接收 DMA 请求通道
 *
 * @tparam Port       Uart 端口号
 * @return uint32_t   Uart 接收 DMA 请求通道
 */
template <uint8_t Port>
static constexpr uint32_t get_rx_dma_request(void)
{
  if constexpr (1 == Port)
  {
    return LL_DMAMUX1_REQ_USART1_RX;
  }
  else if constexpr (2 == Port)
  {
    return LL_DMAMUX1_REQ_USART2_RX;
  }
  else if constexpr (3 == Port)
  {
    return LL_DMAMUX1_REQ_USART3_RX;
  }
  else if constexpr (4 == Port)
  {
    return LL_DMAMUX1_REQ_UART4_RX;
  }
  else if constexpr (5 == Port)
  {
    return LL_DMAMUX1_REQ_UART5_RX;
  }
  else if constexpr (6 == Port)
  {
    return LL_DMAMUX1_REQ_USART6_RX;
  }
  else if constexpr (7 == Port)
  {
    return LL_DMAMUX1_REQ_UART7_RX;
  }
  else if constexpr (8 == Port)
  {
    return LL_DMAMUX1_REQ_UART8_RX;
  }
  else
  {
    return 0;
  }
}

/**
 * @brief  Uart 获取发送 DMA 请求通道
 *
 * @tparam Port       Uart 端口号
 * @return uint32_t   Uart 发送 DMA 请求通道
 */
template <uint8_t Port>
static constexpr uint32_t get_tx_dma_request(void)
{
  if constexpr (1 == Port)
  {
    return LL_DMAMUX1_REQ_USART1_TX;
  }
  else if constexpr (2 == Port)
  {
    return LL_DMAMUX1_REQ_USART2_TX;
  }
  else if constexpr (3 == Port)
  {
    return LL_DMAMUX1_REQ_USART3_TX;
  }
  else if constexpr (4 == Port)
  {
    return LL_DMAMUX1_REQ_UART4_TX;
  }
  else if constexpr (5 == Port)
  {
    return LL_DMAMUX1_REQ_UART5_TX;
  }
  else if constexpr (6 == Port)
  {
    return LL_DMAMUX1_REQ_USART6_TX;
  }
  else if constexpr (7 == Port)
  {
    return LL_DMAMUX1_REQ_UART7_TX;
  }
  else if constexpr (8 == Port)
  {
    return LL_DMAMUX1_REQ_UART8_TX;
  }
  else
  {
    return 0;
  }
}

/**
 * @brief  Uart 接收 DMA
 *
 * @tparam Type Uart 工作类型
 */
template <uart::Uart_Type Type>
class Uart_Rx_Dma;

/**
 * @brief  Uart 接收 DMA 单缓冲区模式特化
 */
template <>
class Uart_Rx_Dma<uart::Uart_Type::DMA>
{
public:
  /// @brief  Uart 接收 DMA 类型声明
  using Rx_Dma_Config = dma::Dma_Config<dma::Dma_Direction::Peripheral_To_Memory, dma::Dma_Mode::Normal, dma::Dma_Priority::Medium, false, true, dma::Dma_Data_Size::Byte, dma::Dma_Data_Size::Byte>;

  /// @brief  Uart 接收 DMA
  dma::Dma<Rx_Dma_Config> dma;
};

/**
 * @brief  Uart 接收 DMA 双缓冲区模式特化
 */
template <>
class Uart_Rx_Dma<uart::Uart_Type::DMA_Double_Buffer>
{
public:
  /// @brief  Uart 接收 DMA 类型声明
  using Rx_Dma_Config = dma::Dma_Config<dma::Dma_Direction::Peripheral_To_Memory, dma::Dma_Mode::Double_Buffer, dma::Dma_Priority::Medium, false, true, dma::Dma_Data_Size::Byte, dma::Dma_Data_Size::Byte>;

  /// @brief  Uart 接收 DMA
  dma::Dma<Rx_Dma_Config> dma;
};

/**
 * @brief  Uart 接收 DMA
 *
 * @tparam Type Uart 工作类型
 */
template <uart::Uart_Type>
class Uart_Rx_Dma
{
};

/**
 * @brief  Uart 发送 DMA
 *
 * @tparam Type Uart 工作类型
 */
template <uart::Uart_Type Type>
class Uart_Tx_Dma;

/**
 * @brief  Uart 发送 DMA 单缓冲区模式特化
 */
template <>
class Uart_Tx_Dma<uart::Uart_Type::DMA>
{
public:
  /// @brief  Uart 发送 DMA 类型声明
  using Rx_Dma_Config = dma::Dma_Config<dma::Dma_Direction::Memory_To_Peripheral, dma::Dma_Mode::Normal, dma::Dma_Priority::Medium, true, false, dma::Dma_Data_Size::Byte, dma::Dma_Data_Size::Byte>;

  /// @brief  Uart 发送 DMA
  dma::Dma<Rx_Dma_Config> dma;
};

/**
 * @brief  Uart 发送 DMA
 *
 * @tparam Type Uart 工作类型
 */
template <uart::Uart_Type Type>
class Uart_Tx_Dma
{
};

template <system::device::Stream_Type Type, typename Config, typename Base_Device, typename DE_Pin, typename RE_Pin>
class Uart_Base;
} /* namespace uart_internal */
} /* namespace base_internal */

/// @brief 名称空间 串口
namespace uart
{
/**
 * @brief  Uart 配置模版类
 *
 * @tparam Port     UART 端口号
 * @tparam Rx_Type  UART 接收类型
 * @tparam Tx_Type  UART 发送类型
 */
template <uint8_t Port, Uart_Type Rx_Type, Uart_Type Tx_Type>
class Uart_Config : public interrupt::Interrupt_Device
{
  /// @warning 端口合法性判断
  static_assert(1 <= Port && 8 >= Port, "Invalid port number");
  /// @warning 接收类型合法性判断
  static_assert(Uart_Type::Normal != Rx_Type, "Receive type not support Normal");
  /// @warning 发送类型合法性判断
  static_assert(Uart_Type::DMA_Double_Buffer != Tx_Type, "Transmit type not support DMA_Double_Buffer");

  // 禁止拷贝和移动
  NO_COPY_MOVE(Uart_Config)

private:
  /// @brief 友元类声明 Uart_Base 模版类
  template <system::device::Stream_Type Type, typename Config, typename Base_Device, typename DE_Pin, typename RE_Pin>
  friend class base_internal::uart_internal::Uart_Base;

  /// @brief  Uart 回调函数
  using Uart_Func_t     = system::system_internal::device_internal::Device_Func_t;
  /// @brief  Uart 回调函数参数
  using Uart_Args_t     = system::system_internal::device_internal::Device_Args_t;
  /// @brief  Uart 错误码
  using Uart_Error_Code = system::device::Device_Error_Code;

  /// @brief  Uart 回调函数信息结构体
  struct Callback_t
  {
    Uart_Func_t received_byte_callback;     /* 字节接收回调函数 */
    Uart_Args_t received_byte_arg;          /* 字节接收回调函数参数 */
    Uart_Func_t received_complete_callback; /* 接收完成回调函数 */
    Uart_Args_t received_complete_arg;      /* 接收完成回调函数参数 */
    Uart_Func_t send_start_callback;        /* 开始发送回调函数 */
    Uart_Args_t send_start_arg;             /* 开始发送回调函数参数 */
    Uart_Func_t send_complete_callback;     /* 发送完成回调函数 */
    Uart_Args_t send_complete_arg;          /* 发送完成回调函数参数 */
    Uart_Func_t memory_switch_callback;     /* 双缓冲内存切换回调函数 */
    Uart_Args_t memory_switch_arg;          /* 双缓冲内存切换回调函数参数 */
    Uart_Func_t error_callback;             /* 错误回调函数 */
    Uart_Args_t error_arg;                  /* 错误回调函数参数 */
  };

  /// @brief Uart 回调函数信息结构体
  static inline Callback_t                     m_callback              = { 0 };
  /// @brief Uart 句柄
  static inline USART_TypeDef*                 m_handle                = base_internal::uart_internal::get_handle<Port>();
  /// @brief Uart 中断通道
  static inline interrupt::Interrupt_Channel_t m_interrupt_channel     = base_internal::uart_internal::get_interrupt_channel<Port>();
  /// @brief Uart 时钟分频系数
  static inline uint32_t                       m_prescaler_value       = LL_USART_PRESCALER_DIV1;
  /// @brief Uart 中断发送缓存区指针
  static inline const uint8_t*                 m_interrupt_send_buffer = nullptr;
  /// @brief Uart 中断发送缓存区剩余字节大小
  static inline uint8_t                        m_interrupt_send_size   = 0;

  /// @brief Uart 接收 DMA
  static inline base_internal::uart_internal::Uart_Rx_Dma<Rx_Type> m_rx_dma;
  /// @brief Uart 发送 DMA
  static inline base_internal::uart_internal::Uart_Tx_Dma<Tx_Type> m_tx_dma;

  /// @brief Uart 默认中断优先级
  static constexpr uint32_t DEF_INTERRUPT_PRIORITY    = 0x05;
  /// @brief Uart 默认中断子优先级
  static constexpr uint32_t DEF_INTERRUPT_SUBPRIORITY = 0x00;
  /// @brief Uart 中断标志位 空闲中断
  static constexpr uint8_t  IDLE                      = 0x01;
  /// @brief Uart 中断标志位 发送完成中断
  static constexpr uint8_t  TC                        = 0x02;
  /// @brief Uart 中断标志位 校验错误中断
  static constexpr uint8_t  PE                        = 0x04;
  /// @brief Uart 中断标志位 帧错误中断
  static constexpr uint8_t  FE                        = 0x08;
  /// @brief Uart 中断标志位 噪声错误中断
  static constexpr uint8_t  NF                        = 0x10;
  /// @brief Uart 中断标志位 溢出错误中断
  static constexpr uint8_t  ORE                       = 0x20;

private:
  /**
   * @brief Uart 接收 DMA 回调函数
   *
   * @note  该函数仅在接收模式为DMA与DMA双缓冲模式时生效
   */
  template <bool enable = (Uart_Type::DMA == Rx_Type || Uart_Type::DMA_Double_Buffer == Rx_Type), typename = std::enable_if_t<enable>>
  static void dma_rx_callback(dma::Dma_Callback_Args_t)
  {
    if constexpr (Uart_Type::DMA == Rx_Type)
    {
      if (m_callback.received_complete_callback)
      {
        m_callback.received_complete_callback(m_callback.received_complete_arg);
      }
    }
    else if constexpr (Uart_Type::DMA_Double_Buffer == Rx_Type)
    {
      if (m_callback.memory_switch_callback)
      {
        m_callback.memory_switch_callback(m_callback.memory_switch_arg);
      }
    }
  }

  /**
   * @brief Uart 发送 DMA 回调函数
   *
   * @note  该函数仅在发送模式为DMA模式时生效
   */
  template <bool enable = (Uart_Type::DMA == Tx_Type), typename = std::enable_if_t<enable>>
  static void dma_tx_callback(dma::Dma_Callback_Args_t)
  {
    LL_USART_EnableIT_TC(m_handle);
  }

  /**
   * @brief Uart 中断处理 同步函数
   */
  static void irq_direct_handle(interrupt::Interrupt_Args_t, uint8_t)
  {
    if constexpr (Uart_Type::Interrupt == Rx_Type)
    {
      if (LL_USART_IsActiveFlag_RXNE_RXFNE(m_handle))
      {
        if (m_callback.received_byte_callback)
        {
          m_callback.received_byte_callback(m_callback.received_byte_arg);
        }
      }
      else if (LL_USART_IsActiveFlag_IDLE(m_handle))
      {
        if (LL_USART_IsEnabledIT_IDLE(m_handle))
        {
          send_to_queue(m_interrupt_channel, IDLE);
        }
        LL_USART_ClearFlag_IDLE(m_handle);
      }
      else if (LL_USART_IsActiveFlag_PE(m_handle))
      {
        if (LL_USART_IsEnabledIT_PE(m_handle))
        {
          send_to_queue(m_interrupt_channel, PE);
        }
        LL_USART_ClearFlag_PE(m_handle);
      }
      else if (LL_USART_IsActiveFlag_NE(m_handle))
      {
        send_to_queue(m_interrupt_channel, NF);
        LL_USART_ClearFlag_NE(m_handle);
      }
    }
    else if constexpr (Uart_Type::DMA == Rx_Type || Uart_Type::DMA_Double_Buffer == Rx_Type)
    {
      if (LL_USART_IsActiveFlag_IDLE(m_handle))
      {
        if (LL_USART_IsEnabledIT_IDLE(m_handle))
        {
          if (m_rx_dma.dma.get_transferred_size())
          {
            m_rx_dma.dma.stop();
            LL_USART_DisableDMAReq_RX(m_handle);
            send_to_queue(m_interrupt_channel, IDLE);
          }
        }
        LL_USART_ClearFlag_IDLE(m_handle);
      }
      else if (LL_USART_IsActiveFlag_PE(m_handle))
      {
        if (LL_USART_IsEnabledIT_PE(m_handle))
        {
          send_to_queue(m_interrupt_channel, PE);
        }
        LL_USART_ClearFlag_PE(m_handle);
      }
      else if (LL_USART_IsActiveFlag_NE(m_handle))
      {
        send_to_queue(m_interrupt_channel, NF);
        LL_USART_ClearFlag_NE(m_handle);
      }
    }

    if constexpr (Uart_Type::Interrupt == Tx_Type)
    {
      if (LL_USART_IsEnabledIT_TXE_TXFNF(m_handle) && LL_USART_IsActiveFlag_TXE_TXFNF(m_handle))
      {
        if (0 == m_interrupt_send_size)
        {
          LL_USART_DisableIT_TXE(m_handle);
          LL_USART_EnableIT_TC(m_handle);
        }
        else
        {
          m_interrupt_send_buffer++;
          m_interrupt_send_size--;

          if (0 != m_interrupt_send_size)
            LL_USART_TransmitData8(m_handle, *m_interrupt_send_buffer);
        }
      }
      else if (LL_USART_IsActiveFlag_TC(m_handle))
      {
        if (LL_USART_IsEnabledIT_TC(m_handle))
        {
          send_to_queue(m_interrupt_channel, TC);
        }
        LL_USART_DisableIT_TC(m_handle);
        LL_USART_ClearFlag_TC(m_handle);
      }
      else if (LL_USART_IsActiveFlag_ORE(m_handle))
      {
        send_to_queue(m_interrupt_channel, ORE);
        LL_USART_ClearFlag_ORE(m_handle);
      }
      else if (LL_USART_IsActiveFlag_FE(m_handle))
      {
        send_to_queue(m_interrupt_channel, FE);
        LL_USART_ClearFlag_FE(m_handle);
      }
    }
    else if constexpr (Uart_Type::DMA == Tx_Type)
    {
      if (LL_USART_IsActiveFlag_TC(m_handle))
      {
        if (LL_USART_IsEnabledIT_TC(m_handle))
        {
          LL_USART_DisableDMAReq_TX(m_handle);
          send_to_queue(m_interrupt_channel, TC);
        }
        LL_USART_DisableIT_TC(m_handle);
        LL_USART_ClearFlag_TC(m_handle);
      }
      else if (LL_USART_IsActiveFlag_ORE(m_handle))
      {
        send_to_queue(m_interrupt_channel, ORE);
        LL_USART_ClearFlag_ORE(m_handle);
      }
      else if (LL_USART_IsActiveFlag_FE(m_handle))
      {
        send_to_queue(m_interrupt_channel, FE);
        LL_USART_ClearFlag_FE(m_handle);
      }
    }
  }

  /**
   * @brief Uart 中断处理 异步函数
   */
  static void irq_queue_handle(interrupt::Interrupt_Args_t, uint8_t flag)
  {
    if constexpr (Uart_Type::Normal != Rx_Type)
    {
      if (IDLE == flag)
      {
        if (m_callback.received_complete_callback)
        {
          m_callback.received_complete_callback(m_callback.received_complete_arg);
        }
      }
      else if (PE == flag || NF == flag)
      {
        if (m_callback.error_callback)
        {
          m_callback.error_callback(m_callback.error_arg);
        }
      }
    }
    else if constexpr (Uart_Type::DMA == Rx_Type || Uart_Type::DMA_Double_Buffer == Rx_Type)
    {
      if (IDLE == flag)
      {
        if (m_callback.received_complete_callback)
        {
          m_callback.received_complete_callback(m_callback.received_complete_arg);
        }
      }
      else if (PE == flag || NF == flag)
      {
        if (m_callback.error_callback)
        {
          m_callback.error_callback(m_callback.error_arg);
        }
      }
    }

    if constexpr (Uart_Type::Interrupt == Tx_Type)
    {
      if (TC == flag)
      {
        if (m_callback.send_complete_callback)
        {
          m_callback.send_complete_callback(m_callback.send_complete_arg);
        }
      }
      else if (ORE == flag || FE == flag)
      {
        if (m_callback.error_callback)
        {
          m_callback.error_callback(m_callback.error_arg);
        }
      }
    }
    else if constexpr (Uart_Type::DMA == Tx_Type)
    {
      if (TC == flag)
      {
        if (m_callback.send_complete_callback)
        {
          m_callback.send_complete_callback(m_callback.send_complete_arg);
        }
      }
    }
  }

  /**
   * @brief  获取 Uart 引脚 复用通道
   *
   * @return Pin_Alternate 复用通道
   */
  static gpio::Pin_Alternate get_alternate(void)
  {
    if constexpr (1 == Port)
    {
      return gpio::Pin_Alternate::AF7;
    }
    else if constexpr (2 == Port)
    {
      return gpio::Pin_Alternate::AF7;
    }
    else if constexpr (3 == Port)
    {
      return gpio::Pin_Alternate::AF7;
    }
    else if constexpr (4 == Port)
    {
      return gpio::Pin_Alternate::AF8;
    }
    else if constexpr (5 == Port)
    {
      return gpio::Pin_Alternate::AF8;
    }
    else if constexpr (6 == Port)
    {
      return gpio::Pin_Alternate::AF7;
    }
    else if constexpr (7 == Port)
    {
      return gpio::Pin_Alternate::AF7;
    }
    else if constexpr (8 == Port)
    {
      return gpio::Pin_Alternate::AF8;
    }
    else
    {
      return gpio::Pin_Alternate::AF0;
    }
  }

  /**
   * @brief Uart 时钟使能
   */
  static void clk_enable(void)
  {
    if constexpr (1 == Port)
    {
      LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
    }
    else if constexpr (2 == Port)
    {
      LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
    }
    else if constexpr (3 == Port)
    {
      LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);
    }
    else if constexpr (4 == Port)
    {
      LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART4);
    }
    else if constexpr (5 == Port)
    {
      LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART5);
    }
    else if constexpr (6 == Port)
    {
      LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART6);
    }
    else if constexpr (7 == Port)
    {
      LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART7);
    }
    else if constexpr (8 == Port)
    {
      LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART8);
    }
  }

  /**
   * @brief Uart 时钟失能
   */
  static void clk_disable(void)
  {
    if constexpr (1 == Port)
    {
      LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_USART1);
    }
    else if constexpr (2 == Port)
    {
      LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_USART2);
    }
    else if constexpr (3 == Port)
    {
      LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_USART3);
    }
    else if constexpr (4 == Port)
    {
      LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_UART4);
    }
    else if constexpr (5 == Port)
    {
      LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_UART5);
    }
    else if constexpr (6 == Port)
    {
      LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_USART6);
    }
    else if constexpr (7 == Port)
    {
      LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_UART7);
    }
    else if constexpr (8 == Port)
    {
      LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_UART8);
    }
  }

  /**
   * @brief Uart 引脚初始化
   */
  static void gpio_init(void)
  {
    using Pin_Port  = gpio::Pin_Port;
    using Pin_Speed = gpio::Pin_Speed;
    using Pin_Mode  = gpio::Pin_Mode;

    if constexpr (1 == Port)
    {
      using Tx_Pin = gpio::Gpio<Pin_Port::PA, 9, Pin_Speed::High>;
      using Rx_Pin = gpio::Gpio<Pin_Port::PA, 10, Pin_Speed::High>;

      Tx_Pin::setup(get_alternate(), Pin_Mode::Alternate);
      Rx_Pin::setup(get_alternate(), Pin_Mode::Alternate_OD);
    }
    else if constexpr (2 == Port)
    {
      using Tx_Pin = gpio::Gpio<Pin_Port::PD, 5, Pin_Speed::High>;
      using Rx_Pin = gpio::Gpio<Pin_Port::PD, 6, Pin_Speed::High>;

      Tx_Pin::setup(get_alternate(), Pin_Mode::Alternate);
      Rx_Pin::setup(get_alternate(), Pin_Mode::Alternate_OD);
    }
    else if constexpr (3 == Port)
    {
      using Tx_Pin = gpio::Gpio<Pin_Port::PD, 8, Pin_Speed::High>;
      using Rx_Pin = gpio::Gpio<Pin_Port::PD, 9, Pin_Speed::High>;

      Tx_Pin::setup(get_alternate(), Pin_Mode::Alternate);
      Rx_Pin::setup(get_alternate(), Pin_Mode::Alternate_OD);
    }
    else if constexpr (4 == Port)
    {
      using Tx_Pin = gpio::Gpio<Pin_Port::PC, 10, Pin_Speed::High>;
      using Rx_Pin = gpio::Gpio<Pin_Port::PC, 11, Pin_Speed::High>;

      Tx_Pin::setup(get_alternate(), Pin_Mode::Alternate);
      Rx_Pin::setup(get_alternate(), Pin_Mode::Alternate_OD);
    }
    else if constexpr (5 == Port)
    {
      using Tx_Pin = gpio::Gpio<Pin_Port::PC, 12, Pin_Speed::High>;
      using Rx_Pin = gpio::Gpio<Pin_Port::PD, 2, Pin_Speed::High>;

      Tx_Pin::setup(get_alternate(), Pin_Mode::Alternate);
      Rx_Pin::setup(get_alternate(), Pin_Mode::Alternate_OD);
    }
    else if constexpr (6 == Port)
    {
      using Tx_Pin = gpio::Gpio<Pin_Port::PC, 6, Pin_Speed::High>;
      using Rx_Pin = gpio::Gpio<Pin_Port::PC, 7, Pin_Speed::High>;

      Tx_Pin::setup(get_alternate(), Pin_Mode::Alternate);
      Rx_Pin::setup(get_alternate(), Pin_Mode::Alternate_OD);
    }
    else if constexpr (7 == Port)
    {
      using Tx_Pin = gpio::Gpio<Pin_Port::PA, 15, Pin_Speed::High>;
      using Rx_Pin = gpio::Gpio<Pin_Port::PA, 8, Pin_Speed::High>;

      Tx_Pin::setup(get_alternate(), Pin_Mode::Alternate);
      Rx_Pin::setup(get_alternate(), Pin_Mode::Alternate_OD);
    }
    else if constexpr (8 == Port)
    {
      using Tx_Pin = gpio::Gpio<Pin_Port::PE, 1, Pin_Speed::High>;
      using Rx_Pin = gpio::Gpio<Pin_Port::PE, 0, Pin_Speed::High>;

      Tx_Pin::setup(get_alternate(), Pin_Mode::Alternate);
      Rx_Pin::setup(get_alternate(), Pin_Mode::Alternate_OD);
    }
  }

  /**
   * @brief Uart 引脚解除初始化
   */
  static void gpio_deinit(void)
  {
    using Pin_Port  = gpio::Pin_Port;
    using Pin_Speed = gpio::Pin_Speed;

    if constexpr (1 == Port)
    {
      using Tx_Pin = gpio::Gpio<Pin_Port::PA, 9, Pin_Speed::High>;
      using Rx_Pin = gpio::Gpio<Pin_Port::PA, 10, Pin_Speed::High>;

      Tx_Pin::clearup();
      Rx_Pin::clearup();
    }
    else if constexpr (2 == Port)
    {
      using Tx_Pin = gpio::Gpio<Pin_Port::PD, 5, Pin_Speed::High>;
      using Rx_Pin = gpio::Gpio<Pin_Port::PD, 6, Pin_Speed::High>;

      Tx_Pin::clearup();
      Rx_Pin::clearup();
    }
    else if constexpr (3 == Port)
    {
      using Tx_Pin = gpio::Gpio<Pin_Port::PD, 8, Pin_Speed::High>;
      using Rx_Pin = gpio::Gpio<Pin_Port::PD, 9, Pin_Speed::High>;

      Tx_Pin::clearup();
      Rx_Pin::clearup();
    }
    else if constexpr (4 == Port)
    {
      using Tx_Pin = gpio::Gpio<Pin_Port::PC, 10, Pin_Speed::High>;
      using Rx_Pin = gpio::Gpio<Pin_Port::PC, 11, Pin_Speed::High>;

      Tx_Pin::clearup();
      Rx_Pin::clearup();
    }
    else if constexpr (5 == Port)
    {
      using Tx_Pin = gpio::Gpio<Pin_Port::PC, 12, Pin_Speed::High>;
      using Rx_Pin = gpio::Gpio<Pin_Port::PD, 2, Pin_Speed::High>;

      Tx_Pin::clearup();
      Rx_Pin::clearup();
    }
    else if constexpr (6 == Port)
    {
      using Tx_Pin = gpio::Gpio<Pin_Port::PC, 6, Pin_Speed::High>;
      using Rx_Pin = gpio::Gpio<Pin_Port::PC, 7, Pin_Speed::High>;

      Tx_Pin::clearup();
      Rx_Pin::clearup();
    }
    else if constexpr (7 == Port)
    {
      using Tx_Pin = gpio::Gpio<Pin_Port::PA, 15, Pin_Speed::High>;
      using Rx_Pin = gpio::Gpio<Pin_Port::PA, 8, Pin_Speed::High>;

      Tx_Pin::clearup();
      Rx_Pin::clearup();
    }
    else if constexpr (8 == Port)
    {
      using Tx_Pin = gpio::Gpio<Pin_Port::PE, 1, Pin_Speed::High>;
      using Rx_Pin = gpio::Gpio<Pin_Port::PE, 0, Pin_Speed::High>;

      Tx_Pin::clearup();
      Rx_Pin::clearup();
    }
  }

  /**
   * @brief Uart 禁用
   */
  static void uart_disable(void)
  {
    if (LL_USART_IsEnabled(m_handle))
    {
      LL_USART_Disable(m_handle);

      if (LL_USART_DIRECTION_TX_RX == LL_USART_GetTransferDirection(m_handle))
      {
        while (LL_USART_IsActiveFlag_TEACK(m_handle) || LL_USART_IsActiveFlag_REACK(m_handle))
        {
        }
      }
      else if (LL_USART_DIRECTION_RX == LL_USART_GetTransferDirection(m_handle))
      {
        while (LL_USART_IsActiveFlag_REACK(m_handle))
        {
        }
      }
      else if (LL_USART_DIRECTION_TX == LL_USART_GetTransferDirection(m_handle))
      {
        while (LL_USART_IsActiveFlag_TEACK(m_handle))
        {
        }
      }

      if (LL_USART_DIRECTION_TX == LL_USART_GetTransferDirection(m_handle) || LL_USART_DIRECTION_TX_RX == LL_USART_GetTransferDirection(m_handle))
      {
        while (LL_USART_IsActiveFlag_TC(m_handle) == 0)
          ;

        if constexpr (Uart_Type::Interrupt == Tx_Type)
        {
          LL_USART_DisableIT_TXE(m_handle);
          LL_USART_DisableIT_TC(m_handle);
        }
        else if constexpr (Uart_Type::DMA == Tx_Type)
        {
          m_tx_dma.dma.stop();

          LL_USART_DisableIT_TC(m_handle);
          LL_USART_DisableDMAReq_TX(m_handle);
        }
      }

      if (LL_USART_DIRECTION_RX == LL_USART_GetTransferDirection(m_handle) || LL_USART_DIRECTION_TX_RX == LL_USART_GetTransferDirection(m_handle))
      {
        if constexpr (Uart_Type::Interrupt == Rx_Type)
        {
          LL_USART_DisableIT_RXNE_RXFNE(m_handle);
          LL_USART_DisableIT_IDLE(m_handle);
          LL_USART_DisableIT_ERROR(m_handle);
        }
        else if constexpr (Uart_Type::DMA == Rx_Type || Uart_Type::DMA_Double_Buffer == Rx_Type)
        {
          m_rx_dma.dma.stop();

          LL_USART_DisableIT_IDLE(m_handle);
          LL_USART_DisableIT_ERROR(m_handle);
          LL_USART_DisableDMAReq_RX(m_handle);
        }
      }
    }
  }

  /**
   * @brief Uart 使能
   */
  static void uart_enable(void)
  {
    if (!LL_USART_IsEnabled(m_handle))
    {
      LL_USART_Enable(m_handle);

      if (LL_USART_DIRECTION_TX_RX == LL_USART_GetTransferDirection(m_handle))
      {
        while ((!(LL_USART_IsActiveFlag_TEACK(m_handle))) || (!(LL_USART_IsActiveFlag_REACK(m_handle))))
        {
        }
      }
      else if (LL_USART_DIRECTION_RX == LL_USART_GetTransferDirection(m_handle))
      {
        while (!(LL_USART_IsActiveFlag_REACK(m_handle)))
        {
        }
      }
      else if (LL_USART_DIRECTION_TX == LL_USART_GetTransferDirection(m_handle))
      {
        while (!(LL_USART_IsActiveFlag_TEACK(m_handle)))
        {
        }
      }

      if (LL_USART_DIRECTION_RX == LL_USART_GetTransferDirection(m_handle) || LL_USART_DIRECTION_TX_RX == LL_USART_GetTransferDirection(m_handle))
      {
        if constexpr (Uart_Type::Interrupt == Rx_Type)
        {
          LL_USART_ClearFlag_IDLE(m_handle);
          LL_USART_ClearFlag_PE(m_handle);
          LL_USART_ClearFlag_NE(m_handle);

          LL_USART_EnableIT_RXNE_RXFNE(m_handle);
          LL_USART_EnableIT_IDLE(m_handle);
          LL_USART_EnableIT_ERROR(m_handle);
        }
        else if constexpr (Uart_Type::DMA == Rx_Type || Uart_Type::DMA_Double_Buffer == Rx_Type)
        {
          LL_USART_ClearFlag_IDLE(m_handle);
          LL_USART_ClearFlag_PE(m_handle);
          LL_USART_ClearFlag_NE(m_handle);

          m_rx_dma.dma.resume();

          LL_USART_EnableIT_IDLE(m_handle);
          LL_USART_EnableIT_ERROR(m_handle);
          LL_USART_EnableDMAReq_RX(m_handle);
        }
      }
    }
  }

public:
  /**
   * @brief  Uart 获取接收缓冲区工作模式
   *
   * @return Ring_Buffer_Mode 接收缓冲区工作模式
   */
  static constexpr system::memory::Ring_Buffer_Mode get_received_buffer_mode(void)
  {
    if constexpr (Rx_Type == Uart_Type::Interrupt)
      return system::memory::Ring_Buffer_Mode::INPUT_BYTES;
    else if constexpr (Rx_Type == Uart_Type::DMA)
      return system::memory::Ring_Buffer_Mode::INPUT_SINGLE_BUFFER;
    else if constexpr (Rx_Type == Uart_Type::DMA_Double_Buffer)
      return system::memory::Ring_Buffer_Mode::INPUT_DOUBLE_BUFFER;
  }

  /**
   * @brief  Uart 获取接收类型
   *
   * @return Uart_Type 接收类型
   */
  static constexpr Uart_Type received_type(void)
  {
    return Rx_Type;
  }

  /**
   * @brief  Uart 获取发送类型
   *
   * @return Uart_Type 发送类型
   */
  static constexpr Uart_Type send_type(void)
  {
    return Tx_Type;
  }

private:
  /**
   * @brief  Uart 读取接收数据寄存器
   *
   * @return uint8_t 接收数据
   */
  static const uint8_t read_rdr(void)
  {
    return LL_USART_ReceiveData8(m_handle);
  }

  /**
   * @brief  Uart 获取接收数据大小
   *
   * @return uint32_t 接收数据大小
   * @note   该函数仅在接收模式为DMA与DMA双缓冲模式时生效
   */
  template <bool enable = (Uart_Type::DMA == Rx_Type || Uart_Type::DMA_Double_Buffer == Rx_Type), typename = std::enable_if_t<enable>>
  static uint32_t receive_size(void)
  {
    return m_rx_dma.dma.get_transferred_size();
  }

  /**
   * @brief  Uart 字节接收回调函数
   *
   * @param  function 回调函数
   * @param  arg      回调函数参数
   */
  static void set_received_byte_callback(Uart_Func_t function, Uart_Args_t arg)
  {
    m_callback.received_byte_callback = function;
    m_callback.received_byte_arg      = arg;
  }

  /**
   * @brief  Uart 设置接收完成回调函数
   *
   * @param  function 回调函数
   * @param  arg      回调函数参数
   */
  static void set_received_complete_callback(Uart_Func_t function, Uart_Args_t arg)
  {
    m_callback.received_complete_callback = function;
    m_callback.received_complete_arg      = arg;
  }

  /**
   * @brief  Uart 设置开始发送回调函数
   *
   * @param  function 回调函数
   * @param  arg      回调函数参数
   */
  static void set_send_start_callback(Uart_Func_t function, Uart_Args_t arg)
  {
    m_callback.send_start_callback = function;
    m_callback.send_start_arg      = arg;
  }

  /**
   * @brief  Uart 设置发送完成回调函数
   *
   * @param  function 回调函数
   * @param  arg      回调函数参数
   */
  static void set_send_complete_callback(Uart_Func_t function, Uart_Args_t arg)
  {
    m_callback.send_complete_callback = function;
    m_callback.send_complete_arg      = arg;
  }

  /**
   * @brief  Uart 设置双缓冲内存切换回调函数
   *
   * @param  function 回调函数
   * @param  arg      回调函数参数
   */
  static void set_memory_switch_callback(Uart_Func_t function, Uart_Args_t arg)
  {
    m_callback.memory_switch_callback = function;
    m_callback.memory_switch_arg      = arg;
  }

  /**
   * @brief  Uart 设置错误回调函数
   *
   * @param  function 回调函数
   * @param  arg      回调函数参数
   */
  static void set_error_callback(Uart_Func_t function, Uart_Args_t arg)
  {
    m_callback.error_callback = function;
    m_callback.error_arg      = arg;
  }

  /**
   * @brief  Uart 设置波特率
   *
   * @param  baud_rate       波特率
   * @return Uart_Error_Code 错误码
   */
  static Uart_Error_Code set_baud_rate(uint32_t baud_rate)
  {
    uint32_t        periph_clk      = LL_RCC_PERIPH_FREQUENCY_NO;
    uint32_t        prescaler_value = LL_USART_PRESCALER_DIV1;
    Uart_Error_Code error_code      = Uart_Error_Code::OK;

    if constexpr (1 == Port || 6 == Port)
    {
      periph_clk = LL_RCC_GetUSARTClockFreq(LL_RCC_USART16_CLKSOURCE);
    }
    else if constexpr (2 == Port || 3 == Port || 4 == Port || 5 == Port || 7 == Port || 8 == Port)
    {
      periph_clk = LL_RCC_GetUSARTClockFreq(LL_RCC_USART234578_CLKSOURCE);
    }

    if (baud_rate >= 57600)
    {
      prescaler_value = LL_USART_PRESCALER_DIV1;
    }
    else if (baud_rate >= 38400 && baud_rate < 57600)
    {
      prescaler_value = LL_USART_PRESCALER_DIV2;
    }
    else if (baud_rate >= 19200 && baud_rate < 38400)
    {
      prescaler_value = LL_USART_PRESCALER_DIV4;
    }
    else if (baud_rate >= 9600 && baud_rate < 19200)
    {
      prescaler_value = LL_USART_PRESCALER_DIV8;
    }
    else if (baud_rate >= 4800 && baud_rate < 9600)
    {
      prescaler_value = LL_USART_PRESCALER_DIV16;
    }
    else if (baud_rate >= 1200 && baud_rate < 4800)
    {
      prescaler_value = LL_USART_PRESCALER_DIV32;
    }
    else
    {
      prescaler_value = LL_USART_PRESCALER_DIV64;
    }

    if (LL_RCC_PERIPH_FREQUENCY_NO != periph_clk && 0 != baud_rate)
    {
      uart_disable();
      LL_USART_SetPrescaler(m_handle, prescaler_value);
      LL_USART_SetBaudRate(m_handle, periph_clk, prescaler_value, LL_USART_OVERSAMPLING_16, baud_rate);
      uart_enable();

      m_prescaler_value = prescaler_value;
    }
    else
    {
      return Uart_Error_Code::INVALID_PARAMETER;
    }

    return error_code;
  }

  /**
   * @brief  Uart 设置数据位
   *
   * @param  data_bits       数据位
   * @return Uart_Error_Code 错误码
   */
  static Uart_Error_Code set_data_bits(uint8_t data_bits)
  {
    uint32_t        data_bits_value = 0;
    Uart_Error_Code error_code      = Uart_Error_Code::OK;

    if (7 == data_bits)
    {
      data_bits_value = LL_USART_DATAWIDTH_7B;
    }
    else if (8 == data_bits)
    {
      data_bits_value = LL_USART_DATAWIDTH_8B;
    }
    else if (9 == data_bits)
    {
      data_bits_value = LL_USART_DATAWIDTH_9B;
    }
    else
    {
      error_code = Uart_Error_Code::INVALID_PARAMETER;
    }

    if (error_code == Uart_Error_Code::OK)
    {
      uart_disable();
      LL_USART_SetDataWidth(m_handle, data_bits_value);
      uart_enable();
    }

    return error_code;
  }

  /**
   * @brief  Uart 设置停止位
   *
   * @param  stop_bits       停止位
   * @return Uart_Error_Code 错误码
   */
  static Uart_Error_Code set_stop_bits(uint8_t stop_bits)
  {
    uint32_t        stop_bits_value = 0;
    Uart_Error_Code error_code      = Uart_Error_Code::OK;

    if (1 == stop_bits)
    {
      stop_bits_value = LL_USART_STOPBITS_1;
    }
    else if (2 == stop_bits)
    {
      stop_bits_value = LL_USART_STOPBITS_2;
    }
    else
    {
      error_code = Uart_Error_Code::INVALID_PARAMETER;
    }

    if (Uart_Error_Code::OK == error_code)
    {
      uart_disable();
      LL_USART_SetStopBitsLength(m_handle, stop_bits_value);
      uart_enable();
    }

    return error_code;
  }

  /**
   * @brief  Uart 设置校验位
   *
   * @param  parity          校验位
   * @return Uart_Error_Code 错误码
   */
  static Uart_Error_Code set_parity(uint8_t parity)
  {
    uint32_t        parity_value = 0;
    Uart_Error_Code error_code   = Uart_Error_Code::OK;

    if (Uart_Parity::Even == parity)
    {
      parity_value = LL_USART_PARITY_EVEN;
    }
    else if (Uart_Parity::Odd == parity)
    {
      parity_value = LL_USART_PARITY_ODD;
    }
    else if (Uart_Parity::None == parity)
    {
      parity_value = LL_USART_PARITY_NONE;
    }
    else
    {
      error_code = Uart_Error_Code::INVALID_PARAMETER;
    }

    if (Uart_Error_Code::OK == error_code)
    {
      uart_disable();
      LL_USART_SetParity(m_handle, parity_value);
      uart_enable();
    }

    return error_code;
  }

  /**
   * @brief  Uart 设置中断优先级
   *
   * @param  priority        中断优先级
   * @return Uart_Error_Code 错误码
   */
  static Uart_Error_Code set_interrupt_priority(uint8_t priority)
  {
    interrupt::Interrupt_Manager::get_instance().set_interrupt_priority(m_interrupt_channel, priority);
    return Uart_Error_Code::OK;
  }

  /**
   * @brief  Uart 设置中断子优先级
   *
   * @param  sub_priority    中断子优先级
   * @return Uart_Error_Code 错误码
   */
  static Uart_Error_Code set_interrupt_sub_priority(uint8_t sub_priority)
  {
    interrupt::Interrupt_Manager::get_instance().set_interrupt_subpriority(m_interrupt_channel, sub_priority);
    return Uart_Error_Code::OK;
  }

  /**
   * @brief  Uart 获取波特率
   *
   * @return uint32_t 波特率
   */
  static const uint32_t get_baud_rate(void)
  {
    uint32_t periph_clk = LL_RCC_PERIPH_FREQUENCY_NO;

    if constexpr (1 == Port || 6 == Port)
    {
      periph_clk = LL_RCC_GetUSARTClockFreq(LL_RCC_USART16_CLKSOURCE);
    }
    else if constexpr (2 == Port || 3 == Port || 4 == Port || 5 == Port || 7 == Port || 8 == Port)
    {
      periph_clk = LL_RCC_GetUSARTClockFreq(LL_RCC_USART234578_CLKSOURCE);
    }

    return LL_USART_GetBaudRate(m_handle, periph_clk, m_prescaler_value, LL_USART_OVERSAMPLING_16);
  }

  /**
   * @brief  Uart 获取数据位
   *
   * @return uint32_t 数据位
   */
  static const uint32_t get_data_bits(void)
  {
    uint32_t bits = LL_USART_GetDataWidth(m_handle);
    return (LL_USART_DATAWIDTH_7B == bits) ? 7 : (LL_USART_DATAWIDTH_8B == bits) ? 8 : (LL_USART_DATAWIDTH_9B == bits) ? 9 : 0;
  }

  /**
   * @brief  Uart 获取停止位
   *
   * @return uint32_t 停止位
   */
  static const uint32_t get_stop_bits(void)
  {
    uint32_t bits = LL_USART_GetStopBitsLength(m_handle);
    return (LL_USART_STOPBITS_1 == bits) ? 1 : (LL_USART_STOPBITS_2 == bits) ? 2 : 0;
  }

  /**
   * @brief  Uart 获取校验位
   *
   * @return uint32_t 校验位
   */
  static const uint32_t get_parity(void)
  {
    uint32_t bits = LL_USART_GetParity(m_handle);
    return (LL_USART_PARITY_NONE == bits) ? Uart_Parity::None : (LL_USART_PARITY_EVEN == bits) ? Uart_Parity::Even : Uart_Parity::Odd;
  }

  /**
   * @brief  Uart 获取中断优先级
   *
   * @return uint32_t 中断优先级
   */
  static const uint32_t get_interrupt_priority(void)
  {
    return interrupt::Interrupt_Manager::get_instance().get_interrupt_priority(m_interrupt_channel);
  }

  /**
   * @brief  Uart 获取中断子优先级
   *
   * @return uint32_t 中断子优先级
   */
  static const uint32_t get_interrupt_sub_priority(void)
  {
    return interrupt::Interrupt_Manager::get_instance().get_interrupt_subpriority(m_interrupt_channel);
  }

  /**
   * @brief  Uart 获取端口号
   *
   * @return uint32_t 端口号
   */
  static constexpr uint32_t get_port_num(void)
  {
    return Port;
  }

  /**
   * @brief  Uart 初始化
   *
   * @param  baud_rate        波特率
   * @param  data_bits        数据位
   * @param  stop_bits        停止位
   * @param  parity           校验位
   * @param  type             流类型
   * @return Uart_Error_Code  错误码
   */
  static Uart_Error_Code init(uint32_t baud_rate, uint8_t data_bits, uint8_t stop_bits, uint8_t parity, system::device::Stream_Type type)
  {
    Uart_Error_Code      error_code = Uart_Error_Code::OK;
    LL_USART_InitTypeDef usart_init = { 0 };

    if (baud_rate >= 57600)
    {
      usart_init.PrescalerValue = LL_USART_PRESCALER_DIV1;
    }
    else if (baud_rate >= 38400 && baud_rate < 57600)
    {
      usart_init.PrescalerValue = LL_USART_PRESCALER_DIV2;
    }
    else if (baud_rate >= 19200 && baud_rate < 38400)
    {
      usart_init.PrescalerValue = LL_USART_PRESCALER_DIV4;
    }
    else if (baud_rate >= 9600 && baud_rate < 19200)
    {
      usart_init.PrescalerValue = LL_USART_PRESCALER_DIV8;
    }
    else if (baud_rate >= 4800 && baud_rate < 9600)
    {
      usart_init.PrescalerValue = LL_USART_PRESCALER_DIV16;
    }
    else if (baud_rate >= 1200 && baud_rate < 4800)
    {
      usart_init.PrescalerValue = LL_USART_PRESCALER_DIV32;
    }
    else
    {
      usart_init.PrescalerValue = LL_USART_PRESCALER_DIV64;
    }

    usart_init.BaudRate = baud_rate;

    if (7 == data_bits)
    {
      usart_init.DataWidth = LL_USART_DATAWIDTH_7B;
    }
    else if (8 == data_bits)
    {
      usart_init.DataWidth = LL_USART_DATAWIDTH_8B;
    }
    else if (9 == data_bits)
    {
      usart_init.DataWidth = LL_USART_DATAWIDTH_9B;
    }
    else
    {
      error_code = Uart_Error_Code::INVALID_PARAMETER;
    }

    if (1 == stop_bits)
    {
      usart_init.StopBits = LL_USART_STOPBITS_1;
    }
    else if (2 == stop_bits)
    {
      usart_init.StopBits = LL_USART_STOPBITS_2;
    }
    else
    {
      error_code = Uart_Error_Code::INVALID_PARAMETER;
    }

    if (Uart_Parity::None == parity)
    {
      usart_init.Parity = LL_USART_PARITY_NONE;
    }
    else if (Uart_Parity::Even == parity)
    {
      usart_init.Parity = LL_USART_PARITY_EVEN;
    }
    else if (Uart_Parity::Odd == parity)
    {
      usart_init.Parity = LL_USART_PARITY_ODD;
    }
    else
    {
      error_code = Uart_Error_Code::INVALID_PARAMETER;
    }

    if (system::device::Stream_Type::READ_WRITE == type)
    {
      usart_init.TransferDirection = LL_USART_DIRECTION_TX_RX;
    }
    else if (system::device::Stream_Type::READ_ONLY == type)
    {
      usart_init.TransferDirection = LL_USART_DIRECTION_RX;
    }
    else if (system::device::Stream_Type::WRITE_ONLY == type)
    {
      usart_init.TransferDirection = LL_USART_DIRECTION_TX;
    }

    usart_init.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    usart_init.OverSampling        = LL_USART_OVERSAMPLING_16;

    if (Uart_Error_Code::OK == error_code)
    {
      if (system::device::Stream_Type::READ_WRITE == type || system::device::Stream_Type::READ_ONLY == type)
      {
        if constexpr (Uart_Type::DMA == Rx_Type)
        {
          if (dma::Dma_Error_Code::OK != m_rx_dma.dma.template open<interrupt::Interrupt_Type::Queue>(base_internal::uart_internal::get_rx_dma_request<Port>(), dma_rx_callback, nullptr))
          {
            error_code = Uart_Error_Code::INIT_FAILED;
          }
        }
        else if constexpr (Uart_Type::DMA_Double_Buffer == Rx_Type)
        {
          if (dma::Dma_Error_Code::OK != m_rx_dma.dma.template open<interrupt::Interrupt_Type::Direct>(base_internal::uart_internal::get_rx_dma_request<Port>(), dma_rx_callback, nullptr))
          {
            error_code = Uart_Error_Code::INIT_FAILED;
          }
        }
      }

      if (system::device::Stream_Type::READ_WRITE == type || system::device::Stream_Type::WRITE_ONLY == type)
      {
        if constexpr (Uart_Type::DMA == Tx_Type)
        {
          if (dma::Dma_Error_Code::OK != m_tx_dma.dma.template open<interrupt::Interrupt_Type::Direct>(base_internal::uart_internal::get_tx_dma_request<Port>(), dma_tx_callback, nullptr))
          {
            error_code = Uart_Error_Code::INIT_FAILED;
          }
        }
      }

      if (Uart_Error_Code::OK == error_code)
      {
        clk_enable();
        gpio_init();

        if (ERROR == LL_USART_Init(m_handle, &usart_init))
        {
          error_code = Uart_Error_Code::INIT_FAILED;

          gpio_deinit();
          clk_disable();
        }
        else
        {
          register_device(m_interrupt_channel, irq_direct_handle, irq_queue_handle, nullptr, DEF_INTERRUPT_PRIORITY, DEF_INTERRUPT_SUBPRIORITY);

          LL_USART_DisableFIFO(m_handle);
          LL_USART_ConfigAsyncMode(m_handle);
          LL_USART_Enable(m_handle);

          if (LL_USART_DIRECTION_TX_RX == LL_USART_GetTransferDirection(m_handle))
          {
            while ((!(LL_USART_IsActiveFlag_TEACK(m_handle))) || (!(LL_USART_IsActiveFlag_REACK(m_handle))))
            {
            }
          }
          else if (LL_USART_DIRECTION_RX == LL_USART_GetTransferDirection(m_handle))
          {
            while (!(LL_USART_IsActiveFlag_REACK(m_handle)))
            {
            }
          }
          else if (LL_USART_DIRECTION_TX == LL_USART_GetTransferDirection(m_handle))
          {
            while (!(LL_USART_IsActiveFlag_TEACK(m_handle)))
            {
            }
          }

          m_prescaler_value = usart_init.PrescalerValue;
        }
      }
    }

    return error_code;
  }

  /**
   * @brief  Uart 使能接收
   *
   * @return Uart_Error_Code 错误码
   * @note   该函数仅在接收模式为中断模式时生效
   */
  template <bool enable = (Uart_Type::Interrupt == Rx_Type), typename = std::enable_if_t<enable>>
  static Uart_Error_Code enable_receive(void)
  {
    Uart_Error_Code error_code = Uart_Error_Code::OK;

    LL_USART_ClearFlag_IDLE(m_handle);
    LL_USART_ClearFlag_PE(m_handle);
    LL_USART_ClearFlag_NE(m_handle);

    LL_USART_EnableIT_RXNE_RXFNE(m_handle);
    LL_USART_EnableIT_IDLE(m_handle);
    LL_USART_EnableIT_ERROR(m_handle);

    return error_code;
  }

  /**
   * @brief  Uart 使能接收
   *
   * @param  memory_ptr       接收内存指针
   * @param  size             接收内存大小
   * @return Uart_Error_Code  错误码
   * @note   该函数仅在接收模式为DMA模式时生效
   */
  template <bool enable = (Uart_Type::DMA == Rx_Type), typename = std::enable_if_t<enable>>
  static Uart_Error_Code enable_receive(uint8_t* memory_ptr, uint32_t size)
  {
    Uart_Error_Code error_code = Uart_Error_Code::OK;

    LL_USART_ClearFlag_IDLE(m_handle);
    LL_USART_ClearFlag_PE(m_handle);
    LL_USART_ClearFlag_NE(m_handle);

    LL_USART_EnableIT_IDLE(m_handle);
    LL_USART_EnableIT_ERROR(m_handle);

    m_rx_dma.dma.start(LL_USART_DMA_GetRegAddr(m_handle, LL_USART_DMA_REG_DATA_RECEIVE), static_cast<uint32_t>(reinterpret_cast<uintptr_t>(memory_ptr)), size);

    LL_USART_EnableDMAReq_RX(m_handle);

    return error_code;
  }

  /**
   * @brief  Uart 使能接收
   *
   * @param  memory0_ptr      接收内存0指针
   * @param  memory1_ptr      接收内存1指针
   * @param  size             接收内存大小
   * @return Uart_Error_Code  错误码
   * @note   该函数仅在接收模式为DMA双缓冲模式时生效
   */
  template <bool enable = (Uart_Type::DMA_Double_Buffer == Rx_Type), typename = std::enable_if_t<enable>>
  static Uart_Error_Code enable_receive(uint8_t* memory0_ptr, uint8_t* memory1_ptr, uint32_t size)
  {
    Uart_Error_Code error_code = Uart_Error_Code::OK;

    LL_USART_ClearFlag_IDLE(m_handle);
    LL_USART_ClearFlag_PE(m_handle);
    LL_USART_ClearFlag_NE(m_handle);

    LL_USART_EnableIT_IDLE(m_handle);
    LL_USART_EnableIT_ERROR(m_handle);

    m_rx_dma.dma.start(LL_USART_DMA_GetRegAddr(m_handle, LL_USART_DMA_REG_DATA_RECEIVE), static_cast<uint32_t>(reinterpret_cast<uintptr_t>(memory0_ptr)), static_cast<uint32_t>(reinterpret_cast<uintptr_t>(memory1_ptr)), size);

    LL_USART_EnableDMAReq_RX(m_handle);

    return error_code;
  }

  /**
   * @brief  Uart 发送数据
   *
   * @param  data     发送数据指针
   * @param  size     发送数据大小
   * @return uint32_t 实际发送数据大小
   */
  static uint32_t send(const uint8_t* data, uint32_t size)
  {
    uint32_t sent_size = 0;

    if (m_callback.send_start_callback)
    {
      m_callback.send_start_callback(m_callback.send_start_arg);
    }

    if constexpr (Uart_Type::Normal == Tx_Type)
    {
      while (sent_size < size)
      {
        while (!(LL_USART_IsActiveFlag_TXE_TXFNF(m_handle)))
        {
        }

        LL_USART_TransmitData8(m_handle, *data);
        data++;
        sent_size++;
      }
    }
    else if constexpr (Uart_Type::Interrupt == Tx_Type)
    {
      if (0 == m_interrupt_send_size)
      {
        m_interrupt_send_buffer = data;
        m_interrupt_send_size   = size;

        LL_USART_TransmitData8(m_handle, *data);
        LL_USART_EnableIT_TXE(m_handle);
      }
    }
    else if constexpr (Uart_Type::DMA == Tx_Type)
    {
      m_tx_dma.dma.start(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(data)), LL_USART_DMA_GetRegAddr(m_handle, LL_USART_DMA_REG_DATA_TRANSMIT), size);

      LL_USART_EnableDMAReq_TX(m_handle);
    }

    return sent_size;
  }

  /**
   * @brief  Uart 解除初始化
   *
   * @return Uart_Error_Code 错误码
   */
  static Uart_Error_Code deinit(void)
  {
    Uart_Error_Code error_code = Uart_Error_Code::OK;

    LL_USART_Disable(m_handle);

    while (LL_USART_IsEnabled(m_handle))
    {
    }

    if constexpr (Uart_Type::Interrupt == Rx_Type)
    {
      LL_USART_DisableIT_RXNE_RXFNE(m_handle);
      LL_USART_DisableIT_IDLE(m_handle);
      LL_USART_DisableIT_ERROR(m_handle);
    }
    else if constexpr (Uart_Type::DMA == Rx_Type || Uart_Type::DMA_Double_Buffer == Rx_Type)
    {
      m_rx_dma.dma.close();

      LL_USART_DisableDMAReq_RX(m_handle);
      LL_USART_DisableIT_IDLE(m_handle);
      LL_USART_DisableIT_ERROR(m_handle);
    }

    if constexpr (Uart_Type::Interrupt == Tx_Type)
    {
      LL_USART_DisableIT_TXE(m_handle);
      LL_USART_DisableIT_TC(m_handle);
    }
    else if constexpr (Uart_Type::DMA == Tx_Type)
    {
      m_tx_dma.dma.close();

      LL_USART_DisableDMAReq_TX(m_handle);
      LL_USART_DisableIT_TC(m_handle);
    }

    unregister_device(m_interrupt_channel);

    if (ERROR == LL_USART_DeInit(m_handle))
    {
      error_code = Uart_Error_Code::DEINIT_FAILED;
    }

    gpio_deinit();
    clk_disable();

    return error_code;
  }
};
} /* namespace uart */
} /* namespace base */
} /* namespace QAQ */

/* ----- 注册中断函数 ----- */
INTERRUPT_HANDLER(USART1)
INTERRUPT_HANDLER(USART2)
INTERRUPT_HANDLER(USART3)
INTERRUPT_HANDLER(UART4)
INTERRUPT_HANDLER(UART5)
INTERRUPT_HANDLER(USART6)
INTERRUPT_HANDLER(UART7)
INTERRUPT_HANDLER(UART8)

#endif /* __UART_CONFIG_HPP__ */
