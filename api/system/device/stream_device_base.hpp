#ifndef __STREAM_DEVICE_BASE_HPP__
#define __STREAM_DEVICE_BASE_HPP__

#include "device_manger.hpp"

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
/**
 * @brief 输入设备基类
 *
 */
class InDevice_Base : public Device_Base, public Input_Base
{
  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(InDevice_Base)

protected:
  /**
   * @brief  输入设备基类 发送事件
   *
   * @param  event 事件标志
   * @return true  发送成功
   * @return false 发送失败
   */
  bool post_event(uint32_t event) override
  {
    return Device_Manager::instance().post_event(this, event);
  }

  /**
   * @brief  输入设备基类 构造函数
   */
  explicit InDevice_Base() {}

  /**
   * @brief  输入设备基类 析构函数
   */
  virtual ~InDevice_Base() {}

public:
  /**
   * @brief  输入设备基类 获取缓存区中的数据数量 - 纯虚函数
   *
   * @return uint32_t 缓存区中的数据数量
   */
  virtual uint32_t available(void) const                                      = 0;

  /**
   * @brief  输入设备基类 缓存区是否为空 - 纯虚函数
   *
   * @return true  缓存区空
   * @return false 缓存区不为空
   */
  virtual bool empty(void) const                                              = 0;

  /**
   * @brief  输入设备基类 缓存区是否为满 - 纯虚函数
   *
   * @return true  缓存区满
   * @return false 缓存区不为满
   */
  virtual bool full(void) const                                               = 0;

  /**
   * @brief  输入设备基类 清空缓存区 - 纯虚函数
   *
   * @return Device_Error_Code 设备错误码
   */
  virtual device::Device_Error_Code clear(void)                               = 0;

  /**
   * @brief  输入设备基类 探视数据 - 纯虚函数
   *
   * @param  data       探视数据指针
   * @param  request    探视数据大小
   * @param  timeout_ms 探视超时时间
   * @return int64_t    实际探视数据大小
   */
  virtual int64_t peek(void* data, uint32_t request, uint32_t timeout_ms = 0) = 0;

  /**
   * @brief  输入设备基类 回退数据 - 纯虚函数
   */
  virtual void roll_back(void)                                                = 0;

  /**
   * @brief  输入设备基类 等待输入完成
   *
   * @param  timeout_ms         超时时间 - 毫秒
   * @return Device_Error_Code  错误码
   */
  device::Device_Error_Code wait(uint32_t timeout_ms = TX_WAIT_FOREVER)
  {
    device::Device_Error_Code error_code = device::Device_Error_Code::OK;
    uint32_t                  event_bits = 0;

    if (!m_opened)
    {
      error_code = device::Device_Error_Code::NOT_OPENED;
    }
    else
    {
      if (empty())
      {
        event_bits = m_event_flags.wait((static_cast<uint32_t>(Device_Event_Bits::Receive_Finish) | static_cast<uint32_t>(Device_Event_Bits::Close)), timeout_ms);
        if (event_bits & static_cast<uint32_t>(Device_Event_Bits::Close))
        {
          error_code = device::Device_Error_Code::CLOSED;
        }
        else if (event_bits & static_cast<uint32_t>(Device_Event_Bits::Receive_Finish))
        {
          error_code = device::Device_Error_Code::OK;
        }
        else
        {
          error_code = device::Device_Error_Code::TIMEOUT;
        }
      }
    }

    return error_code;
  }

  /**
   * @brief  输入设备基类 获取设备类型
   *
   * @return Device_Type 设备类型
   */
  device::Device_Type get_type(void) const override
  {
    return device::Device_Type::STREAMING;
  }

  /**
   * @brief  输入设备基类 获取设备类型
   *
   * @return Device_Type 设备类型
   */
  static constexpr device::Device_Type type() noexcept
  {
    return device::Device_Type::STREAMING;
  }

  /**
   * @brief  输入设备基类 获取流类型
   *
   * @return Stream_Type 流类型
   */
  device::Stream_Type get_stream_type(void) const
  {
    return device::Stream_Type::READ_ONLY;
  }

  /**
   * @brief  输入设备基类 获取流类型
   *
   * @return Stream_Type 流类型
   */
  static constexpr device::Stream_Type stream_type() noexcept
  {
    return device::Stream_Type::READ_ONLY;
  }
};

/**
 * @brief 输出设备基类
 *
 */
class OutDevice_Base : public Device_Base, public Output_Base
{
  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(OutDevice_Base)

protected:
  /**
   * @brief  输出设备基类 发送事件
   *
   * @param  event 事件标志
   * @return true  发送成功
   * @return false 发送失败
   */
  bool post_event(uint32_t event) override
  {
    return Device_Manager::instance().post_event(this, event);
  }

  /**
   * @brief  输出设备基类 构造函数
   */
  explicit OutDevice_Base() {}

  /**
   * @brief  输出设备基类 析构函数
   */
  virtual ~OutDevice_Base() {}

public:
  /**
   * @brief  输出设备基类 获取设备类型
   *
   * @return Device_Type 设备类型
   */
  device::Device_Type get_type(void) const override
  {
    return device::Device_Type::STREAMING;
  }

  /**
   * @brief  输出设备基类 获取设备类型
   *
   * @return Device_Type 设备类型
   */
  static constexpr device::Device_Type type() noexcept
  {
    return device::Device_Type::STREAMING;
  }

  /**
   * @brief  输出设备基类 获取流类型
   *
   * @return Stream_Type 流类型
   */
  device::Stream_Type get_stream_type(void) const
  {
    return device::Stream_Type::WRITE_ONLY;
  }

  /**
   * @brief  输出设备基类 获取流类型
   *
   * @return Stream_Type 流类型
   */
  static constexpr device::Stream_Type stream_type() noexcept
  {
    return device::Stream_Type::WRITE_ONLY;
  }
};

/**
 * @brief 输入输出设备基类
 *
 */
class IODevice_Base : public Device_Base, public Input_Base, public Output_Base
{
  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(IODevice_Base)

protected:
  /**
   * @brief  输入输出设备基类 发送事件
   *
   * @param  event 事件标志
   * @return true  发送成功
   * @return false 发送失败
   */
  bool post_event(uint32_t event) override
  {
    return Device_Manager::instance().post_event(this, event);
  }

  /**
   * @brief  输入输出设备基类 构造函数
   */
  explicit IODevice_Base() {}

  /**
   * @brief  输入输出设备基类 析构函数
   */
  virtual ~IODevice_Base() {}

public:
  /**
   * @brief  输入输出设备基类 获取缓存区中的数据数量 - 纯虚函数
   *
   * @return uint32_t 缓存区中的数据数量
   */
  virtual uint32_t available(void) const                                      = 0;

  /**
   * @brief  输入输出设备基类 缓存区是否为空 - 纯虚函数
   *
   * @return true  缓存区空
   * @return false 缓存区不为空
   */
  virtual bool empty(void) const                                              = 0;

  /**
   * @brief  输入输出设备基类 缓存区是否为满 - 纯虚函数
   *
   * @return true  缓存区满
   * @return false 缓存区不为满
   */
  virtual bool full(void) const                                               = 0;

  /**
   * @brief  输入输出设备基类 清空缓存区 - 纯虚函数
   *
   * @return Device_Error_Code 设备错误码
   */
  virtual device::Device_Error_Code clear(void)                               = 0;

  /**
   * @brief  输入输出设备基类 探视数据 - 纯虚函数
   *
   * @param  data       探视数据指针
   * @param  request    探视数据大小
   * @param  timeout_ms 探视超时时间
   * @return int64_t    实际探视数据大小
   */
  virtual int64_t peek(void* data, uint32_t request, uint32_t timeout_ms = 0) = 0;

  /**
   * @brief  输入输出设备基类 回退数据 - 纯虚函数
   */
  virtual void roll_back(void)                                                = 0;

  /**
   * @brief  输入输出设备基类 等待输入完成
   *
   * @param  timeout_ms         超时时间 - 毫秒
   * @return Device_Error_Code  错误码
   */
  device::Device_Error_Code wait(uint32_t timeout_ms = TX_WAIT_FOREVER)
  {
    device::Device_Error_Code error_code = device::Device_Error_Code::OK;
    uint32_t                  event_bits = 0;

    if (!m_opened)
    {
      error_code = device::Device_Error_Code::NOT_OPENED;
    }
    else
    {
      if (empty())
      {
        event_bits = m_event_flags.wait((static_cast<uint32_t>(Device_Event_Bits::Receive_Finish) | static_cast<uint32_t>(Device_Event_Bits::Close)), timeout_ms);
        if (event_bits & static_cast<uint32_t>(Device_Event_Bits::Close))
        {
          error_code = device::Device_Error_Code::CLOSED;
        }
        else if (event_bits & static_cast<uint32_t>(Device_Event_Bits::Receive_Finish))
        {
          error_code = device::Device_Error_Code::OK;
        }
        else
        {
          error_code = device::Device_Error_Code::TIMEOUT;
        }
      }
    }

    return error_code;
  }

  /**
   * @brief  输入输出设备基类 获取设备类型
   *
   * @return Device_Type 设备类型
   */
  device::Device_Type get_type(void) const override
  {
    return device::Device_Type::STREAMING;
  }

  /**
   * @brief  输入输出设备基类 获取设备类型
   *
   * @return Device_Type 设备类型
   */
  static constexpr device::Device_Type type() noexcept
  {
    return device::Device_Type::STREAMING;
  }

  /**
   * @brief  输入输出设备基类 获取流类型
   *
   * @return Stream_Type 流类型
   */
  device::Stream_Type get_stream_type(void) const
  {
    return device::Stream_Type::READ_WRITE;
  }

  /**
   * @brief  输入输出设备基类 获取流类型
   *
   * @return Stream_Type 流类型
   */
  static constexpr device::Stream_Type stream_type() noexcept
  {
    return device::Stream_Type::READ_WRITE;
  }
};
} /* namespace device_internal */
} /* namespace system_internal */
} /* namespace system */
} /* namespace QAQ */

#endif /* __STREAM_DEVICE_BASE_HPP__ */
