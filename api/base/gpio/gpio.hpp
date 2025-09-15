#ifndef __GPIO_HPP__
#define __GPIO_HPP__

#include "exit.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 基本对象
namespace base
{
/// @brief 名称空间 GPIO
namespace gpio
{
/**
 * @brief GPIO 引脚号类型
 */
using Pin_Num = uint8_t;

/**
 * @brief GPIO 端口
 */
enum class Pin_Port : uint8_t
{
  PA = 0, /* GPIO 端口 A */
  PB,     /* GPIO 端口 B */
  PC,     /* GPIO 端口 C */
  PD,     /* GPIO 端口 D */
  PE,     /* GPIO 端口 E */
  PF,     /* GPIO 端口 F */
  PG,     /* GPIO 端口 G */
  PH,     /* GPIO 端口 H */
  PI,     /* GPIO 端口 I */
  PJ,     /* GPIO 端口 J */
  PK,     /* GPIO 端口 K */
};

/**
 * @brief GPIO 工作模式
 */
enum class Pin_Mode : uint8_t
{
  Input = 0U,   /* GPIO 输入模式 */
  Output,       /* GPIO 推挽输出模式 */
  Output_OD,    /* GPIO 开漏输出模式 */
  Alternate,    /* GPIO 复用推挽输出模式 */
  Alternate_OD, /* GPIO 复用开漏输出模式 */
  Analog,       /* GPIO 模拟输入模式 */
};

/**
 * @brief GPIO 边缘检测
 */
enum class Pin_Edge : uint8_t
{
  Rising,  /* GPIO 上升沿检测 */
  Falling, /* GPIO 下降沿检测 */
  Both,    /* GPIO 上升沿和下降沿检测 */
};

/**
 * @brief GPIO 速度
 */
enum class Pin_Speed : uint8_t
{
  Low = 0U,  /* GPIO 低速 */
  Medium,    /* GPIO 中速 */
  High,      /* GPIO 高速 */
  Very_High, /* GPIO 超高速 */
};

/**
 * @brief GPIO 上下拉
 */
enum class Pin_Pull : uint8_t
{
  OFF = 0U, /* GPIO 无上下拉 */
  Up,       /* GPIO 上拉 */
  Down,     /* GPIO 下拉 */
};

/**
 * @brief GPIO 复用通道设置
 */
enum class Pin_Alternate : uint8_t
{
  AF0 = 0U, /* GPIO 复用功能 0 */
  AF1,      /* GPIO 复用功能 1 */
  AF2,      /* GPIO 复用功能 2 */
  AF3,      /* GPIO 复用功能 3 */
  AF4,      /* GPIO 复用功能 4 */
  AF5,      /* GPIO 复用功能 5 */
  AF6,      /* GPIO 复用功能 6 */
  AF7,      /* GPIO 复用功能 7 */
  AF8,      /* GPIO 复用功能 8 */
  AF9,      /* GPIO 复用功能 9 */
  AF10,     /* GPIO 复用功能 10 */
  AF11,     /* GPIO 复用功能 11 */
  AF12,     /* GPIO 复用功能 12 */
  AF13,     /* GPIO 复用功能 13 */
  AF14,     /* GPIO 复用功能 14 */
  AF15,     /* GPIO 复用功能 15 */
};
} /* namespace gpio */

/// @brief 名称空间 内部
namespace base_internal
{
/// @brief 名称空间 GPIO 内部
namespace gpio_internal
{
/**
 * @brief GPIO 寄存器指针类型
 */
using reg32_t = volatile uint32_t;

/**
 * @brief GPIO 获取寄存器指针
 *
 * @tparam port_val GPIO 端口号
 * @return constexpr GPIO_TypeDef* GPIO 寄存器指针
 */
template <gpio::Pin_Port port_val>
constexpr GPIO_TypeDef* get_ptr(void) noexcept
{
  switch (port_val)
  {
    case gpio::Pin_Port::PA :
      return GPIOA;
    case gpio::Pin_Port::PB :
      return GPIOB;
    case gpio::Pin_Port::PC :
      return GPIOC;
    case gpio::Pin_Port::PD :
      return GPIOD;
    case gpio::Pin_Port::PE :
      return GPIOE;
    case gpio::Pin_Port::PF :
      return GPIOF;
    case gpio::Pin_Port::PG :
      return GPIOG;
    case gpio::Pin_Port::PH :
      return GPIOH;
    case gpio::Pin_Port::PI :
      return GPIOI;
    case gpio::Pin_Port::PJ :
      return GPIOJ;
    case gpio::Pin_Port::PK :
      return GPIOK;
    default :
      return nullptr;
  }
}

/**
 * @brief  GPIO 模版基类
 *
 * @tparam port   GPIO 端口号
 * @tparam pin    GPIO 引脚号
 * @tparam speed  GPIO 速度
 */
template <gpio::Pin_Port port, gpio::Pin_Num pin, gpio::Pin_Speed speed = gpio::Pin_Speed::High>
class Gpio_Base
{
  // 端口号合法性判断
  static_assert(static_cast<uint8_t>(port) < static_cast<uint8_t>(gpio::Pin_Port::PK), "Invalid port number");
  // 引脚号合法性判断
  static_assert(pin < 16U, "Invalid pin number");
  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(Gpio_Base)

private:
  // 端口寄存器类型声明
  using Gpio_Reg                       = GPIO_TypeDef;
  // 引脚模式枚举类型声明
  using Pin_Mode                       = gpio::Pin_Mode;
  /// @brief 端口寄存器指针
  static inline Gpio_Reg* const m_reg  = get_ptr<port>();
  /// @brief 工作模式
  static inline Pin_Mode        m_mode = Pin_Mode::Input;

private:
  /**
   * @brief GPIO 时钟使能
   *
   */
  static void enable_clock(void) noexcept
  {
    RCC->AHB4ENR = RCC->AHB4ENR | (1U << static_cast<uint8_t>(port));
    __DSB();
  }

  /**
   * @brief GPIO 时钟失能
   *
   */
  static void disable_clock(void) noexcept
  {
    RCC->AHB4ENR = RCC->AHB4ENR & ~(1U << static_cast<uint8_t>(port));
  }

  /**
   * @brief GPIO EXIT 使能
   *
   */
  static void enable_exit(void) noexcept
  {
    SYSCFG->EXTICR[pin / 4U] = SYSCFG->EXTICR[pin / 4U] | (static_cast<uint8_t>(port) << (4U * (pin % 4U)));
  }

  /**
   * @brief GPIO EXIT 失能
   *
   */
  static void disable_exit(void) noexcept
  {
    SYSCFG->EXTICR[pin / 4U] = SYSCFG->EXTICR[pin / 4U] & ~(0xFU << (4U * (pin % 4U)));
  }

  /**
   * @brief GPIO 工作模式寄存器设置
   *
   */
  static void configure_mode(void) noexcept
  {
    if (m_mode == Pin_Mode::Input)
    {
      constexpr reg32_t mask  = 3U << (pin * 2U);
      constexpr reg32_t value = 0U << (pin * 2U);
      m_reg->MODER            = (m_reg->MODER & ~mask) | value;
    }
    else if (m_mode == Pin_Mode::Output)
    {
      constexpr reg32_t mask   = 3U << (pin * 2U);
      constexpr reg32_t value  = 1U << (pin * 2U);
      m_reg->MODER             = (m_reg->MODER & ~mask) | value;

      constexpr reg32_t mask2  = 1U << pin;
      constexpr reg32_t value2 = 0U << pin;
      m_reg->OTYPER            = (m_reg->OTYPER & ~mask2) | value2;
    }
    else if (m_mode == Pin_Mode::Output_OD)
    {
      constexpr reg32_t mask   = 3U << (pin * 2U);
      constexpr reg32_t value  = 1U << (pin * 2U);
      m_reg->MODER             = (m_reg->MODER & ~mask) | value;

      constexpr reg32_t mask2  = 1U << pin;
      constexpr reg32_t value2 = 1U << pin;
      m_reg->OTYPER            = (m_reg->OTYPER & ~mask2) | value2;
    }
    else if (m_mode == Pin_Mode::Alternate)
    {
      constexpr reg32_t mask   = 3U << (pin * 2U);
      constexpr reg32_t value  = 2U << (pin * 2U);
      m_reg->MODER             = (m_reg->MODER & ~mask) | value;

      constexpr reg32_t mask2  = 1U << pin;
      constexpr reg32_t value2 = 0U << pin;
      m_reg->OTYPER            = (m_reg->OTYPER & ~mask2) | value2;
    }
    else if (m_mode == Pin_Mode::Alternate_OD)
    {
      constexpr reg32_t mask   = 3U << (pin * 2U);
      constexpr reg32_t value  = 2U << (pin * 2U);
      m_reg->MODER             = (m_reg->MODER & ~mask) | value;

      constexpr reg32_t mask2  = 1U << pin;
      constexpr reg32_t value2 = 1U << pin;
      m_reg->OTYPER            = (m_reg->OTYPER & ~mask2) | value2;
    }
    else if (m_mode == Pin_Mode::Analog)
    {
      constexpr reg32_t mask  = 3U << (pin * 2U);
      constexpr reg32_t value = 3U << (pin * 2U);
      m_reg->MODER            = (m_reg->MODER & ~mask) | value;
    }
  }

  /**
   * @brief GPIO 工作模式寄存器清除
   *
   */
  static void cleaup_mode(void) noexcept
  {
    m_reg->MODER  = m_reg->MODER & ~(3U << (pin * 2U));
    m_reg->OTYPER = m_reg->OTYPER & ~(1U << pin);
  }

  /**
   * @brief GPIO 速度寄存器设置
   *
   */
  static void configure_speed(void) noexcept
  {
    constexpr reg32_t mask  = 3U << (pin * 2U);
    constexpr reg32_t value = static_cast<reg32_t>(speed) << (pin * 2U);
    m_reg->OSPEEDR          = (m_reg->OSPEEDR & ~mask) | value;
  }

  /**
   * @brief GPIO 速度寄存器清除
   *
   */
  static void cleanup_speed(void) noexcept
  {
    m_reg->OSPEEDR = m_reg->OSPEEDR & ~(3U << (pin * 2U));
  }

  /**
   * @brief GPIO 上下拉寄存器设置
   *
   * @param pull 上下拉设置
   */
  static void configure_pull(gpio::Pin_Pull pull) noexcept
  {
    constexpr reg32_t mask  = 3U << (pin * 2U);
    constexpr reg32_t value = static_cast<reg32_t>(pull) << (pin * 2U);
    m_reg->PUPDR            = (m_reg->PUPDR & ~mask) | value;
  }

  /**
   * @brief GPIO 上下拉寄存器清除
   *
   */
  static void cleanup_pull(void) noexcept
  {
    m_reg->PUPDR = m_reg->PUPDR & ~(3U << (pin * 2U));
  }

  /**
   * @brief GPIO 复用寄存器设置
   *
   * @param alt  复用通道
   */
  static void configure_alt(gpio::Pin_Alternate alt) noexcept
  {
    if (m_mode == Pin_Mode::Alternate || m_mode == Pin_Mode::Alternate_OD)
    {
      constexpr uint8_t afr_index = static_cast<uint8_t>(pin) / 8U;
      constexpr uint8_t afr_shift = (static_cast<uint8_t>(pin) % 8U) * 4U;
      constexpr reg32_t mask      = 0xFU << afr_shift;
      constexpr reg32_t value     = static_cast<reg32_t>(alt) << afr_shift;
      m_reg->AFR[afr_index]       = (m_reg->AFR[afr_index] & ~mask) | value;
    }
  }

  /**
   * @brief GPIO 复用寄存器清除
   *
   */
  static void cleanup_alt(void) noexcept
  {
    if (m_mode == Pin_Mode::Alternate || m_mode == Pin_Mode::Alternate_OD)
    {
      constexpr uint8_t afr_index = static_cast<uint8_t>(pin) / 8U;
      constexpr uint8_t afr_shift = (static_cast<uint8_t>(pin) % 8U) * 4U;
      constexpr reg32_t mask      = 0xFU << afr_shift;
      constexpr reg32_t value     = 0U << afr_shift;
      m_reg->AFR[afr_index]       = (m_reg->AFR[afr_index] & ~mask) | value;
    }
  }

  /**
   * @brief GPIO 边缘检测寄存器设置
   *
   * @param edge 边缘检测设置
   */
  static void configure_edge(gpio::Pin_Edge edge) noexcept
  {
    EXTI->RTSR1 = (EXTI->RTSR1 & ~(1U << pin)) | ((edge == gpio::Pin_Edge::Rising || edge == gpio::Pin_Edge::Both) ? (1U << pin) : 0U);
    EXTI->FTSR1 = (EXTI->FTSR1 & ~(1U << pin)) | ((edge == gpio::Pin_Edge::Falling || edge == gpio::Pin_Edge::Both) ? (1U << pin) : 0U);
    EXTI->IMR1  = EXTI->IMR1 | (1U << pin);
  }

  /**
   * @brief GPIO 边缘检测寄存器清除
   *
   */
  static void cleanup_edge(void) noexcept
  {
    EXTI->RTSR1 = EXTI->RTSR1 & ~(1U << pin);
    EXTI->FTSR1 = EXTI->FTSR1 & ~(1U << pin);
    EXTI->IMR1  = EXTI->IMR1 & ~(1U << pin);
  }

  /**
   * @brief GPIO 中断配置 (同步或异步)
   *
   * @param type         中断触发类型
   * @param edge         边缘检测设置
   * @param func         中断回调函数
   * @param arg          中断回调参数
   * @param priority     中断优先级
   * @param sub_priority 中断子优先级
   */
  static void configure_interrupt(interrupt::Interrupt_Type type, gpio::Pin_Edge edge, exit::Exit_Func_t func, exit::Exit_Args_t arg, uint32_t priority, uint32_t sub_priority) noexcept
  {
    enable_exit();
    configure_edge(edge);
    exit::Exit_Interrupt_Manager::get_instance().register_interrupt(static_cast<uint8_t>(pin), type, func, arg, priority, sub_priority);
  }

  /**
   * @brief GPIO 中断配置 (混合)
   *
   * @param edge         边缘检测设置
   * @param queue_func   异步中断回调函数
   * @param direct_func  同步中断回调函数
   * @param arg          中断回调参数
   * @param priority     中断优先级
   * @param sub_priority 中断子优先级
   */
  static void configure_interrupt(gpio::Pin_Edge edge, exit::Exit_Func_t queue_func, exit::Exit_Func_t direct_func, exit::Exit_Args_t arg = nullptr, uint32_t priority = 0U, uint32_t sub_priority = 0U) noexcept
  {
    enable_exit();
    configure_edge(edge);
    exit::Exit_Interrupt_Manager::get_instance().register_interrupt(static_cast<uint8_t>(pin), queue_func, direct_func, arg, priority, sub_priority);
  }

  /**
   * @brief GPIO 中断配置清除
   *
   */
  static void cleanup_interrupt(void) noexcept
  {
    disable_exit();
    cleanup_edge();
    exit::Exit_Interrupt_Manager::get_instance().unregister_interrupt(static_cast<uint8_t>(pin));
  }

protected:
  /**
   * @brief GPIO 构造函数
   *
   */
  explicit Gpio_Base() {}

  /**
   * @brief GPIO 配置
   *
   * @param mode 工作模式
   * @param pull 上下拉设置
   */
  static void setup(Pin_Mode mode, gpio::Pin_Pull pull = gpio::Pin_Pull::OFF) noexcept
  {
    m_mode = mode;
    enable_clock();
    configure_speed();
    configure_mode();
    configure_pull(pull);
  }

  /**
   * @brief GPIO 配置 (复用)
   *
   * @param alt  复用通道
   * @param mode 工作模式
   * @param pull 上下拉设置
   */
  static void setup(gpio::Pin_Alternate alt, Pin_Mode mode = Pin_Mode::Alternate_OD, gpio::Pin_Pull pull = gpio::Pin_Pull::OFF) noexcept
  {
    m_mode = mode;
    enable_clock();
    configure_speed();
    configure_mode();
    configure_alt(alt);
    configure_pull(pull);
  }

  /**
   * @brief GPIO 工作模式设置
   *
   * @param mode 工作模式
   */
  static void set_mode(Pin_Mode mode) noexcept
  {
    if (m_mode != mode)
    {
      m_mode = mode;
      configure_mode();
    }
  }

  /**
   * @brief GPIO 工作模式设置 (复用)
   *
   * @param alt  复用通道
   * @param mode 工作模式
   */
  static void set_mode(gpio::Pin_Alternate alt, Pin_Mode mode = Pin_Mode::Alternate_OD) noexcept
  {
    m_mode = mode;
    configure_mode();
    configure_alt(alt);
  }

  /**
   * @brief GPIO 上下拉设置
   *
   * @param pull 上下拉设置
   */
  static void set_pull(gpio::Pin_Pull pull) noexcept
  {
    configure_pull(pull);
  }

  /**
   * @brief GPIO 读取输入电平
   *
   * @param value 电平值
   */
  static void read(bool& value) noexcept
  {
    value = (m_reg->IDR >> (1U << pin)) != 0U;
  }

  /**
   * @brief  GPIO 读取输入电平
   *
   * @return bool 电平值
   */
  static bool read(void) noexcept
  {
    return (m_reg->IDR >> (1U << pin)) != 0U;
  }

  /**
   * @brief GPIO 输出
   *
   * @param value 电平值
   */
  static void write(bool value) noexcept
  {
    if (value)
    {
      m_reg->BSRR = (1U << pin);
    }
    else
    {
      m_reg->BSRR = (1U << (pin + 16U));
    }
  }

  /**
   * @brief GPIO 输出 高电平
   *
   */
  static void set(void) noexcept
  {
    m_reg->BSRR = (1U << pin);
  }

  /**
   * @brief GPIO 输出 低电平
   *
   */
  static void reset(void) noexcept
  {
    m_reg->BSRR = (1U << (pin + 16U));
  }

  /**
   * @brief GPIO 电平翻转
   *
   */
  static void toggle(void) noexcept
  {
    m_reg->ODR = m_reg->ODR ^ (1U << pin);
  }

  /**
   * @brief GPIO 复位
   *
   */
  static void clearup(void) noexcept
  {
    disable_clock();
    cleaup_mode();
    cleanup_speed();
    cleanup_pull();
    cleanup_alt();
    cleanup_interrupt();
    disable_exit();
  }

  /**
   * @brief  GPIO 获取端口号
   *
   * @return gpio::Pin_Port 端口号
   */
  static constexpr gpio::Pin_Port get_port(void) noexcept
  {
    return port;
  }

  /**
   * @brief  GPIO 获取引脚号
   *
   * @return Pin_Num 引脚号
   */
  static constexpr gpio::Pin_Num get_pin(void) noexcept
  {
    return pin;
  }

  /**
   * @brief  GPIO 获取速度
   *
   * @return gpio::Pin_Speed 速度
   */
  static constexpr gpio::Pin_Speed get_speed(void) noexcept
  {
    return speed;
  }

  /**
   * @brief  GPIO 获取工作模式
   *
   * @return Pin_Mode 工作模式
   */
  static Pin_Mode get_mode(void) noexcept
  {
    return m_mode;
  }

  /**
   * @brief GPIO 中断配置 (同步或异步)
   *
   * @param type         中断触发类型
   * @param edge         边缘检测设置
   * @param func         中断回调函数
   * @param arg          中断回调参数
   * @param priority     中断优先级
   * @param sub_priority 中断子优先级
   */
  static void interrupt_add(interrupt::Interrupt_Type type, gpio::Pin_Edge edge, exit::Exit_Func_t func, exit::Exit_Args_t arg = nullptr, uint32_t priority = 0U, uint32_t sub_priority = 0U) noexcept
  {
    configure_interrupt(type, edge, func, arg, priority, sub_priority);
  }

  /**
   * @brief GPIO 中断配置 (混合)
   *
   * @param edge         边缘检测设置
   * @param queue_func   异步中断回调函数
   * @param direct_func  同步中断回调函数
   * @param arg          中断回调参数
   * @param priority     中断优先级
   * @param sub_priority 中断子优先级
   */
  static void interrupt_add(gpio::Pin_Edge edge, exit::Exit_Func_t queue_func, exit::Exit_Func_t direct_func, exit::Exit_Args_t arg = nullptr, uint32_t priority = 0U, uint32_t sub_priority = 0U) noexcept
  {
    configure_interrupt(edge, queue_func, direct_func, arg, priority, sub_priority);
  }

  /**
   * @brief GPIO 中断配置清除
   *
   */
  static void interrupt_delete(void) noexcept
  {
    cleanup_interrupt();
  }

  /**
   * @brief GPIO 析构函数
   *
   */
  virtual ~Gpio_Base() {}
};
} /* namespace gpio_internal */
} /* namespace base_internal */

/// @brief 名称空间 GPIO
namespace gpio
{
/**
 * @brief  GPIO 模版类
 *
 * @tparam port   GPIO 端口号
 * @tparam pin    GPIO 引脚号
 * @tparam speed  GPIO 速度
 */
template <Pin_Port port, Pin_Num pin, Pin_Speed speed = Pin_Speed::High>
class Gpio : public base_internal::gpio_internal::Gpio_Base<port, pin, speed>
{
  // 基类类型
  using Base = base_internal::gpio_internal::Gpio_Base<port, pin, speed>;
  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(Gpio)

public:
  /**
   * @brief GPIO 构造函数
   *
   */
  explicit Gpio() {}

  /**
   * @brief GPIO 配置
   *
   * @param mode 工作模式
   * @param pull 上下拉设置
   */
  static void setup(Pin_Mode mode = Pin_Mode::Output, Pin_Pull pull = Pin_Pull::Up) noexcept
  {
    Base::setup(mode, pull);
  }

  /**
   * @brief GPIO 配置 (复用)
   *
   * @param alt  复用通道
   * @param mode 工作模式
   * @param pull 上下拉设置
   */
  static void setup(Pin_Alternate alt, Pin_Mode mode = Pin_Mode::Alternate_OD, Pin_Pull pull = Pin_Pull::OFF) noexcept
  {
    Base::setup(alt, mode, pull);
  }

  /**
   * @brief GPIO 工作模式设置
   *
   * @param mode 工作模式
   */
  static void set_mode(Pin_Mode mode) noexcept
  {
    Base::set_mode(mode);
  }

  /**
   * @brief GPIO 工作模式设置 (复用)
   *
   * @param alt  复用通道
   * @param mode 工作模式
   */
  static void set_mode(Pin_Alternate alt, Pin_Mode mode = Pin_Mode::Alternate_OD) noexcept
  {
    Base::set_mode(alt, mode);
  }

  /**
   * @brief GPIO 上下拉设置
   *
   * @param pull 上下拉设置
   */
  static void set_pull(Pin_Pull pull) noexcept
  {
    Base::set_pull(pull);
  }

  /**
   * @brief GPIO 读取输入电平
   *
   * @param value 电平值
   */
  static void read(bool& value) noexcept
  {
    value = Base::read();
  }

  /**
   * @brief  GPIO 读取输入电平
   *
   * @return bool 电平值
   */
  static bool read(void) noexcept
  {
    return Base::read();
  }

  /**
   * @brief GPIO 输出
   *
   * @param value 电平值
   */
  static void write(bool value) noexcept
  {
    Base::write(value);
  }

  /**
   * @brief GPIO 输出 高电平
   *
   */
  static void set(void) noexcept
  {
    Base::write(true);
  }

  /**
   * @brief GPIO 输出 高电平
   *
   */
  static void high(void) noexcept
  {
    Base::write(true);
  }

  /**
   * @brief GPIO 输出 低电平
   *
   */
  static void reset(void) noexcept
  {
    Base::write(false);
  }

  /**
   * @brief GPIO 输出 低电平
   *
   */
  static void low(void) noexcept
  {
    Base::write(false);
  }

  /**
   * @brief GPIO 电平翻转
   *
   */
  static void toggle(void) noexcept
  {
    Base::toggle();
  }

  /**
   * @brief GPIO 复位
   *
   */
  static void clearup(void) noexcept
  {
    Base::clearup();
  }

  /**
   * @brief  GPIO 获取端口号
   *
   * @return Pin_Port 端口号
   */
  static constexpr Pin_Port get_port(void) noexcept
  {
    return Base::get_port();
  }

  /**
   * @brief  GPIO 获取引脚号
   *
   * @return Pin_Num 引脚号
   */
  static constexpr Pin_Num get_pin(void) noexcept
  {
    return Base::get_pin();
  }

  /**
   * @brief  GPIO 获取速度
   *
   * @return Pin_Speed 速度
   */
  static constexpr Pin_Speed get_speed(void) noexcept
  {
    return Base::get_speed();
  }

  /**
   * @brief  GPIO 获取工作模式
   *
   * @return Pin_Mode 工作模式
   */
  static Pin_Mode get_mode(void) noexcept
  {
    return Base::get_mode();
  }

  /**
   * @brief GPIO 中断配置 (同步或异步)
   *
   * @param type         中断触发类型
   * @param edge         边缘检测设置
   * @param func         中断回调函数
   * @param arg          中断回调参数
   * @param priority     中断优先级
   * @param sub_priority 中断子优先级
   */
  static void interrupt_add(interrupt::Interrupt_Type type, Pin_Edge edge, exit::Exit_Func_t func, exit::Exit_Args_t arg = nullptr, uint32_t priority = 0U, uint32_t sub_priority = 0U) noexcept
  {
    Base::interrupt_add(type, edge, func, arg, priority, sub_priority);
  }

  /**
   * @brief GPIO 中断配置 (混合)
   *
   * @param edge         边缘检测设置
   * @param queue_func   异步中断回调函数
   * @param direct_func  同步中断回调函数
   * @param arg          中断回调参数
   * @param priority     中断优先级
   * @param sub_priority 中断子优先级
   */
  static void interrupt_add(Pin_Edge edge, exit::Exit_Func_t queue_func, exit::Exit_Func_t direct_func, exit::Exit_Args_t arg = nullptr, uint32_t priority = 0U, uint32_t sub_priority = 0U) noexcept
  {
    Base::interrupt_add(edge, queue_func, direct_func, arg, priority, sub_priority);
  }

  /**
   * @brief GPIO 中断配置清除
   *
   */
  static void interrupt_delete(void) noexcept
  {
    Base::interrupt_delete();
  }

  /**
   * @brief GPIO 析构函数
   *
   */
  virtual ~Gpio() {}
};
} /* namespace gpio */
} /* namespace base */
} /* namespace QAQ */

#endif /* __GPIO_HPP__ */
