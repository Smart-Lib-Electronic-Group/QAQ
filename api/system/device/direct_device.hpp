#ifndef __DIRECT_DEVICE_HPP__
#define __DIRECT_DEVICE_HPP__

#include "direct_device_base.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 系统
namespace system
{
/// @brief 名称空间 设备
namespace device
{
class Direct_Device : public system_internal::device_internal::Direct_Device_Base
{
  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(Direct_Device)

private:
  /// @brief 设备事件标志位
  using Bits = system_internal::device_internal::Device_Event_Bits;

protected:
  /**
   * @brief  直接传输设备 设备数据接收元方法 - 纯虚函数
   *
   * @param  data               数据指针
   * @param  size               数据大小
   * @return uint32_t           实际接收数据大小
   */
  virtual uint32_t recv_impl(uint8_t* data, uint32_t size)       = 0;

  /**
   * @brief  直接传输设备 设备数据发送元方法 - 纯虚函数
   *
   * @param  data               数据指针
   * @param  size               数据大小
   * @return uint32_t           实际发送数据大小
   */
  virtual uint32_t send_impl(const uint8_t* data, uint32_t size) = 0;

  /**
   * @brief  直接传输设备 输入完成
   */
  void input_complete(void)
  {
    m_event_flags.clear(static_cast<uint32_t>(Bits::Enable_Receive));
    m_event_flags.set(static_cast<uint32_t>(Bits::Receive_Finish));
  }

  /**
   * @brief  直接传输设备 输出完成
   */
  void output_complete(void)
  {
    m_event_flags.clear(static_cast<uint32_t>(Bits::Enable_Transfer));
    m_event_flags.set(static_cast<uint32_t>(Bits::Transmit_Finish));
  }

  /**
   * @brief  直接传输设备 构造函数
   */
  explicit Direct_Device() {}

  /**
   * @brief  直接传输设备 析构函数
   */
  virtual ~Direct_Device() {}

public:
  /**
   * @brief  直接传输设备 读取数据
   *
   * @param  data               数据指针
   * @param  size               数据大小
   * @param  timeout_ms         超时时间
   * @return int64_t            实际读取数据大小
   */
  int64_t read(void* data, uint32_t size, uint32_t timeout_ms = TX_WAIT_FOREVER) override
  {
    int64_t  ret          = 0;
    uint32_t event_bits   = 0;
    bool     need_receive = false;
    uint8_t* data_ptr     = reinterpret_cast<uint8_t*>(data);

    if (!m_opened)
    {
      ret = -1;
    }
    else if (0 != size)
    {
      if (m_event_flags.wait(static_cast<uint32_t>(Bits::Enable_Receive), 0))
      {
        event_bits = m_event_flags.wait((static_cast<uint32_t>(Bits::Receive_Finish) | static_cast<uint32_t>(Bits::Close)), timeout_ms);
        if (static_cast<uint32_t>(Bits::Receive_Finish) & event_bits)
        {
          need_receive = true;
        }
      }
      else
      {
        need_receive = true;
      }

      if (m_opened && need_receive)
      {
        m_event_flags.set(static_cast<uint32_t>(Bits::Enable_Receive));
        m_event_flags.clear(static_cast<uint32_t>(Bits::Receive_Finish));

        ret = recv_impl(data_ptr, size);

        if (ret > 0)
        {
          event_bits = m_event_flags.wait((static_cast<uint32_t>(Bits::Receive_Finish) | static_cast<uint32_t>(Bits::Close)), timeout_ms);
          if (event_bits & static_cast<uint32_t>(Bits::Receive_Finish))
          {
            m_event_flags.clear(static_cast<uint32_t>(Bits::Receive_Finish));
          }
          else if (0 != timeout_ms)
          {
            post_event(static_cast<uint32_t>(Bits::Receive_Timeout));
          }
        }
      }
    }

    return ret;
  }

  /**
   * @brief  直接传输设备 写入数据
   *
   * @param  data               数据指针
   * @param  size               数据大小
   * @param  timeout_ms         超时时间
   * @return int64_t            实际写入数据大小
   */
  int64_t write(const void* data, uint32_t size, uint32_t timeout_ms = 0) override
  {
    int64_t        ret           = 0;
    uint32_t       event_bits    = 0;
    bool           need_transfer = false;
    const uint8_t* data_ptr      = reinterpret_cast<const uint8_t*>(data);

    if (!m_opened)
    {
      ret = -1;
    }
    else if (0 != size)
    {
      if (m_event_flags.wait(static_cast<uint32_t>(Bits::Enable_Transfer), 0))
      {
        event_bits = m_event_flags.wait((static_cast<uint32_t>(Bits::Transmit_Finish) | static_cast<uint32_t>(Bits::Close)), timeout_ms);
        if (static_cast<uint32_t>(Bits::Transmit_Finish) & event_bits)
        {
          need_transfer = true;
        }
      }
      else
      {
        need_transfer = true;
      }

      if (m_opened && need_transfer)
      {
        m_event_flags.set(static_cast<uint32_t>(Bits::Enable_Transfer));
        m_event_flags.clear(static_cast<uint32_t>(Bits::Transmit_Finish));

        ret = send_impl(data_ptr, size);

        if (ret > 0)
        {
          event_bits = m_event_flags.wait((static_cast<uint32_t>(Bits::Transmit_Finish) | static_cast<uint32_t>(Bits::Close)), timeout_ms);
          if (event_bits & static_cast<uint32_t>(Bits::Transmit_Finish))
          {
            m_event_flags.clear(static_cast<uint32_t>(Bits::Transmit_Finish));
          }
        }
      }
    }

    return ret;
  }

  /**
   * @brief  直接传输设备 刷新数据
   *
   * @param  timeout_ms         超时时间
   * @return Device_Error_Code  错误码
   */
  Device_Error_Code flush(uint32_t timeout_ms = TX_WAIT_FOREVER) override
  {
    Device_Error_Code error_code = Device_Error_Code::OK;
    uint32_t          event_bits = 0;

    if (!m_opened)
    {
      error_code = Device_Error_Code::NOT_OPENED;
    }
    else
    {
      if (m_event_flags.wait(static_cast<uint32_t>(Bits::Enable_Transfer), 0))
      {
        event_bits = m_event_flags.wait((static_cast<uint32_t>(Bits::Transmit_Finish) | static_cast<uint32_t>(Bits::Close)), timeout_ms);

        if (static_cast<uint32_t>(Bits::Transmit_Finish) & event_bits)
        {
          m_event_flags.clear(static_cast<uint32_t>(Bits::Transmit_Finish));
        }
        else if (static_cast<uint32_t>(Bits::Close) & event_bits)
        {
          error_code = Device_Error_Code::CLOSED;
        }
        else
        {
          error_code = Device_Error_Code::TIMEOUT;
        }
      }
    }

    return error_code;
  }
};
} /* namespace device */
} /* namespace system */
} /* namespace QAQ */

#endif /* __DIRECT_DEVICE_HPP__ */
