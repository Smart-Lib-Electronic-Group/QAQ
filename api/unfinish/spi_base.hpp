#ifndef __SPI_BASE_HPP__
#define __SPI_BASE_HPP__

#include "direct_device.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 基本对象
namespace base
{
/// @brief 名称空间 SPI
namespace spi
{
/// @brief SPI 工作模式
enum class Spi_Type : uint8_t
{
  Normal,    /* 正常模式 */
  Interrupt, /* 中断模式 */
  DMA,       /* DMA 模式 */
};

/// @brief SPI 配置信息选项
class Config
{
public:
  /// @brief 工作模式
  static constexpr uint32_t Mode                   = 0x01;
  /// @brief 字节序
  static constexpr uint32_t Endian                 = 0x02;
  /// @brief 数据大小
  static constexpr uint32_t Data_Size              = 0x03;
  /// @brief 波特率分频系数
  static constexpr uint32_t Baud_Rate_Prescaler    = 0x04;
  /// @brief 时钟极性
  static constexpr uint32_t Clock_Polarity         = 0x05;
  /// @brief 时钟相位
  static constexpr uint32_t Clock_Phase            = 0x06;
  /// @brief 中断优先级
  static constexpr uint32_t Interrupt_Priority     = 0x07;
  /// @brief 中断子优先级
  static constexpr uint32_t Interrupt_Sub_Priority = 0x08;
  /// @brief 端口编号
  static constexpr uint32_t Port_Num               = 0x09;
};

/// @brief SPI 工作模式
class Spi_Mode
{
public:
  /// @brief 全双工从机
  static constexpr uint32_t FULL_DUPLEX_SLAVE  = 0x00;
  /// @brief 半双工从机
  static constexpr uint32_t HALF_DUPLEX_SLAVE  = 0x01;
  /// @brief 全双工主机
  static constexpr uint32_t FULL_DUPLEX_MASTER = 0x02;
  /// @brief 半双工主机
  static constexpr uint32_t HALF_DUPLEX_MASTER = 0x03;
};

/// @brief SPI 字节序
class Spi_Endian
{
public:
  /// @brief 大端序
  static constexpr uint32_t MSB = 0x00;
  /// @brief 小端序
  static constexpr uint32_t LSB = 0x800000;
};

/// @brief SPI 时钟极性
class Spi_Clock_Polarity
{
public:
  /// @brief 低极性
  static constexpr uint32_t LOW_POLARITY  = 0x00;
  /// @brief 高极性
  static constexpr uint32_t HIGH_POLARITY = 0x2000000;
};

/// @brief SPI 时钟相位
class Spi_Clock_Phase
{
public:
  /// @brief 第一个时钟沿
  static constexpr uint32_t FIRST_EDGE  = 0x00;
  /// @brief 第二个时钟沿
  static constexpr uint32_t SECOND_EDGE = 0x01000000;
};
} /* namespace spi */

/// @brief 命名空间 内部
namespace base_internal
{
/// @brief 命名空间 SPI 内部
namespace spi_internal
{
/// @brief Spi 默认工作模式
static constexpr uint32_t Default_Mode                   = spi::Spi_Mode::FULL_DUPLEX_MASTER;
/// @brief Spi 默认字节序
static constexpr uint32_t Default_Endian                 = spi::Spi_Endian::MSB;
/// @brief Spi 默认数据大小
static constexpr uint32_t Default_Data_Size              = 8;
/// @brief Spi 默认波特率分频系数
static constexpr uint32_t Default_Baud_Rate_Prescaler    = 4;
/// @brief Spi 默认时钟极性
static constexpr uint32_t Default_Clock_Polarity         = spi::Spi_Clock_Polarity::LOW_POLARITY;
/// @brief Spi 默认时钟相位
static constexpr uint32_t Default_Clock_Phase            = spi::Spi_Clock_Phase::FIRST_EDGE;
/// @brief Spi 默认中断优先级
static constexpr uint32_t Default_Interrupt_Priority     = 5;
/// @brief Spi 默认中断子优先级
static constexpr uint32_t Default_Interrupt_Sub_Priority = 0;

template <typename Config>
class Spi_Base;

template <typename T>
class Spi_Set_Config
{
  using Spi_Config_Code = spi::Config;

  template <typename Config>
  friend class Spi_Base;

  T& spi;

  explicit Spi_Set_Config(T& spi) : spi(spi) {}

  ~Spi_Set_Config() {};

public:
  Spi_Set_Config& mode(uint32_t mode)
  {
    spi.config(Spi_Config_Code::Mode, mode);
    return *this;
  }

  Spi_Set_Config& endian(uint32_t endian)
  {
    spi.config(Spi_Config_Code::Endian, endian);
    return *this;
  }

  Spi_Set_Config& data_size(uint32_t data_size)
  {
    spi.config(Spi_Config_Code::Data_Size, data_size);
    return *this;
  }

  Spi_Set_Config& baud_rate_prescaler(uint32_t baud_rate_prescaler)
  {
    spi.config(Spi_Config_Code::Baud_Rate_Prescaler, baud_rate_prescaler);
    return *this;
  }

  Spi_Set_Config& clock_polarity(uint32_t clock_polarity)
  {
    spi.config(Spi_Config_Code::Clock_Polarity, clock_polarity);
    return *this;
  }

  Spi_Set_Config& clock_phase(uint32_t clock_phase)
  {
    spi.config(Spi_Config_Code::Clock_Phase, clock_phase);
    return *this;
  }

  Spi_Set_Config& interrupt_priority(uint32_t interrupt_priority)
  {
    spi.config(Spi_Config_Code::Interrupt_Priority, interrupt_priority);
    return *this;
  }

  Spi_Set_Config& interrupt_sub_priority(uint32_t interrupt_sub_priority)
  {
    spi.config(Spi_Config_Code::Interrupt_Sub_Priority, interrupt_sub_priority);
    return *this;
  }
};

template <typename T>
class Spi_Get_Config
{
  using Spi_Error_Code  = system::device::Device_Error_Code;
  using Spi_Config_Code = spi::Config;

  const T& spi;

  explicit Spi_Get_Config(cosnt T& spi) : spi(spi) {}

  ~Spi_Get_Config() {};

public:
  uint32_t mode(void) const
  {
    return spi.get_config(Spi_Config_Code::Mode);
  }

  Spi_Get_Config& mode(uint32_t& mode)
  {
    mode = spi.get_config(Spi_Config_Code::Mode);
    return *this;
  }

  uint32_t endian(void) const
  {
    return spi.get_config(Spi_Config_Code::Endian);
  }

  Spi_Get_Config& endian(uint32_t& endian)
  {
    endian = spi.get_config(Spi_Config_Code::Endian);
    return *this;
  }

  uint32_t data_size(void) const
  {
    return spi.get_config(Spi_Config_Code::Data_Size);
  }

  Spi_Get_Config& data_size(uint32_t& data_size)
  {
    data_size = spi.get_config(Spi_Config_Code::Data_Size);
    return *this;
  }

  uint32_t baud_rate_prescaler(void) const
  {
    return spi.get_config(Spi_Config_Code::Baud_Rate_Prescaler);
  }

  Spi_Get_Config& baud_rate_prescaler(uint32_t& baud_rate_prescaler)
  {
    baud_rate_prescaler = spi.get_config(Spi_Config_Code::Baud_Rate_Prescaler);
    return *this;
  }

  uint32_t clock_polarity(void) const
  {
    return spi.get_config(Spi_Config_Code::Clock_Polarity);
  }

  Spi_Get_Config& clock_polarity(uint32_t& clock_polarity)
  {
    clock_polarity = spi.get_config(Spi_Config_Code::Clock_Polarity);
    return *this;
  }

  uint32_t clock_phase(void) const
  {
    return spi.get_config(Spi_Config_Code::Clock_Phase);
  }

  Spi_Get_Config& clock_phase(uint32_t& clock_phase)
  {
    clock_phase = spi.get_config(Spi_Config_Code::Clock_Phase);
    return *this;
  }

  uint32_t interrupt_priority(void) const
  {
    return spi.get_config(Spi_Config_Code::Interrupt_Priority);
  }

  Spi_Get_Config& interrupt_priority(uint32_t& interrupt_priority)
  {
    interrupt_priority = spi.get_config(Spi_Config_Code::Interrupt_Priority);
    return *this;
  }

  uint32_t interrupt_sub_priority(void) const
  {
    return spi.get_config(Spi_Config_Code::Interrupt_Sub_Priority);
  }

  Spi_Get_Config& interrupt_sub_priority(uint32_t& interrupt_sub_priority)
  {
    interrupt_sub_priority = spi.get_config(Spi_Config_Code::Interrupt_Sub_Priority);
    return *this;
  }

  uint32_t port_num(void) const
  {
    return spi.get_config(Spi_Config_Code::Port_Num);
  }

  Spi_Get_Config& port_num(uint32_t& port_num)
  {
    port_num = spi.get_config(Spi_Config_Code::Port_Num);
    return *this;
  }
};

template <typename Config>
class Spi_Base : public system::device::Direct_Device
{
  QAQ_NO_COPY_MOVE(Spi_Base)

  using Spi_Type        = spi::Spi_Type;
  using Spi_Error_Code  = system::device::Device_Error_Code;
  using Spi_Config_Code = spi::Config;
  using Self            = Spi_Base<Config>;
  using Base            = system::device::Direct_Device;

private:
  Spi_Set_Config<Self> m_set_config;
  Spi_Get_Config<Self> m_get_config;

  static void send_complete_callback(void* arg)
  {
    Self* spi = static_cast<Self*>(arg);
    spi->output_complete();
  }

  static void received_complete_callback(void* arg)
  {
    Self* spi = static_cast<Self*>(arg);
    spi->input_complete();
  }

  void manger_handler(uint32_t event) override
  {
    if (event & system::system_internal::device_internal::Device_Event_Bits::Receive_Timeout)
    {
      if constexpr (Spi_Type::Interrupt == Config::received_type() || Spi_Type::DMA == Config::received_type())
      {
      }
    }
  }

  Spi_Error_Code open_impl(void) override {}

  uint32_t send_impl(const uint8_t* data, uint32_t size) override {}

  uint32_t recv_impl(uint8_t* data, uint32_t size) override {}

  Spi_Error_Code config_impl(uint32_t param, uint32_t value) override {}

  uint32_t get_config_impl(uint32_t param) const override {}

  Spi_Error_Code close_impl(void) override {}

public:
  using Base::config;
  using Base::get_config;

  explicit Spi_Base() : m_set_config(*this), m_get_config(*this) {}

  Spi_Set_Config<Self>& config(void)
  {
    return m_set_config;
  }

  Spi_Get_Config<Self>& get_config(void)
  {
    return m_get_config;
  }

  virtual ~Spi_Base() {}
};
} /* namespace spi_internal */
} /* namespace base_internal */
} /* namespace base */
} /* namespace QAQ */

#endif /* __SPI_BASE_HPP__ */
