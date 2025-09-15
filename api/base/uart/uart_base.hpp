#ifndef __UART_BASE_HPP__
#define __UART_BASE_HPP__

#include "streaming_device.hpp"
#include "signal.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 基本对象
namespace base
{
/// @brief 名称空间 串口
namespace uart
{
/// @brief Uart 工作模式
enum class Uart_Type : uint8_t
{
  Normal,            /* 正常模式 */
  Interrupt,         /* 中断模式 */
  DMA,               /* DMA 模式 */
  DMA_Double_Buffer, /* DMA 双缓冲区模式 */
};

/// @brief Uart 配置信息选项
class Config
{
public:
  /// @brief 波特率
  static constexpr uint32_t Baud_Rate              = 0x01;
  /// @brief 数据位
  static constexpr uint32_t Data_Bits              = 0x02;
  /// @brief 停止位
  static constexpr uint32_t Stop_Bits              = 0x03;
  /// @brief 校验位
  static constexpr uint32_t Parity                 = 0x04;
  /// @brief 中断优先级
  static constexpr uint32_t Interrupt_Priority     = 0x05;
  /// @brief 中断子优先级
  static constexpr uint32_t Interrupt_Sub_Priority = 0x06;
  /// @brief 端口号
  static constexpr uint32_t Port_Num               = 0x07;
};

/// @brief Uart 校验模式
class Uart_Parity
{
public:
  /// @brief 无校验
  static constexpr uint8_t None = 0x00;
  /// @brief 偶校验
  static constexpr uint8_t Even = 0x01;
  /// @brief 奇校验
  static constexpr uint8_t Odd  = 0x02;
};
} /* namespace uart */

/// @brief 命名空间 内部
namespace base_internal
{
/// @brief 名称空间 串口 内部
namespace uart_internal
{
/// @brief Uart 默认波特率
static constexpr uint32_t Default_Baud_Rate = 9600;
/// @brief Uart 默认数据位
static constexpr uint8_t  Default_Data_Bits = 8;
/// @brief Uart 默认停止位
static constexpr uint8_t  Default_Stop_Bits = 1;
/// @brief Uart 默认校验位
static constexpr uint8_t  Default_Parity    = uart::Uart_Parity::None;

/**
 * @brief  Uart 基类
 *
 * @tparam Type         工作模式
 * @tparam Config       配置模版
 * @tparam Base_Device  设备基类
 * @tparam DE_Pin       DE 引脚
 * @tparam RE_Pin       RE 引脚
 */
template <system::device::Stream_Type Type, typename Config, typename Base_Device, typename DE_Pin = void, typename RE_Pin = void>
class Uart_Base;

/**
 * @brief  Uart 配置接口类
 *
 * @tparam T    目标模版
 */
template <typename T>
class Uart_Set_Config
{
  /// @brief Uart 配置码
  using Uart_Config_Code = uart::Config;

  /// @brief 友元类 Uart 基类
  template <system::device::Stream_Type Type, typename Config, typename Base_Device, typename DE_Pin, typename RE_Pin>
  friend class Uart_Base;

  /// @brief 目标 Uart 对象引用
  T& uart;

  /**
   * @brief 构造函数
   *
   * @param uart 目标 Uart 对象引用
   */
  explicit Uart_Set_Config(T& uart) : uart(uart) {}

  /**
   * @brief 析构函数
   */
  ~Uart_Set_Config() {}

public:
  /**
   * @brief  Uart 配置 波特率
   *
   * @param  baud_rate        波特率
   * @return Uart_Set_Config& 配置接口引用
   */
  Uart_Set_Config& baud_rate(uint32_t baud_rate)
  {
    uart.config(Uart_Config_Code::Baud_Rate, baud_rate);
    return *this;
  }

  /**
   * @brief  Uart 配置 数据位
   *
   * @param  data_bits        数据位
   * @return Uart_Set_Config& 配置接口引用
   */
  Uart_Set_Config& data_bits(uint32_t data_bits)
  {
    uart.config(Uart_Config_Code::Data_Bits, data_bits);
    return *this;
  }

  /**
   * @brief  Uart 配置 停止位
   *
   * @param  stop_bits        停止位
   * @return Uart_Set_Config& 配置接口引用
   */
  Uart_Set_Config& stop_bits(uint32_t stop_bits)
  {
    uart.config(Uart_Config_Code::Stop_Bits, stop_bits);
    return *this;
  }

  /**
   * @brief  Uart 配置 校验位
   *
   * @param  parity           校验位
   * @return Uart_Set_Config& 配置接口引用
   */
  Uart_Set_Config& parity(uint32_t parity)
  {
    uart.config(Uart_Config_Code::Parity, parity);
    return *this;
  }

  /**
   * @brief  Uart 配置 中断优先级
   *
   * @param  interrupt_priority   中断优先级
   * @return Uart_Set_Config&     配置接口引用
   */
  Uart_Set_Config& interrupt_priority(uint32_t interrupt_priority)
  {
    uart.config(Uart_Config_Code::Interrupt_Priority, interrupt_priority);
    return *this;
  }

  /**
   * @brief  Uart 配置 中断子优先级
   *
   * @param  interrupt_sub_priority   中断子优先级
   * @return Uart_Set_Config&         配置接口引用
   */
  Uart_Set_Config& interrupt_sub_priority(uint32_t interrupt_sub_priority)
  {
    uart.config(Uart_Config_Code::Interrupt_Sub_Priority, interrupt_sub_priority);
    return *this;
  }
};

/**
 * @brief  Uart 获取配置接口类
 *
 * @tparam T    目标模版
 */
template <typename T>
class Uart_Get_Config
{
  /// @brief Uart 错误码
  using Uart_Error_Code  = system::device::Device_Error_Code;
  /// @brief Uart 配置码
  using Uart_Config_Code = uart::Config;

  /// @brief 友元类 Uart 基类
  template <system::device::Stream_Type Type, typename Config, typename Base_Device, typename DE_Pin, typename RE_Pin>
  friend class Uart_Base;

  /// @brief 目标 Uart 对象引用
  const T& uart;

  /**
   * @brief 构造函数
   *
   * @param uart 目标 Uart 对象引用
   */
  explicit Uart_Get_Config(const T& uart) : uart(uart) {}

  /**
   * @brief 析构函数
   */
  ~Uart_Get_Config() {}

public:
  /**
   * @brief  Uart 获取 波特率
   *
   * @return uint32_t 波特率
   */
  uint32_t baud_rate() const
  {
    return uart.get_config(Uart_Config_Code::Baud_Rate);
  }

  /**
   * @brief  Uart 获取 波特率
   *
   * @param  baud_rate        波特率引用
   * @return Uart_Get_Config& 配置接口引用
   */
  Uart_Get_Config& baud_rate(uint32_t& baud_rate) const
  {
    baud_rate = uart.get_config(Uart_Config_Code::Baud_Rate);
    return *this;
  }

  /**
   * @brief  Uart 获取 数据位
   *
   * @return uint32_t 数据位
   */
  uint32_t data_bits() const
  {
    return uart.get_config(Uart_Config_Code::Data_Bits);
  }

  /**
   * @brief  Uart 获取 数据位
   *
   * @param  data_bits        数据位引用
   * @return Uart_Get_Config& 配置接口引用
   */
  Uart_Get_Config& data_bits(uint32_t& data_bits) const
  {
    data_bits = uart.get_config(Uart_Config_Code::Data_Bits);
    return *this;
  }

  /**
   * @brief  Uart 获取 停止位
   *
   * @return uint32_t 停止位
   */
  uint32_t stop_bits() const
  {
    return uart.get_config(Uart_Config_Code::Stop_Bits);
  }

  /**
   * @brief  Uart 获取 停止位
   *
   * @param  stop_bits        停止位引用
   * @return Uart_Get_Config& 配置接口引用
   */
  Uart_Get_Config& stop_bits(uint32_t& stop_bits) const
  {
    stop_bits = uart.get_config(Uart_Config_Code::Stop_Bits);
    return *this;
  }

  /**
   * @brief  Uart 获取 校验位
   *
   * @return uint32_t 校验位
   */
  uint32_t parity() const
  {
    return uart.get_config(Uart_Config_Code::Parity);
  }

  /**
   * @brief  Uart 获取 校验位
   *
   * @param  parity           校验位引用
   * @return Uart_Get_Config& 配置接口引用
   */
  Uart_Get_Config& parity(uint32_t& parity) const
  {
    parity = uart.get_config(Uart_Config_Code::Parity);
    return *this;
  }

  /**
   * @brief  Uart 获取 中断优先级
   *
   * @return uint32_t 中断优先级
   */
  uint32_t interrupt_priority() const
  {
    return uart.get_config(Uart_Config_Code::Interrupt_Priority);
  }

  /**
   * @brief  Uart 获取 中断优先级
   *
   * @param  interrupt_priority   中断优先级引用
   * @return Uart_Get_Config&     配置接口引用
   */
  Uart_Get_Config& interrupt_priority(uint32_t& interrupt_priority) const
  {
    interrupt_priority = uart.get_config(Uart_Config_Code::Interrupt_Priority);
    return *this;
  }

  /**
   * @brief  Uart 获取 中断子优先级
   *
   * @return uint32_t 中断子优先级
   */
  uint32_t interrupt_sub_priority() const
  {
    return uart.get_config(Uart_Config_Code::Interrupt_Sub_Priority);
  }

  /**
   * @brief  Uart 获取 中断子优先级
   *
   * @param  interrupt_sub_priority   中断子优先级引用
   * @return Uart_Get_Config&         配置接口引用
   */
  Uart_Get_Config& interrupt_sub_priority(uint32_t& interrupt_sub_priority) const
  {
    interrupt_sub_priority = uart.get_config(Uart_Config_Code::Interrupt_Sub_Priority);
    return *this;
  }

  /**
   * @brief  Uart 获取 端口号
   *
   * @return uint32_t 端口号
   */
  uint32_t port_num() const
  {
    return uart.get_config(Uart_Config_Code::Port_Num);
  }

  /**
   * @brief  Uart 获取 端口号
   *
   * @param  port_num         端口号引用
   * @return Uart_Get_Config& 配置接口引用
   */
  Uart_Get_Config& port_num(uint32_t& port_num) const
  {
    port_num = uart.get_config(Uart_Config_Code::Port_Num);
    return *this;
  }
};

/**
 * @brief  Uart 基类 - 读写模式特化
 *
 * @tparam Config       配置模版
 * @tparam Base_Device  设备基类
 * @tparam DE_Pin       DE 引脚
 * @tparam RE_Pin       RE 引脚
 */
template <typename Config, typename Base_Device, typename DE_Pin, typename RE_Pin>
class Uart_Base<system::device::Stream_Type::READ_ONLY, Config, Base_Device, DE_Pin, RE_Pin> : public system::device::Stream_Device<Base_Device::stream_type(), Base_Device::input_buffer_size(), Base_Device::output_buffer_size(), Config::get_received_buffer_mode()>
{
  /// @warning Uart只能是流设备
  static_assert(Base_Device::type() == system::device::Device_Type::STREAMING, "Uart must be streaming device");

  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(Uart_Base)

  /// @brief Uart 工作模式
  using Uart_Type        = uart::Uart_Type;
  /// @brief Uart 错误码
  using Uart_Error_Code  = system::device::Device_Error_Code;
  /// @brief Uart 配置码
  using Uart_Config_Code = uart::Config;
  /// @brief Uart 当前类型
  using Type             = Uart_Base<Base_Device::stream_type(), Config, Base_Device, DE_Pin, RE_Pin>;
  /// @brief Uart 基类类型
  using Base             = system::device::Stream_Device<Base_Device::stream_type(), Base_Device::input_buffer_size(), Base_Device::output_buffer_size(), Config::get_received_buffer_mode()>;

private:
  /// @brief Uart 配置接口模版类
  Uart_Set_Config<Type> m_set_config;
  /// @brief Uart 获取配置接口模版类
  Uart_Get_Config<Type> m_get_config;

  /**
   * @brief Uart 接收字节回调函数
   *
   * @param arg  入口参数
   * @note  该函数仅在中断输入模式下有效
   */
  template <bool enable = (Uart_Type::Interrupt == Config::received_type()), typename = std::enable_if_t<enable>>
  static void received_byte_callback(void* arg)
  {
    static_cast<Type*>(arg)->input_buffer_push(Config::read_rdr());
  }

  /**
   * @brief Uart 接收完成回调函数
   *
   * @param arg  入口参数
   */
  static void received_complete_callback(void* arg)
  {
    Type* uart = static_cast<Type*>(arg);

    if constexpr (Uart_Type::Interrupt == Config::received_type())
    {
      uart->input_complete();
    }
    else if constexpr (Uart_Type::DMA == Config::received_type())
    {
      uint8_t* memory_ptr  = nullptr;
      uint32_t memory_size = uart->input_buffer_size() - 1;

      uart->input_complete(Config::receive_size());
      memory_ptr = uart->input_buffer_ptr(memory_size);
      Config::enable_receive(memory_ptr, memory_size);
    }
    else if constexpr (Uart_Type::DMA_Double_Buffer == Config::received_type())
    {
      uint8_t* memory0_ptr = nullptr;
      uint8_t* memory1_ptr = nullptr;
      uint32_t memory_size = 0;

      uart->input_complete(Config::receive_size());
      memory_size = uart->input_buffer_ptr(memory0_ptr, memory1_ptr);
      Config::enable_receive(memory0_ptr, memory1_ptr, memory_size);
    }

    uart->signal_receive_complete(uart);
  }

  /**
   * @brief Uart 内存切换回调函数
   *
   * @param arg  入口参数
   * @note  该函数仅在DMA双缓存输入模式下有效
   */
  template <bool enable = (Uart_Type::DMA_Double_Buffer == Config::received_type()), typename = std::enable_if_t<enable>>
  static void memory_switch_callback(void* arg)
  {
    Type* uart = static_cast<Type*>(arg);

    uart->memory_switch();
    uart->signal_receive_complete(uart);
  }

  /**
   * @brief Uart 设备管理器事件处理句柄
   *
   * @param event 事件标志
   */
  void manger_handler(uint32_t event) override
  {
    if (event & static_cast<uint32_t>(system::system_internal::device_internal::Device_Event_Bits::Enable_Receive))
    {
      if constexpr (Uart_Type::Interrupt == Config::received_type())
      {
        Config::enable_receive();
      }
      else if constexpr (Uart_Type::DMA == Config::received_type())
      {
        uint32_t memory_size = this->input_buffer_size() - 1;
        uint8_t* memory_ptr  = this->input_buffer_ptr(memory_size);

        Config::enable_receive(memory_ptr, memory_size);
      }
      else if constexpr (Uart_Type::DMA_Double_Buffer == Config::received_type())
      {
        uint8_t* memory0_ptr = nullptr;
        uint8_t* memory1_ptr = nullptr;
        uint32_t memory_size = this->input_buffer_ptr(memory0_ptr, memory1_ptr);

        Config::enable_receive(memory0_ptr, memory1_ptr, memory_size);
      }
    }
  }

  /**
   * @brief Uart 设备打开 - 元方法覆写
   *
   * @return Uart_Error_Code 错误码
   */
  Uart_Error_Code open_impl() override
  {
    Uart_Error_Code error_code = Uart_Error_Code::OK;

    if constexpr (!std::is_same_v<DE_Pin, void>)
    {
      DE_Pin::interrupt_delete();
      DE_Pin::setup();
      DE_Pin::reset();
    }

    if constexpr (!std::is_same_v<RE_Pin, void>)
    {
      RE_Pin::interrupt_delete();
      RE_Pin::setup();
      RE_Pin::reset();
    }

    error_code = Config::init(Default_Baud_Rate, Default_Data_Bits, Default_Stop_Bits, Default_Parity, Base_Device::stream_type());

    if (Uart_Error_Code::OK == error_code)
    {
      if constexpr (Uart_Type::Interrupt == Config::received_type())
      {
        Config::set_received_byte_callback(received_byte_callback, this);
      }
      else if constexpr (Uart_Type::DMA_Double_Buffer == Config::received_type())
      {
        Config::set_memory_switch_callback(memory_switch_callback, this);
      }

      Config::set_received_complete_callback(received_complete_callback, this);
    }

    return error_code;
  }

  /**
   * @brief Uart 设备关闭 - 元方法覆写
   *
   * @return Uart_Error_Code 错误码
   */
  Uart_Error_Code close_impl() override
  {
    if constexpr (!std::is_same_v<DE_Pin, void>)
    {
      DE_Pin::clearup();
    }

    if constexpr (!std::is_same_v<RE_Pin, void>)
    {
      RE_Pin::clearup();
    }

    return Config::deinit();
  }

  /**
   * @brief Uart 设备配置 - 元方法覆写
   *
   * @param  param            配置参数
   * @param  value            配置值
   * @return Uart_Error_Code  错误码
   */
  Uart_Error_Code config_impl(uint32_t param, uint32_t value) override
  {
    Uart_Error_Code error_code = Uart_Error_Code::OK;

    if (Uart_Config_Code::Baud_Rate == param)
    {
      error_code = Config::set_baud_rate(value);
    }
    else if (Uart_Config_Code::Data_Bits == param)
    {
      error_code = Config::set_data_bits(value);
    }
    else if (Uart_Config_Code::Stop_Bits == param)
    {
      error_code = Config::set_stop_bits(value);
    }
    else if (Uart_Config_Code::Parity == param)
    {
      error_code = Config::set_parity(value);
    }
    else if (Uart_Config_Code::Interrupt_Priority == param)
    {
      error_code = Config::set_interrupt_priority(value);
    }
    else if (Uart_Config_Code::Interrupt_Sub_Priority == param)
    {
      error_code = Config::set_interrupt_sub_priority(value);
    }
    else
    {
      error_code = Uart_Error_Code::INVALID_PARAMETER;
    }

    return error_code;
  }

  /**
   * @brief Uart 设备配置获取 - 元方法覆写
   *
   * @param  param            配置参数
   * @return uint32_t         配置值
   */
  const uint32_t get_config_impl(uint32_t param) override
  {
    uint32_t value = 0;

    if (Uart_Config_Code::Baud_Rate == param)
    {
      value = Config::get_baud_rate();
    }
    else if (Uart_Config_Code::Data_Bits == param)
    {
      value = Config::get_data_bits();
    }
    else if (Uart_Config_Code::Stop_Bits == param)
    {
      value = Config::get_stop_bits();
    }
    else if (Uart_Config_Code::Parity == param)
    {
      value = Config::get_parity();
    }
    else if (Uart_Config_Code::Interrupt_Priority == param)
    {
      value = Config::get_interrupt_priority();
    }
    else if (Uart_Config_Code::Interrupt_Sub_Priority == param)
    {
      value = Config::get_interrupt_sub_priority();
    }
    else if (Uart_Config_Code::Port_Num == param)
    {
      value = Config::get_port_num();
    }

    return value;
  }

public:
  /// @brief Uart 设备配置 - 接口声明
  using Base::config;
  /// @brief Uart 设备配置获取 - 接口声明
  using Base::get_config;

  /// @brief Uart 设备接收完成信号
  system::signal::Signal<system::system_internal::device_internal::InDevice_Base*> signal_receive_complete;

  /**
   * @brief Uart 构造函数
   *
   */
  explicit Uart_Base() : m_set_config(*this), m_get_config(*this) {}

  /**
   * @brief  Uart 配置设置接口
   *
   * @return Uart_Set_Config<Type>&  Uart 配置接口类(引用)
   */
  Uart_Set_Config<Type>& config(void)
  {
    return m_set_config;
  }

  /**
   * @brief Uart 配置获取接口
   *
   * @return Uart_Get_Config<Type>&  Uart 获取配置接口类(引用)
   */
  Uart_Get_Config<Type>& get_config(void)
  {
    return m_get_config;
  }

  /**
   * @brief Uart 析构函数
   *
   */
  virtual ~Uart_Base() {}
};

/**
 * @brief  Uart 基类 - 只写模式特化
 *
 * @tparam Config       配置模版
 * @tparam Base_Device  设备基类
 * @tparam DE_Pin       DE 引脚
 * @tparam RE_Pin       RE 引脚
 */
template <typename Config, typename Base_Device, typename DE_Pin, typename RE_Pin>
class Uart_Base<system::device::Stream_Type::WRITE_ONLY, Config, Base_Device, DE_Pin, RE_Pin> : public system::device::Stream_Device<Base_Device::stream_type(), Base_Device::input_buffer_size(), Base_Device::output_buffer_size(), Config::get_received_buffer_mode()>
{
  /// @warning Uart只能是流设备
  static_assert(Base_Device::type() == system::device::Device_Type::STREAMING, "Uart must be streaming device");

  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(Uart_Base)

  /// @brief Uart 工作模式
  using Uart_Type        = uart::Uart_Type;
  /// @brief Uart 错误码
  using Uart_Error_Code  = system::device::Device_Error_Code;
  /// @brief Uart 配置码
  using Uart_Config_Code = uart::Config;
  /// @brief Uart 当前类型
  using Type             = Uart_Base<Base_Device::stream_type(), Config, Base_Device, DE_Pin, RE_Pin>;
  /// @brief Uart 基类类型
  using Base             = system::device::Stream_Device<Base_Device::stream_type(), Base_Device::input_buffer_size(), Base_Device::output_buffer_size(), Config::get_received_buffer_mode()>;

private:
  /// @brief Uart 配置接口模版类
  Uart_Set_Config<Type> m_set_config;
  /// @brief Uart 获取配置接口模版类
  Uart_Get_Config<Type> m_get_config;

  /**
   * @brief Uart 发送完成回调函数
   *
   * @param arg  入口参数
   */
  static void send_complete_callback(void* arg)
  {
    Type* uart = static_cast<Type*>(arg);

    uart->output_complete();
    uart->signal_send_complete(uart);
  }

  /**
   * @brief  Uart 数据发送 - 元方法覆写
   *
   * @param  data     数据指针
   * @param  size     数据大小
   * @return uint32_t 实际发送大小
   */
  uint32_t send_impl(const uint8_t* data, uint32_t size) override
  {
    return Config::send(data, size);
  }

  /**
   * @brief Uart 设备管理器事件处理句柄
   *
   * @param event 事件标志
   */
  void manger_handler(uint32_t event) override
  {
    if constexpr (0 < this->output_buffer_size())
    {
      if (event & static_cast<uint32_t>(system::system_internal::device_internal::Device_Event_Bits::Enable_Transfer))
      {
        uint32_t size = 0;
        uint8_t* ptr  = this->output_start(size);

        Config::send(ptr, size);
      }
    }
  }

  /**
   * @brief Uart 设备打开 - 元方法覆写
   *
   * @return Uart_Error_Code 错误码
   */
  Uart_Error_Code open_impl() override
  {
    Uart_Error_Code error_code = Uart_Error_Code::OK;

    if constexpr (!std::is_same_v<DE_Pin, void>)
    {
      DE_Pin::interrupt_delete();
      DE_Pin::setup();
      DE_Pin::reset();
    }

    if constexpr (!std::is_same_v<RE_Pin, void>)
    {
      RE_Pin::interrupt_delete();
      RE_Pin::setup();
      RE_Pin::reset();
    }

    error_code = Config::init(Default_Baud_Rate, Default_Data_Bits, Default_Stop_Bits, Default_Parity, Base_Device::stream_type());

    if (Uart_Error_Code::OK == error_code)
    {
      Config::set_send_complete_callback(send_complete_callback, this);
    }

    return error_code;
  }

  /**
   * @brief Uart 设备关闭 - 元方法覆写
   *
   * @return Uart_Error_Code 错误码
   */
  Uart_Error_Code close_impl() override
  {
    if constexpr (!std::is_same_v<DE_Pin, void>)
    {
      DE_Pin::clearup();
    }

    if constexpr (!std::is_same_v<RE_Pin, void>)
    {
      RE_Pin::clearup();
    }

    return Config::deinit();
  }

  /**
   * @brief Uart 设备配置 - 元方法覆写
   *
   * @param  param            配置参数
   * @param  value            配置值
   * @return Uart_Error_Code  错误码
   */
  Uart_Error_Code config_impl(uint32_t param, uint32_t value) override
  {
    Uart_Error_Code error_code = Uart_Error_Code::OK;

    if (Uart_Config_Code::Baud_Rate == param)
    {
      error_code = Config::set_baud_rate(value);
    }
    else if (Uart_Config_Code::Data_Bits == param)
    {
      error_code = Config::set_data_bits(value);
    }
    else if (Uart_Config_Code::Stop_Bits == param)
    {
      error_code = Config::set_stop_bits(value);
    }
    else if (Uart_Config_Code::Parity == param)
    {
      error_code = Config::set_parity(value);
    }
    else if (Uart_Config_Code::Interrupt_Priority == param)
    {
      error_code = Config::set_interrupt_priority(value);
    }
    else if (Uart_Config_Code::Interrupt_Sub_Priority == param)
    {
      error_code = Config::set_interrupt_sub_priority(value);
    }
    else
    {
      error_code = Uart_Error_Code::INVALID_PARAMETER;
    }

    return error_code;
  }

  /**
   * @brief Uart 设备配置获取 - 元方法覆写
   *
   * @param  param            配置参数
   * @return uint32_t         配置值
   */
  const uint32_t get_config_impl(uint32_t param) override
  {
    uint32_t value = 0;

    if (Uart_Config_Code::Baud_Rate == param)
    {
      value = Config::get_baud_rate();
    }
    else if (Uart_Config_Code::Data_Bits == param)
    {
      value = Config::get_data_bits();
    }
    else if (Uart_Config_Code::Stop_Bits == param)
    {
      value = Config::get_stop_bits();
    }
    else if (Uart_Config_Code::Parity == param)
    {
      value = Config::get_parity();
    }
    else if (Uart_Config_Code::Interrupt_Priority == param)
    {
      value = Config::get_interrupt_priority();
    }
    else if (Uart_Config_Code::Interrupt_Sub_Priority == param)
    {
      value = Config::get_interrupt_sub_priority();
    }
    else if (Uart_Config_Code::Port_Num == param)
    {
      value = Config::get_port_num();
    }

    return value;
  }

public:
  /// @brief Uart 设备配置 - 接口声明
  using Base::config;
  /// @brief Uart 设备配置获取 - 接口声明
  using Base::get_config;

  /// @brief Uart 设备发送完成信号
  system::signal::Signal<system::system_internal::device_internal::OutDevice_Base*> signal_send_complete;

  /**
   * @brief Uart 构造函数
   *
   */
  explicit Uart_Base() : m_set_config(*this), m_get_config(*this) {}

  /**
   * @brief  Uart 配置设置接口
   *
   * @return Uart_Set_Config<Type>&  Uart 配置接口类(引用)
   */
  Uart_Set_Config<Type>& config(void)
  {
    return m_set_config;
  }

  /**
   * @brief Uart 配置获取接口
   *
   * @return Uart_Get_Config<Type>&  Uart 获取配置接口类(引用)
   */
  Uart_Get_Config<Type>& get_config(void)
  {
    return m_get_config;
  }

  /**
   * @brief Uart 析构函数
   *
   */
  virtual ~Uart_Base() {}
};

/**
 * @brief  Uart 基类 - 读写模式特化
 *
 * @tparam Config       配置模版
 * @tparam Base_Device  设备基类
 * @tparam DE_Pin       DE 引脚
 * @tparam RE_Pin       RE 引脚
 */
template <typename Config, typename Base_Device, typename DE_Pin, typename RE_Pin>
class Uart_Base<system::device::Stream_Type::READ_WRITE, Config, Base_Device, DE_Pin, RE_Pin> : public system::device::Stream_Device<Base_Device::stream_type(), Base_Device::input_buffer_size(), Base_Device::output_buffer_size(), Config::get_received_buffer_mode()>
{
  /// @warning Uart只能是流设备
  static_assert(Base_Device::type() == system::device::Device_Type::STREAMING, "Uart must be streaming device");

  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(Uart_Base)

  /// @brief Uart 工作模式
  using Uart_Type        = uart::Uart_Type;
  /// @brief Uart 错误码
  using Uart_Error_Code  = system::device::Device_Error_Code;
  /// @brief Uart 配置码
  using Uart_Config_Code = uart::Config;
  /// @brief Uart 当前类型
  using Type             = Uart_Base<Base_Device::stream_type(), Config, Base_Device, DE_Pin, RE_Pin>;
  /// @brief Uart 基类类型
  using Base             = system::device::Stream_Device<Base_Device::stream_type(), Base_Device::input_buffer_size(), Base_Device::output_buffer_size(), Config::get_received_buffer_mode()>;

private:
  /// @brief Uart 配置接口模版类
  Uart_Set_Config<Type> m_set_config;
  /// @brief Uart 获取配置接口模版类
  Uart_Get_Config<Type> m_get_config;

  /**
   * @brief Uart 接收字节回调函数
   *
   * @param arg  入口参数
   * @note  该函数仅在中断输入模式下有效
   */
  template <bool enable = (Uart_Type::Interrupt == Config::received_type()), typename = std::enable_if_t<enable>>
  static void received_byte_callback(void* arg)
  {
    static_cast<Type*>(arg)->input_buffer_push(Config::read_rdr());
  }

  /**
   * @brief Uart 接收完成回调函数
   *
   * @param arg  入口参数
   */
  static void received_complete_callback(void* arg)
  {
    Type* uart = static_cast<Type*>(arg);

    if constexpr (Uart_Type::Interrupt == Config::received_type())
    {
      uart->input_complete();
    }
    else if constexpr (Uart_Type::DMA == Config::received_type())
    {
      uint8_t* memory_ptr  = nullptr;
      uint32_t memory_size = uart->input_buffer_size() - 1;

      uart->input_complete(Config::receive_size());
      memory_ptr = uart->input_buffer_ptr(memory_size);
      Config::enable_receive(memory_ptr, memory_size);
    }
    else if constexpr (Uart_Type::DMA_Double_Buffer == Config::received_type())
    {
      uint8_t* memory0_ptr = nullptr;
      uint8_t* memory1_ptr = nullptr;
      uint32_t memory_size = 0;

      uart->input_complete(Config::receive_size());
      memory_size = uart->input_buffer_ptr(memory0_ptr, memory1_ptr);
      Config::enable_receive(memory0_ptr, memory1_ptr, memory_size);
    }

    uart->signal_receive_complete(uart);
  }

  /**
   * @brief Uart 内存切换回调函数
   *
   * @param arg  入口参数
   * @note  该函数仅在DMA双缓存输入模式下有效
   */
  template <bool enable = (Uart_Type::DMA_Double_Buffer == Config::received_type()), typename = std::enable_if_t<enable>>
  static void memory_switch_callback(void* arg)
  {
    Type* uart = static_cast<Type*>(arg);

    uart->memory_switch();
    uart->signal_receive_complete(uart);
  }

  /**
   * @brief Uart 发送完成回调函数
   *
   * @param arg  入口参数
   */
  static void send_complete_callback(void* arg)
  {
    Type* uart = static_cast<Type*>(arg);

    uart->output_complete();

    if constexpr (!std::is_same_v<DE_Pin, void>)
    {
      DE_Pin::reset();
    }

    if constexpr (!std::is_same_v<RE_Pin, void>)
    {
      RE_Pin::reset();
    }

    uart->signal_send_complete(uart);
  }

  /**
   * @brief  Uart 数据发送 - 元方法覆写
   *
   * @param  data     数据指针
   * @param  size     数据大小
   * @return uint32_t 实际发送大小
   */
  uint32_t send_impl(const uint8_t* data, uint32_t size) override
  {
    return Config::send(data, size);
  }

  /**
   * @brief Uart 设备管理器事件处理句柄
   *
   * @param event 事件标志
   */
  void manger_handler(uint32_t event) override
  {
    if constexpr (0 < this->output_buffer_size())
    {
      if (event & static_cast<uint32_t>(system::system_internal::device_internal::Device_Event_Bits::Enable_Transfer))
      {
        uint32_t size = 0;
        uint8_t* ptr  = this->output_start(size);

        if constexpr (!std::is_same_v<DE_Pin, void>)
        {
          DE_Pin::set();
        }

        if constexpr (!std::is_same_v<RE_Pin, void>)
        {
          RE_Pin::set();
        }

        Config::send(ptr, size);
      }
      else if (event & static_cast<uint32_t>(system::system_internal::device_internal::Device_Event_Bits::Enable_Receive))
      {
        if constexpr (Uart_Type::Interrupt == Config::received_type())
        {
          Config::enable_receive();
        }
        else if constexpr (Uart_Type::DMA == Config::received_type())
        {
          uint32_t memory_size = this->input_buffer_size() - 1;
          uint8_t* memory_ptr  = this->input_buffer_ptr(memory_size);

          Config::enable_receive(memory_ptr, memory_size);
        }
        else if constexpr (Uart_Type::DMA_Double_Buffer == Config::received_type())
        {
          uint8_t* memory0_ptr = nullptr;
          uint8_t* memory1_ptr = nullptr;
          uint32_t memory_size = this->input_buffer_ptr(memory0_ptr, memory1_ptr);

          Config::enable_receive(memory0_ptr, memory1_ptr, memory_size);
        }
      }
    }
    else
    {
      if (event & static_cast<uint32_t>(system::system_internal::device_internal::Device_Event_Bits::Enable_Receive))
      {
        if constexpr (Uart_Type::Interrupt == Config::received_type())
        {
          Config::enable_receive();
        }
        else if constexpr (Uart_Type::DMA == Config::received_type())
        {
          uint32_t memory_size = this->input_buffer_size() - 1;
          uint8_t* memory_ptr  = this->input_buffer_ptr(memory_size);

          Config::enable_receive(memory_ptr, memory_size);
        }
        else if constexpr (Uart_Type::DMA_Double_Buffer == Config::received_type())
        {
          uint8_t* memory0_ptr = nullptr;
          uint8_t* memory1_ptr = nullptr;
          uint32_t memory_size = this->input_buffer_ptr(memory0_ptr, memory1_ptr);

          Config::enable_receive(memory0_ptr, memory1_ptr, memory_size);
        }
      }
    }
  }

  /**
   * @brief Uart 设备打开 - 元方法覆写
   *
   * @return Uart_Error_Code 错误码
   */
  Uart_Error_Code open_impl() override
  {
    Uart_Error_Code error_code = Uart_Error_Code::OK;

    if constexpr (!std::is_same_v<DE_Pin, void>)
    {
      DE_Pin::interrupt_delete();
      DE_Pin::setup();
      DE_Pin::reset();
    }

    if constexpr (!std::is_same_v<RE_Pin, void>)
    {
      RE_Pin::interrupt_delete();
      RE_Pin::setup();
      RE_Pin::reset();
    }

    error_code = Config::init(Default_Baud_Rate, Default_Data_Bits, Default_Stop_Bits, Default_Parity, Base_Device::stream_type());

    if (Uart_Error_Code::OK == error_code)
    {
      if constexpr (Uart_Type::Interrupt == Config::received_type())
      {
        Config::set_received_byte_callback(received_byte_callback, this);
      }
      else if constexpr (Uart_Type::DMA_Double_Buffer == Config::received_type())
      {
        Config::set_memory_switch_callback(memory_switch_callback, this);
      }

      Config::set_received_complete_callback(received_complete_callback, this);
      Config::set_send_complete_callback(send_complete_callback, this);
    }

    return error_code;
  }

  /**
   * @brief Uart 设备关闭 - 元方法覆写
   *
   * @return Uart_Error_Code 错误码
   */
  Uart_Error_Code close_impl() override
  {
    if constexpr (!std::is_same_v<DE_Pin, void>)
    {
      DE_Pin::clearup();
    }

    if constexpr (!std::is_same_v<RE_Pin, void>)
    {
      RE_Pin::clearup();
    }

    return Config::deinit();
  }

  /**
   * @brief Uart 设备配置 - 元方法覆写
   *
   * @param  param            配置参数
   * @param  value            配置值
   * @return Uart_Error_Code  错误码
   */
  Uart_Error_Code config_impl(uint32_t param, uint32_t value) override
  {
    Uart_Error_Code error_code = Uart_Error_Code::OK;

    if (Uart_Config_Code::Baud_Rate == param)
    {
      error_code = Config::set_baud_rate(value);
    }
    else if (Uart_Config_Code::Data_Bits == param)
    {
      error_code = Config::set_data_bits(value);
    }
    else if (Uart_Config_Code::Stop_Bits == param)
    {
      error_code = Config::set_stop_bits(value);
    }
    else if (Uart_Config_Code::Parity == param)
    {
      error_code = Config::set_parity(value);
    }
    else if (Uart_Config_Code::Interrupt_Priority == param)
    {
      error_code = Config::set_interrupt_priority(value);
    }
    else if (Uart_Config_Code::Interrupt_Sub_Priority == param)
    {
      error_code = Config::set_interrupt_sub_priority(value);
    }
    else
    {
      error_code = Uart_Error_Code::INVALID_PARAMETER;
    }

    return error_code;
  }

  /**
   * @brief Uart 设备配置获取 - 元方法覆写
   *
   * @param  param            配置参数
   * @return uint32_t         配置值
   */
  uint32_t get_config_impl(uint32_t param) const override
  {
    uint32_t value = 0;

    if (Uart_Config_Code::Baud_Rate == param)
    {
      value = Config::get_baud_rate();
    }
    else if (Uart_Config_Code::Data_Bits == param)
    {
      value = Config::get_data_bits();
    }
    else if (Uart_Config_Code::Stop_Bits == param)
    {
      value = Config::get_stop_bits();
    }
    else if (Uart_Config_Code::Parity == param)
    {
      value = Config::get_parity();
    }
    else if (Uart_Config_Code::Interrupt_Priority == param)
    {
      value = Config::get_interrupt_priority();
    }
    else if (Uart_Config_Code::Interrupt_Sub_Priority == param)
    {
      value = Config::get_interrupt_sub_priority();
    }
    else if (Uart_Config_Code::Port_Num == param)
    {
      value = Config::get_port_num();
    }

    return value;
  }

public:
  /// @brief Uart 设备配置 - 接口声明
  using Base::config;
  /// @brief Uart 设备配置获取 - 接口声明
  using Base::get_config;

  /// @brief Uart 设备接收完成信号
  system::signal::Signal<system::system_internal::device_internal::IODevice_Base*> signal_receive_complete;
  /// @brief Uart 设备发送完成信号
  system::signal::Signal<system::system_internal::device_internal::IODevice_Base*> signal_send_complete;

  /**
   * @brief Uart 构造函数
   *
   */
  explicit Uart_Base() : m_set_config(*this), m_get_config(*this) {}

  /**
   * @brief  Uart 配置设置接口
   *
   * @return Uart_Set_Config<Type>&  Uart 配置接口类(引用)
   */
  Uart_Set_Config<Type>& config(void)
  {
    return m_set_config;
  }

  /**
   * @brief Uart 配置获取接口
   *
   * @return Uart_Get_Config<Type>&  Uart 获取配置接口类(引用)
   */
  Uart_Get_Config<Type>& get_config(void)
  {
    return m_get_config;
  }

  /**
   * @brief Uart 析构函数
   *
   */
  virtual ~Uart_Base() {}
};
} /* namespace uart_internal */
} /* namespace base_internal */
} /* namespace base */
} /* namespace QAQ */

#endif /* __UART_BASE_HPP__ */
