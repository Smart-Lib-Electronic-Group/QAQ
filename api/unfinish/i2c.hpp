#ifndef __I2C_HPP__
#define __I2C_HPP__

#include "gpio.hpp"

namespace QAQ::I2C
{

using speed_t = uint32_t;

enum class mode_e : uint8_t
{
  Master,
  Slave,
};

enum class device_type_e : uint8_t
{
  General,
  memory,
};

enum class state_e : uint8_t
{
  Idle,
  Addressing,
  Transmitting,
  Receiving,
  Error
};

class DWT_Delay
{
public:
  static void init()
  {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT       = 0;
    DWT->CTRL        |= DWT_CTRL_CYCCNTENA_Msk;
  }

  static void delay_us(uint32_t us)
  {
    uint32_t cycles = (SystemCoreClock / 1000000) * us;
    while (DWT->CYCCNT < cycles)
      ;
  }
};

template <typename scl, typename sda, speed_t speed, mode_e mode, device_type_e device_type>
class I2C
{
private:
  static void scl_high() noexcept
  {
    scl_t::write(true);
  }

  static void scl_low() noexcept
  {
    scl_t::write(false);
  }

  static void sda_high() noexcept
  {
    sda_t::write(true);
  }

  static void sda_low() noexcept
  {
    sda_t::write(false);
  }

  static bool sda_read() noexcept
  {
    return sda_t::read();
  }

  static void delay() noexcept
  {
    DWT_Delay::delay_us(1);
  }

  static bool start_condition() noexcept
  {
    if (sda_read() == false)
    {
      return false;
    }
    else
    {
      sda_low();
      delay();
      scl_low();
      delay();
      return true;
    }
  }
};

}   // namespace QAQ::I2C

#endif /* __I2C_HPP__ */
