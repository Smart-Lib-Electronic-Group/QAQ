#ifndef __DEVICE_BASE_HPP__
#define __DEVICE_BASE_HPP__

#include "event_flags.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 系统
namespace system
{
/// @brief 名称空间 设备
namespace device
{
/// @brief 设备错误码
enum class Device_Error_Code : uint8_t
{
  OK,                /* 成功 */
  TIMEOUT,           /* 超时 */
  BUSY,              /* 繁忙 */
  INVALID_PARAMETER, /* 参数无效 */
  NOT_OPENED,        /* 未打开 */
  CLOSED,            /* 已关闭 */
  IO_ERROR,          /* 输入输出错误 */
  INIT_FAILED,       /* 初始化失败 */
  DEINIT_FAILED,     /* 解除初始化失败 */
  PORT_ALREADY_USED, /* 端口已被使用 */
  WAIT_FOR_CONNECT,  /* 等待连接 */
};

/// @brief 设备类型
enum class Device_Type : uint8_t
{
  STREAMING, /* 流设备 */
  STORAGE,   /* 存储设备 */
  DIRECTORY, /* 直接传输设备 */
};

/// @brief 流设备类型
enum class Stream_Type : uint8_t
{
  READ_ONLY,  /* 只读 */
  WRITE_ONLY, /* 只写 */
  READ_WRITE, /* 读写 */
};
} /* namespace device */

/// @brief 命名空间 内部
namespace system_internal
{
/// @brief 名称空间 设备 内部
namespace device_internal
{
class Device_Manager;

/// @brief 回调函数参数类型
using Device_Args_t = void*;
/// @brief 回调函数类型
using Device_Func_t = void (*)(Device_Args_t);

/// @brief 设备事件标志位
enum class Device_Event_Bits : uint32_t
{
  Enable_Transfer = 0x01, /* 使能传输 */
  Transmit_Finish = 0x02, /* 传输完成 */
  Enable_Receive  = 0x04, /* 使能接收 */
  Receive_Finish  = 0x08, /* 接收完成 */
  Receive_Timeout = 0x10, /* 接收超时 */
  Close           = 0x20, /* 关闭 */
  Error           = 0x40, /* 错误 */
  All             = 0xFF, /* 所有事件 */
};

/**
 * @brief 设备基类
 *
 */
class Device_Base
{
  /// @brief 友元声明 设备管理器
  friend class Device_Manager;

protected:
  /// @brief 事件标志组
  kernel::Event_Flags m_event_flags;
  /// @brief 开关状态标志
  std::atomic_bool    m_opened                                                  = false;

  /**
   * @brief  设备基类 设备管理器事件处理句柄 - 纯虚函数
   *
   * @param  event 事件标志
   */
  virtual void manger_handler(uint32_t event)                                   = 0;

  /**
   * @brief  设备基类 发送事件 - 纯虚函数
   *
   * @param  event 事件标志
   * @return true  发送成功
   * @return false 发送失败
   */
  virtual bool post_event(uint32_t event)                                       = 0;

  /**
   * @brief  设备基类 设备开启元方法 - 纯虚函数
   *
   * @return Device_Error_Code 设备错误码
   */
  virtual device::Device_Error_Code open_impl(void)                             = 0;

  /**
   * @brief  设备基类 设备关闭元方法 - 纯虚函数
   *
   * @return Device_Error_Code 设备错误码
   */
  virtual device::Device_Error_Code close_impl(void)                            = 0;

  /**
   * @brief  设备基类 配置设备元方法 - 纯虚函数
   *
   * @param  param              配置参数
   * @param  value              配置值
   * @return Device_Error_Code  设备错误码
   */
  virtual device::Device_Error_Code config_impl(uint32_t param, uint32_t value) = 0;

  /**
   * @brief  设备基类 获取设备配置元方法 - 纯虚函数
   *
   * @param  param              配置参数
   * @return const uint32_t     配置值
   */
  virtual uint32_t get_config_impl(uint32_t param) const                        = 0;

public:
  /**
   * @brief  设备基类 打开设备
   *
   * @return Device_Error_Code 错误码
   */
  device::Device_Error_Code open(void)
  {
    device::Device_Error_Code error_code = device::Device_Error_Code::OK;

    if (m_opened)
    {
      error_code = device::Device_Error_Code::BUSY;
    }
    else
    {
      error_code = open_impl();
      if (device::Device_Error_Code::OK == error_code)
      {
        m_opened = true;
        m_event_flags.clear(static_cast<uint32_t>(Device_Event_Bits::All));
        post_event(static_cast<uint32_t>(Device_Event_Bits::Enable_Receive));
      }
    }

    return error_code;
  }

  /**
   * @brief  设备基类 关闭设备
   *
   * @return Device_Error_Code 错误码
   */
  device::Device_Error_Code close(void)
  {
    device::Device_Error_Code error_code = device::Device_Error_Code::OK;

    if (!m_opened)
    {
      error_code = device::Device_Error_Code::NOT_OPENED;
    }
    else
    {
      error_code = close_impl();
      if (device::Device_Error_Code::OK == error_code)
      {
        m_opened = false;
        m_event_flags.set(static_cast<uint32_t>(Device_Event_Bits::Close));
      }
    }

    return error_code;
  }

  /**
   * @brief  设备基类 配置设备
   *
   * @param  param              配置参数
   * @param  value              配置值
   * @return Device_Error_Code  错误码
   */
  device::Device_Error_Code config(uint32_t param, uint32_t value)
  {
    device::Device_Error_Code error_code = device::Device_Error_Code::OK;

    if (!m_opened)
    {
      error_code = device::Device_Error_Code::NOT_OPENED;
    }
    else
    {
      error_code = config_impl(param, value);
    }

    return error_code;
  }

  /**
   * @brief  设备基类 获取配置参数
   *
   * @param  param              配置参数
   * @return uint32_t           配置值
   */
  uint32_t get_config(uint32_t param) const
  {
    uint32_t ret_value = 0;

    if (m_opened)
    {
      ret_value = get_config_impl(param);
    }

    return ret_value;
  }

  /**
   * @brief  设备基类 获取设备类型 - 纯虚函数
   *
   * @return Device_Type 设备类型
   */
  virtual device::Device_Type get_type(void) const = 0;

  /**
   * @brief  设备基类 设备是否打开 - 纯虚函数
   *
   * @return true 设备已打开
   * @return false 设备未打开
   */
  bool is_opened(void) const
  {
    return m_opened;
  }
};

/**
 * @brief 输入基类
 *
 */
class Input_Base
{
public:
  /**
   * @brief  输入基类 读取数据 - 纯虚函数
   *
   * @param  data       读取数据地址
   * @param  size       读取数据大小
   * @param  timeout_ms 读取数据超时时间
   * @return int64_t    实际读取数据大小
   */
  virtual int64_t read(void* data, uint32_t size, uint32_t timeout_ms = TX_WAIT_FOREVER) = 0;
};

/**
 * @brief 输出基类
 *
 */
class Output_Base
{
public:
  /**
   * @brief  输出基类 写入数据 - 纯虚函数
   *
   * @param  data       写入数据地址
   * @param  size       写入数据大小
   * @param  timeout_ms 写入数据超时时间
   * @return int64_t    实际写入数据大小
   */
  virtual int64_t write(const void* data, uint32_t size, uint32_t timeout_ms = 0) = 0;

  /**
   * @brief  输出基类 刷新数据 - 纯虚函数
   *
   * @param  timeout_ms         刷新数据超时时间
   * @return Device_Error_Code  设备错误码
   */
  virtual device::Device_Error_Code flush(uint32_t timeout_ms = TX_WAIT_FOREVER)  = 0;
};
} /* namespace device_internal */
} /* namespace system_internal */
} /* namespace system */
} /* namespace QAQ */

#endif /* __DEVICE_BASE_HPP__ */
