#ifndef __SPI_CONFIG_HPP__
#define __SPI_CONFIG_HPP__

#include "stm32h7xx_ll_spi.h"
#include "interrupt.hpp"
#include "gpio.hpp"
#include "dma.hpp"
#include "spi_base.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 基本对象
namespace base
{
/// @brief 命名空间 内部
namespace base_internal
{
/// @brief 命名空间 SPI 内部
namespace spi_internal
{
template <uint8_t Port>
class Spi_Base_Interface
{
public:
  static constexpr SPI_TypeDef* get_handle(void)
  {
    static constexpr SPI_TypeDef* spi_handle[] = { SPI1, SPI2, SPI3, SPI4, SPI5, SPI6 };

    return spi_handle[Port];
  }

  static constexpr interrupt::Interrupt_Channel_t get_interrupt_channel(void)
  {
    static constexpr IRQn_Type irqn[] = { SPI1_IRQn, SPI2_IRQn, SPI3_IRQn, SPI4_IRQn, SPI5_IRQn, SPI6_IRQn };

    return irqn[Port];
  }

  static constexpr uint32_t get_rx_dma_request(void)
  {
    static constexpr uint32_t dma_request[] = { LL_DMAMUX1_REQ_SPI1_RX, LL_DMAMUX1_REQ_SPI2_RX, LL_DMAMUX1_REQ_SPI3_RX, LL_DMAMUX1_REQ_SPI4_RX, LL_DMAMUX1_REQ_SPI5_RX, LL_DMAMUX2_REQ_SPI6_RX };

    return dma_request[Port];
  }

  static constexpr uint32_t get_tx_dma_request(void)
  {
    static constexpr uint32_t dma_request[] = { LL_DMAMUX1_REQ_SPI1_TX, LL_DMAMUX1_REQ_SPI2_TX, LL_DMAMUX1_REQ_SPI3_TX, LL_DMAMUX1_REQ_SPI4_TX, LL_DMAMUX1_REQ_SPI5_TX, LL_DMAMUX2_REQ_SPI6_TX };

    return dma_request[Port];
  }

  static uint32_t get_baud_rate_prescaler(uint32_t baud_rate_prescaler)
  {
    if (2 == baud_rate_prescaler)
    {
      return LL_SPI_BAUDRATEPRESCALER_DIV2;
    }
    else if (4 == baud_rate_prescaler)
    {
      return LL_SPI_BAUDRATEPRESCALER_DIV4;
    }
    else if (8 == baud_rate_prescaler)
    {
      return LL_SPI_BAUDRATEPRESCALER_DIV8;
    }
    else if (16 == baud_rate_prescaler)
    {
      return LL_SPI_BAUDRATEPRESCALER_DIV16;
    }
    else if (32 == baud_rate_prescaler)
    {
      return LL_SPI_BAUDRATEPRESCALER_DIV32;
    }
    else if (64 == baud_rate_prescaler)
    {
      return LL_SPI_BAUDRATEPRESCALER_DIV64;
    }
    else if (128 == baud_rate_prescaler)
    {
      return LL_SPI_BAUDRATEPRESCALER_DIV128;
    }
    else if (256 == baud_rate_prescaler)
    {
      return LL_SPI_BAUDRATEPRESCALER_DIV256;
    }
    else
    {
      return LL_SPI_BAUDRATEPRESCALER_DIV4;
    }
  }

  static uint32_t get_data_size(uint32_t data_size)
  {
    if constexpr (1 == Port || 2 == Port || 3 == Port)
    {
      static constexpr uint32_t data_size_table[] = { LL_SPI_DATAWIDTH_4BIT, LL_SPI_DATAWIDTH_5BIT, LL_SPI_DATAWIDTH_6BIT, LL_SPI_DATAWIDTH_7BIT, LL_SPI_DATAWIDTH_8BIT, LL_SPI_DATAWIDTH_9BIT, LL_SPI_DATAWIDTH_10BIT, LL_SPI_DATAWIDTH_11BIT, LL_SPI_DATAWIDTH_12BIT, LL_SPI_DATAWIDTH_13BIT, LL_SPI_DATAWIDTH_14BIT, LL_SPI_DATAWIDTH_15BIT, LL_SPI_DATAWIDTH_16BIT, LL_SPI_DATAWIDTH_17BIT, LL_SPI_DATAWIDTH_18BIT, LL_SPI_DATAWIDTH_19BIT, LL_SPI_DATAWIDTH_20BIT, LL_SPI_DATAWIDTH_21BIT, LL_SPI_DATAWIDTH_22BIT, LL_SPI_DATAWIDTH_23BIT, LL_SPI_DATAWIDTH_24BIT, LL_SPI_DATAWIDTH_25BIT, LL_SPI_DATAWIDTH_26BIT, LL_SPI_DATAWIDTH_27BIT, LL_SPI_DATAWIDTH_28BIT, LL_SPI_DATAWIDTH_29BIT, LL_SPI_DATAWIDTH_30BIT, LL_SPI_DATAWIDTH_31BIT, LL_SPI_DATAWIDTH_32BIT };

      if (data_size < 4 || data_size > 32)
      {
        return LL_SPI_DATAWIDTH_8BIT;
      }
      else
      {
        return data_size_table[data_size - 4];
      }
    }
    else
    {
      static constexpr uint32_t data_size_table[] = { LL_SPI_DATAWIDTH_4BIT, LL_SPI_DATAWIDTH_5BIT, LL_SPI_DATAWIDTH_6BIT, LL_SPI_DATAWIDTH_7BIT, LL_SPI_DATAWIDTH_8BIT, LL_SPI_DATAWIDTH_9BIT, LL_SPI_DATAWIDTH_10BIT, LL_SPI_DATAWIDTH_11BIT, LL_SPI_DATAWIDTH_12BIT, LL_SPI_DATAWIDTH_13BIT, LL_SPI_DATAWIDTH_14BIT, LL_SPI_DATAWIDTH_15BIT, LL_SPI_DATAWIDTH_16BIT };

      if (data_size < 4 || data_size > 16)
      {
        return LL_SPI_DATAWIDTH_8BIT;
      }
      else
      {
        return data_size_table[data_size - 4];
      }
    }
  }

  static void enable_clk(void)
  {
    if constexpr (1 == Port)
    {
      LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
    }
    else if constexpr (2 == Port)
    {
      LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
    }
    else if constexpr (3 == Port)
    {
      LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI3);
    }
    else if constexpr (4 == Port)
    {
      LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI4);
    }
    else if constexpr (5 == Port)
    {
      LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI5);
    }
    else if constexpr (6 == Port)
    {
      LL_APB4_GRP1_EnableClock(LL_APB4_GRP1_PERIPH_SPI6);
    }
  }

  static void disable_clk(void)
  {
    if constexpr (1 == Port)
    {
      LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_SPI1);
    }
    else if constexpr (2 == Port)
    {
      LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_SPI2);
    }
    else if constexpr (3 == Port)
    {
      LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_SPI3);
    }
    else if constexpr (4 == Port)
    {
      LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_SPI4);
    }
    else if constexpr (5 == Port)
    {
      LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_SPI5);
    }
    else if constexpr (6 == Port)
    {
      LL_APB4_GRP1_DisableClock(LL_APB4_GRP1_PERIPH_SPI6);
    }
  }

  static void gpio_init(void)
  {
    using Pin_Port                         = gpio::Pin_Port;
    using Pin_Speed                        = gpio::Pin_Speed;
    using Pin_Mode                         = gpio::Pin_Mode;
    using Alternate                        = gpio::Pin_Alternate;

    static constexpr Alternate alternate[] = { Alternate::AF5, Alternate::AF5, Alternate::AF5, Alternate::AF5, Alternate::AF5, Alternate::AF5 };

    if constexpr (1 == Port)
    {
      using sclk_pin = gpio::Gpio<Pin_Port::PA, 5, Pin_Speed::High>;
      using miso_pin = gpio::Gpio<Pin_Port::PA, 6, Pin_Speed::High>;
      using mosi_pin = gpio::Gpio<Pin_Port::PB, 5, Pin_Speed::High>;

      sclk_pin::init(alternate[Port], Pin_Mode::Alternate);
      miso_pin::init(alternate[Port], Pin_Mode::Alternate);
      mosi_pin::init(alternate[Port], Pin_Mode::Alternate);
    }
    else if constexpr (2 == Port)
    {
      using sclk_pin = gpio::Gpio<Pin_Port::PB, 13, Pin_Speed::High>;
      using miso_pin = gpio::Gpio<Pin_Port::PB, 14, Pin_Speed::High>;
      using mosi_pin = gpio::Gpio<Pin_Port::PB, 15, Pin_Speed::High>;

      sclk_pin::init(alternate[Port], Pin_Mode::Alternate);
      miso_pin::init(alternate[Port], Pin_Mode::Alternate);
      mosi_pin::init(alternate[Port], Pin_Mode::Alternate);
    }
    else if constexpr (3 == Port)
    {
      using sclk_pin = gpio::Gpio<Pin_Port::PB, 3, Pin_Speed::High>;
      using miso_pin = gpio::Gpio<Pin_Port::PB, 4, Pin_Speed::High>;
      using mosi_pin = gpio::Gpio<Pin_Port::PB, 5, Pin_Speed::High>;

      sclk_pin::init(alternate[Port], Pin_Mode::Alternate);
      miso_pin::init(alternate[Port], Pin_Mode::Alternate);
      mosi_pin::init(alternate[Port], Pin_Mode::Alternate);
    }
    else if constexpr (4 == Port)
    {
      using sclk_pin = gpio::Gpio<Pin_Port::PE, 2, Pin_Speed::High>;
      using miso_pin = gpio::Gpio<Pin_Port::PE, 5, Pin_Speed::High>;
      using mosi_pin = gpio::Gpio<Pin_Port::PE, 6, Pin_Speed::High>;

      sclk_pin::init(alternate[Port], Pin_Mode::Alternate);
      miso_pin::init(alternate[Port], Pin_Mode::Alternate);
      mosi_pin::init(alternate[Port], Pin_Mode::Alternate);
    }
    else if constexpr (5 == Port)
    {
      using sclk_pin = gpio::Gpio<Pin_Port::PF, 7, Pin_Speed::High>;
      using miso_pin = gpio::Gpio<Pin_Port::PF, 8, Pin_Speed::High>;
      using mosi_pin = gpio::Gpio<Pin_Port::PF, 9, Pin_Speed::High>;

      sclk_pin::init(alternate[Port], Pin_Mode::Alternate);
      miso_pin::init(alternate[Port], Pin_Mode::Alternate);
      mosi_pin::init(alternate[Port], Pin_Mode::Alternate);
    }
    else if constexpr (6 == Port)
    {
      using sclk_pin = gpio::Gpio<Pin_Port::PG, 12, Pin_Speed::High>;
      using miso_pin = gpio::Gpio<Pin_Port::PG, 13, Pin_Speed::High>;
      using mosi_pin = gpio::Gpio<Pin_Port::PG, 14, Pin_Speed::High>;

      sclk_pin::init(alternate[Port], Pin_Mode::Alternate);
      miso_pin::init(alternate[Port], Pin_Mode::Alternate);
      mosi_pin::init(alternate[Port], Pin_Mode::Alternate);
    }
  }

  static void gpio_deinit(void)
  {
    using Pin_Port  = gpio::Pin_Port;
    using Pin_Mode  = gpio::Pin_Mode;
    using Pin_Speed = gpio::Pin_Speed;

    if constexpr (1 == Port)
    {
      using sclk_pin = gpio::Gpio<Pin_Port::PA, 5, Pin_Speed::High>;
      using miso_pin = gpio::Gpio<Pin_Port::PA, 6, Pin_Speed::High>;
      using mosi_pin = gpio::Gpio<Pin_Port::PB, 5, Pin_Speed::High>;

      sclk_pin::clearup();
      miso_pin::clearup();
      mosi_pin::clearup();
    }
    else if constexpr (2 == Port)
    {
      using sclk_pin = gpio::Gpio<Pin_Port::PB, 13, Pin_Speed::High>;
      using miso_pin = gpio::Gpio<Pin_Port::PB, 14, Pin_Speed::High>;
      using mosi_pin = gpio::Gpio<Pin_Port::PB, 15, Pin_Speed::High>;

      sclk_pin::clearup();
      miso_pin::clearup();
      mosi_pin::clearup();
    }
    else if constexpr (3 == Port)
    {
      using sclk_pin = gpio::Gpio<Pin_Port::PB, 3, Pin_Speed::High>;
      using miso_pin = gpio::Gpio<Pin_Port::PB, 4, Pin_Speed::High>;
      using mosi_pin = gpio::Gpio<Pin_Port::PB, 5, Pin_Speed::High>;

      sclk_pin::clearup();
      miso_pin::clearup();
      mosi_pin::clearup();
    }
    else if constexpr (4 == Port)
    {
      using sclk_pin = gpio::Gpio<Pin_Port::PE, 2, Pin_Speed::High>;
      using miso_pin = gpio::Gpio<Pin_Port::PE, 5, Pin_Speed::High>;
      using mosi_pin = gpio::Gpio<Pin_Port::PE, 6, Pin_Speed::High>;

      sclk_pin::clearup();
      miso_pin::clearup();
      mosi_pin::clearup();
    }
    else if constexpr (5 == Port)
    {
      using sclk_pin = gpio::Gpio<Pin_Port::PF, 7, Pin_Speed::High>;
      using miso_pin = gpio::Gpio<Pin_Port::PF, 8, Pin_Speed::High>;
      using mosi_pin = gpio::Gpio<Pin_Port::PF, 9, Pin_Speed::High>;

      sclk_pin::clearup();
      miso_pin::clearup();
      mosi_pin::clearup();
    }
    else if constexpr (6 == Port)
    {
      using sclk_pin = gpio::Gpio<Pin_Port::PG, 12, Pin_Speed::High>;
      using miso_pin = gpio::Gpio<Pin_Port::PG, 13, Pin_Speed::High>;
      using mosi_pin = gpio::Gpio<Pin_Port::PG, 14, Pin_Speed::High>;

      sclk_pin::clearup();
      miso_pin::clearup();
      mosi_pin::clearup();
    }
  }
};

template <spi::Spi_Type Type>
class Spi_Rx_Dma;

template <>
class Spi_Rx_Dma<spi::Spi_Type::DMA> : public dma::Dma<dma::Dma_Config<dma::Dma_Direction::Peripheral_To_Memory, dma::Dma_Mode::Normal, dma::Dma_Priority::Medium, false, true, dma::Dma_Data_Size::Byte, dma::Dma_Data_Size::Byte>>
{
};

template <spi::Spi_Type Type>
class Spi_Rx_Dma
{
};

template <spi::Spi_Type Type>
class Spi_Tx_Dma;

template <>
class Spi_Tx_Dma<spi::Spi_Type::DMA> : public dma::Dma<dma::Dma_Config<dma::Dma_Direction::Memory_To_Peripheral, dma::Dma_Mode::Normal, dma::Dma_Priority::Medium, true, false, dma::Dma_Data_Size::Byte, dma::Dma_Data_Size::Byte>>
{
};

template <spi::Spi_Type Type>
class Spi_Tx_Dma
{
};

template <typename Config>
class Spi_Base;
} /* namespace spi_internal */
} /* namespace base_internal */

namespace spi
{
template <uint8_t Port, Spi_Type Rx_Type, Spi_Type Tx_Type>
class Spi_Config : public interrupt::Interrupt_Device
{
  /// @warning 端口合法性判断
  static_assert(1 <= Port && 6 >= Port, "Invalid port number");

  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(Spi_Config)

private:
  template <typename Config>
  friend class base_internal::spi_internal::Spi_Base;

  using Spi_Func_t     = system::system_internal::device_internal::Device_Func_t;

  using Spi_Args_t     = system::system_internal::device_internal::Device_Args_t;

  using Spi_Error_Code = system::device::Device_Error_Code;

  struct Callback_t
  {
    Spi_Func_t received_complete_callback;
    Spi_Args_t received_complete_args;
    Spi_Func_t transmitted_complete_callback;
    Spi_Args_t transmitted_complete_args;
    Spi_Func_t error_callback;
    Spi_Args_t error_args;
  };

  static inline Callback_t m_callback                              = { 0 };

  static inline mutable SPI_TypeDef* m_spi                         = base_internal::spi_internal::Spi_Base_Interface<Port>::get_handle();

  static inline interrupt::Interrupt_Channel_t m_interrupt_channel = base_internal::spi_internal::Spi_Base_Interface<Port>::get_interrupt_channel();

  static inline base_internal::spi_internal::Spi_Rx_Dma<Rx_Type> m_rx_dma;
  static inline base_internal::spi_internal::Spi_Tx_Dma<Tx_Type> m_tx_dma;

  static inline const volatile uint8_t* m_interrupt_transmitted_buffer = nullptr;
  static inline volatile uint8_t        m_interrupt_transmitted_size   = 0;
  static inline volatile uint8_t*       m_interrupt_received_buffer    = nullptr;
  static inline volatile uint8_t        m_interrupt_received_size      = 0;

  static inline uint8_t m_data_size                                    = 8;

  static constexpr uint32_t TXC                                        = 0x01;
  static constexpr uint32_t RXC                                        = 0x02;
  static constexpr uint32_t OVR                                        = 0x04;
  static constexpr uint32_t UDR                                        = 0x08;
  static constexpr uint32_t MODF                                       = 0x10;
  static constexpr uint32_t SUSP                                       = 0x20;

  static void dma_rx_callback(dma::Dma_Callback_Args_t)
  {
    if (m_callback.received_complete_callback)
    {
      m_callback.received_complete_callback(m_callback.received_complete_callback_args);
    }
  }

  static void dma_tx_callback(dma::Dma_Callback_Args_t)
  {
    if (m_callback.transmit_complete_callback)
    {
      m_callback.transmit_complete_callback(m_callback.transmit_complete_callback_args);
    }
  }

  static void rx_direct_handle(void)
  {
    if (m_data_size <= 8)
    {
      *m_interrupt_received_buffer = LL_SPI_ReceiveData8(m_spi);
      m_interrupt_received_buffer  = m_interrupt_received_buffer + sizeof(uint8_t);
      m_interrupt_received_size    = m_interrupt_received_size - sizeof(uint8_t);
    }
    else if (m_data_size <= 16)
    {
      *(static_cast<uint16_t*>(m_interrupt_received_buffer)) = LL_SPI_ReceiveData16(m_spi);
      m_interrupt_received_buffer                            = m_interrupt_received_buffer + sizeof(uint16_t);
      m_interrupt_received_size                              = m_interrupt_received_size - sizeof(uint16_t);
    }
    else
    {
      *(static_cast<uint32_t*>(m_interrupt_received_buffer)) = LL_SPI_ReceiveData32(m_spi);
      m_interrupt_received_buffer                            = m_interrupt_received_buffer + sizeof(uint32_t);
      m_interrupt_received_size                              = m_interrupt_received_size - sizeof(uint32_t);
    }

    if (0 == m_interrupt_received_size)
    {
      LL_SPI_DisableIT_RXP(m_spi);
    }
  }

  static void tx_direct_handle(void)
  {
    if (m_data_size <= 8)
    {
      LL_SPI_TransmitData8(m_spi, *m_interrupt_transmit_buffer);
      m_interrupt_transmit_buffer = m_interrupt_transmit_buffer + sizeof(uint8_t);
      m_interrupt_transmit_size   = m_interrupt_transmit_size - sizeof(uint8_t);
    }
    else if (m_data_size <= 16)
    {
      LL_SPI_TransmitData16(m_spi, *(static_cast<uint16_t*>(m_interrupt_transmit_buffer)));
      m_interrupt_transmit_buffer = m_interrupt_transmit_buffer + sizeof(uint16_t);
      m_interrupt_transmit_size   = m_interrupt_transmit_size - sizeof(uint16_t);
    }
    else
    {
      LL_SPI_TransmitData32(m_spi, *(static_cast<uint32_t*>(m_interrupt_transmit_buffer)));
      m_interrupt_transmit_buffer = m_interrupt_transmit_buffer + sizeof(uint32_t);
      m_interrupt_transmit_size   = m_interrupt_transmit_size - sizeof(uint32_t);
    }

    if (0 == m_interrupt_transmit_size)
    {
      LL_SPI_DisableIT_TXP(m_spi);
    }
  }

  static void irq_direct_handle(interrupt::Interrupt_Args_t, uint8_t)
  {
    if (LL_SPI_IsActiveFlag_SUSP(m_spi) && LL_SPI_IsEnabledIT_EOT(m_spi))
    {
      send_to_queue(m_interrupt_channel, SUSP);
      LL_SPI_ClearFlag_SUSP(m_spi);
      return;
    }

    if (!LL_SPI_IsActiveFlag_MODF(m_spi) && !LL_SPI_IsActiveFlag_OVR(m_spi) && !LL_SPI_IsActiveFlag_UDR(m_spi))
    {
      if (LL_SPI_IsEnabledIT_DXP(m_spi) && LL_SPI_IsActiveFlag_DXP(m_spi))
      {
        rx_direct_handle();
        tx_direct_handle();
        return;
      }
      else
      {
        if (LL_SPI_IsEnabledIT_RXP(m_spi) && LL_SPI_IsActiveFlag_RXP(m_spi))
        {
          rx_direct_handle();
          return;
        }
        else if (LL_SPI_IsEnabledIT_TXP(m_spi) && LL_SPI_IsActiveFlag_TXP(m_spi))
        {
          tx_direct_handle();
          return;
        }
      }

      if (LL_SPI_IsActiveFlag_EOT(m_spi))
      {
      }
    }
    else
    {
      static uint32_t flag;
      flag = 0;

      if (LL_SPI_IsActiveFlag_MODF(m_spi))
      {
        if (LL_SPI_IsEnabledIT_MODF(m_spi))
        {
          flag |= MODF;
        }
        LL_SPI_ClearFlag_MODF(m_spi);
      }

      if (LL_SPI_IsActiveFlag_OVR(m_spi))
      {
        if (LL_SPI_IsEnabledIT_OVR(m_spi))
        {
          flag |= OVR;
        }
        LL_SPI_ClearFlag_OVR(m_spi);
      }

      if (LL_SPI_IsActiveFlag_UDR(m_spi))
      {
        if (LL_SPI_IsEnabledIT_UDR(m_spi))
        {
          flag |= UDR;
        }
        LL_SPI_ClearFlag_UDR(m_spi);
      }

      if (0 != flag)
      {
        LL_SPI_Disable(m_spi);

        LL_SPI_DisableIT_RXP(m_spi);
        LL_SPI_DisableIT_TXP(m_spi);
        LL_SPI_DisableIT_EOT(m_spi);
        LL_SPI_DisableIT_OVR(m_spi);
        LL_SPI_DisableIT_UDR(m_spi);
        LL_SPI_DisableIT_MODF(m_spi);

        if constexpr (Spi_Type::DMA == Rx_Type)
        {
          if (LL_SPI_IsEnabledDMAReq_RX(m_spi))
          {
            LL_SPI_DisableDMAReq_RX(m_spi);

            m_rx_dma.stop();
          }
        }

        if constexpr (Spi_Type::DMA == Tx_Type)
        {
          if (LL_SPI_IsEnabledDMAReq_TX(m_spi))
          {
            LL_SPI_DisableDMAReq_TX(m_spi);

            m_tx_dma.stop();
          }
        }

        send_to_queue(m_interrupt_channel, flag);
      }
    }

    if constexpr (Spi_Type::Interrupt == Rx_Type)
    {
    }
    else if constexpr (Spi_Type::DMA == Rx_Type)
    {
    }

    if constexpr (Spi_Type::Interrupt == Tx_Type)
    {
    }
    else if constexpr (Spi_Type::DMA == Tx_Type)
    {
    }
  }

  static void irq_queue_handle(interrupt::Interrupt_Args_t, uint8_t flag)
  {
    if constexpr (Spi_Type::Interrupt == Rx_Type)
    {
    }
    else if constexpr (Spi_Type::DMA == Rx_Type)
    {
    }

    if constexpr (Spi_Type::Interrupt == Tx_Type)
    {
    }
    else if constexpr (Spi_Type::DMA == Tx_Type)
    {
    }
  }

  void spi_enable(void) {}

  void spi_disable(void) {}

public:
  static constexpr Spi_Type received_type(void)
  {
    return Rx_Type;
  }

  static constexpr Spi_Type transmitted_type(void)
  {
    return Tx_Type;
  }

private:
  static void set_received_complete_callback(Spi_Func_t function, Spi_Args_t arg)
  {
    m_callback.received_complete_callback = function;
    m_callback.received_complete_arg      = arg;
  }

  static void set_transmitted_complete_callback(Spi_Func_t function, Spi_Args_t arg)
  {
    m_callback.transmitted_complete_callback = function;
    m_callback.transmitted_complete_arg      = arg;
  }

  static void set_error_callback(Spi_Func_t function, Spi_Args_t arg)
  {
    m_callback.error_callback = function;
    m_callback.error_arg      = arg;
  }

  static Spi_Error_Code set_mode(uint32_t mode) {}

  static Spi_Error_Code set_endian(uint32_t endian) {}

  static Spi_Error_Code set_data_size(uint32_t data_size) {}

  static Spi_Error_Code set_baud_rate_prescaler(uint32_t baud_rate_prescaler) {}

  static Spi_Error_Code set_clock_polarity(uint32_t clock_polarity) {}

  static Spi_Error_Code set_clock_phase(uint32_t clock_phase) {}

  static Spi_Error_Code set_interrupt_priority(uint8_t priority) {}

  static Spi_Error_Code set_interrupt_sub_priority(uint8_t sub_priority) {}

  static uint32_t get_mode(void) const {}

  static uint32_t get_endian(void) const {}

  static uint32_t get_data_size(void) const {}

  static uint32_t get_baud_rate_prescaler(void) const {}

  static uint32_t get_clock_polarity(void) const {}

  static uint32_t get_clock_phase(void) const {}

  static uint32_t get_interrupt_priority(void) const {}

  static uint32_t get_interrupt_sub_priority(void) const {}

  static constexpr uint32_t get_port_num(void) const
  {
    return Port;
  }

  static Spi_Error_Code init(uint32_t mode, uint32_t endian, uint32_t data_size, uint32_t baud_rate_prescaler, uint32_t clock_polarity, uint32_t clock_phase, uint8_t priority, uint8_t sub_priority) {}

  static uint32_t receive(uint8_t* data, uint32_t size) {}

  static uint32_t transmit(const uint8_t* data, uint32_t size) {}

  template <bool enable = (Spi_Type::Normal != Rx_Type), typename = std::enable_if_t<enable>>
  static Spi_Error_Code receive_timeout_cleanup(void)
  {
  }

  static Spi_Error_Code deinit(void) {}
};
} /* namespace spi */
} /* namespace base */
} /* namespace QAQ */

INTERRUPT_HANDLER(SPI1)
INTERRUPT_HANDLER(SPI2)
INTERRUPT_HANDLER(SPI3)
INTERRUPT_HANDLER(SPI4)
INTERRUPT_HANDLER(SPI5)
INTERRUPT_HANDLER(SPI6)

#endif /* __SPI_CONFIG_HPP__ */
