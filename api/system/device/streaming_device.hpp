#ifndef __STREAMING_DEVICE_HPP__
#define __STREAMING_DEVICE_HPP__

#include "stream_device_base.hpp"
#include "ring_buffer.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 系统
namespace system
{
/// @brief 名称空间 设备
namespace device
{
/**
 * @brief  流设备模版类
 *
 * @tparam Type          流设备类型
 * @tparam In_Buf_Size   输入缓存区大小
 * @tparam Out_Buf_Size  输出缓存区大小
 * @tparam In_Buf_Mode   输入缓存区模式
 */
template <Stream_Type Type, uint32_t In_Buf_Size, uint32_t Out_Buf_Size, memory::Ring_Buffer_Mode In_Buf_Mode = memory::Ring_Buffer_Mode::INPUT_BYTES>
class Stream_Device;

/**
 * @brief  流设备模版类 只读模式特化
 *
 * @tparam In_Buf_Size  输入缓存区大小
 * @tparam Out_Buf_Size 输出缓存区大小
 * @tparam In_Buf_Mode  输入缓存区模式
 */
template <uint32_t In_Buf_Size, uint32_t Out_Buf_Size, memory::Ring_Buffer_Mode In_Buf_Mode>
class Stream_Device<Stream_Type::READ_ONLY, In_Buf_Size, Out_Buf_Size, In_Buf_Mode> : public system_internal::device_internal::InDevice_Base
{
  /// @warning 输入缓存区大小必须大于0
  static_assert(In_Buf_Size > 0, "Input buffer size must be greater than 0");
  /// @warning 输出缓存区大小必须为0
  static_assert(Out_Buf_Size == 0, "Output buffer size must be 0");

  // 禁止拷贝和移动
  NO_COPY_MOVE(Stream_Device)

private:
  /// @brief 设备事件标志位
  using Bits = system_internal::device_internal::Device_Event_Bits;

protected:
  /// @brief 输入缓存区
  memory::Ring_Buffer<uint8_t, In_Buf_Size, In_Buf_Mode> m_input_buffer;

  /**
   * @brief  流设备 推送数据进入缓存区
   *
   * @param  data               数据
   * @note   该方法仅在输入缓存区模式为INPUT_BYTES时有效
   */
  template <bool enable = (memory::Ring_Buffer_Mode::INPUT_BYTES == In_Buf_Mode), typename = std::enable_if_t<enable>>
  void input_buffer_push(const uint8_t data)
  {
    m_input_buffer.push(data);
  }

  /**
   * @brief  流设备 输入完成
   *
   * @note   该方法仅在输入缓存区模式为INPUT_BYTES时有效
   */
  template <bool enable = (memory::Ring_Buffer_Mode::INPUT_BYTES == In_Buf_Mode), typename = std::enable_if_t<enable>>
  void input_complete(void)
  {
    m_event_flags.set(static_cast<uint32_t>(Bits::Receive_Finish));
  }

  /**
   * @brief  流设备 获取输入缓存区指针
   *
   * @param  size     允许输入的最大数据大小 - 引用返回
   * @return uint8_t* 数据缓存区指针
   * @note   该方法仅在输入缓存区模式为INPUT_SINGLE_BUFFER时有效
   */
  template <bool enable = (memory::Ring_Buffer_Mode::INPUT_SINGLE_BUFFER == In_Buf_Mode), typename = std::enable_if_t<enable>>
  uint8_t* input_buffer_ptr(uint32_t& size)
  {
    return m_input_buffer.input_start(size);
  }

  /**
   * @brief  流设备 输入完成
   *
   * @param  size     成功输入的数据大小
   * @note   该方法仅在输入缓存区模式为INPUT_SINGLE_BUFFER或INPUT_DOUBLE_BUFFER时有效
   */
  template <bool enable = (memory::Ring_Buffer_Mode::INPUT_SINGLE_BUFFER == In_Buf_Mode || memory::Ring_Buffer_Mode::INPUT_DOUBLE_BUFFER == In_Buf_Mode), typename = std::enable_if_t<enable>>
  void input_complete(uint32_t size)
  {
    m_input_buffer.input_complete(size);
    if (size)
    {
      m_event_flags.set(static_cast<uint32_t>(Bits::Receive_Finish));
    }
  }

  /**
   * @brief  流设备 获取输入缓存区指针
   *
   * @param  ptr1     数据缓存区1指针
   * @param  ptr2     数据缓存区2指针
   * @return uint32_t 成功输入的数据大小
   * @note   该方法仅在输入缓存区模式为INPUT_DOUBLE_BUFFER时有效
   */
  template <bool enable = (memory::Ring_Buffer_Mode::INPUT_DOUBLE_BUFFER == In_Buf_Mode), typename = std::enable_if_t<enable>>
  uint32_t input_buffer_ptr(uint8_t*& ptr1, uint8_t*& ptr2)
  {
    return m_input_buffer.input_start(ptr1, ptr2);
  }

  /**
   * @brief  流设备 输入缓存区切换内存
   *
   * @note   该方法仅在输入缓存区模式为INPUT_DOUBLE_BUFFER时有效
   */
  template <bool enable = (memory::Ring_Buffer_Mode::INPUT_DOUBLE_BUFFER == In_Buf_Mode), typename = std::enable_if_t<enable>>
  void memory_switch(void)
  {
    m_input_buffer.switch_buffer();
    m_event_flags.set(static_cast<uint32_t>(Bits::Receive_Finish));
  }

  /**
   * @brief  流设备 构造函数
   */
  explicit Stream_Device() {}

  /**
   * @brief  流设备 析构函数
   */
  virtual ~Stream_Device() {}

public:
  /**
   * @brief  流设备 获取输入缓存区大小
   *
   * @return uint32_t 输入缓存区大小
   */
  static constexpr uint32_t input_buffer_size()
  {
    return In_Buf_Size;
  }

  /**
   * @brief  流设备 获取输出缓存区大小
   *
   * @return uint32_t 输出缓存区大小
   */
  static constexpr uint32_t output_buffer_size()
  {
    return 0;
  }

  /**
   * @brief  流设备 读取数据
   *
   * @param  data               数据缓存区指针
   * @param  size               数据大小
   * @param  timeout_ms         超时时间 - 毫秒
   * @return int64_t            读取数据大小
   */
  int64_t read(void* data, uint32_t size, uint32_t timeout_ms = TX_WAIT_FOREVER) override
  {
    int64_t  ret      = 0;
    uint8_t* data_ptr = reinterpret_cast<uint8_t*>(data);

    if (!m_opened)
    {
      ret = -1;
    }
    else
    {
      uint32_t read_bytes = 0;
      uint32_t event_bits = 0;
      while (read_bytes < size && m_opened)
      {
        if (m_input_buffer.empty())
        {
          event_bits = m_event_flags.wait((static_cast<uint32_t>(Bits::Receive_Finish) | static_cast<uint32_t>(Bits::Close)), timeout_ms);
          if ((0 == event_bits) || (event_bits & static_cast<uint32_t>(Bits::Close)))
          {
            break;
          }
        }

        read_bytes += m_input_buffer.read(data_ptr + read_bytes, std::min(size - read_bytes, m_input_buffer.available()));

        if (m_input_buffer.empty())
        {
          m_event_flags.clear(static_cast<uint32_t>(Bits::Receive_Finish));
        }
      }
      ret = read_bytes;
    }

    return ret;
  }

  /**
   * @brief  流设备 获取输入缓存区当前存在的数据数量
   *
   * @return uint32_t           数据大小
   */
  uint32_t available(void) const override
  {
    uint32_t ret = 0;

    if (m_opened)
    {
      ret = m_input_buffer.available();
    }

    return ret;
  }

  /**
   * @brief  流设备 输入缓存区当前是否为空
   *
   * @return true               为空
   * @return false              不为空
   */
  bool empty(void) const override
  {
    bool ret = true;

    if (m_opened)
    {
      ret = m_input_buffer.empty();
    }

    return ret;
  }

  /**
   * @brief  流设备 输入缓存区当前是否已满
   *
   * @return true               已满
   * @return false              未满
   */
  bool full(void) const override
  {
    bool ret = false;

    if (m_opened)
    {
      ret = m_input_buffer.full();
    }

    return ret;
  }

  /**
   * @brief  流设备 清空输入缓存区数据
   *
   * @return Device_Error_Code 错误码
   */
  Device_Error_Code clear(void) override
  {
    Device_Error_Code error_code = Device_Error_Code::OK;

    if (!m_opened)
    {
      error_code = Device_Error_Code::NOT_OPENED;
    }
    else
    {
      m_input_buffer.clear();
    }

    return error_code;
  }

  /**
   * @brief  流设备 探视输入缓存区数据
   *
   * @param  data               数据缓存区指针
   * @param  request            请求数据大小
   * @param  timeout_ms         超时时间 - 毫秒
   * @return int64_t            查看数据大小
   */
  int64_t peek(void* data, uint32_t request, uint32_t timeout_ms = 0)
  {
    int64_t  ret      = 0;
    uint8_t* data_ptr = reinterpret_cast<uint8_t*>(data);

    if (!m_opened)
    {
      ret = -1;
    }
    else
    {
      uint32_t peek_bytes = 0;
      uint32_t event_bits = 0;
      while (peek_bytes < request && m_opened)
      {
        if (m_input_buffer.empty())
        {
          event_bits = m_event_flags.wait((static_cast<uint32_t>(Bits::Receive_Finish) | static_cast<uint32_t>(Bits::Close)), timeout_ms);
          if ((0 == event_bits) || (event_bits & static_cast<uint32_t>(Bits::Close)))
          {
            break;
          }
        }

        peek_bytes += m_input_buffer.peek(data_ptr + peek_bytes, std::min(request - peek_bytes, m_input_buffer.available()));
      }
      ret = peek_bytes;
    }

    return ret;
  }

  /**
   * @brief  流设备 输入缓存区数据回滚
   */
  void roll_back(void)
  {
    if (m_opened)
    {
      m_input_buffer.roll_back();
    }
  }
};

/**
 * @brief  流设备模版类 只写模式特化 - 无输出缓存区
 *
 * @tparam In_Buf_Size  输入缓存区大小
 * @tparam In_Buf_Mode  输入缓存区模式
 */
template <uint32_t In_Buf_Size, memory::Ring_Buffer_Mode In_Buf_Mode>
class Stream_Device<Stream_Type::WRITE_ONLY, In_Buf_Size, 0, In_Buf_Mode> : public system_internal::device_internal::OutDevice_Base
{
  /// @warning 输入缓存区大小必须等于0
  static_assert(In_Buf_Size == 0, "In_Buf_Size must be 0");

  // 禁止拷贝和移动
  NO_COPY_MOVE(Stream_Device)

private:
  /// @brief 设备事件标志位
  using Bits = system_internal::device_internal::Device_Event_Bits;

protected:
  /**
   * @brief  流设备 设备数据发送元方法 - 纯虚函数
   *
   * @param  data      数据指针
   * @param  size      数据大小
   * @return uint32_t  实际发送数据大小
   */
  virtual uint32_t send_impl(const uint8_t* data, uint32_t size) = 0;

  /**
   * @brief  流设备 输出完成
   */
  void output_complete(void)
  {
    m_event_flags.clear(static_cast<uint32_t>(Bits::Enable_Transfer));
    m_event_flags.set(static_cast<uint32_t>(Bits::Transmit_Finish));
  }

  /**
   * @brief  流设备 构造函数
   */
  explicit Stream_Device() {}

  /**
   * @brief  流设备 析构函数
   */
  virtual ~Stream_Device() {}

public:
  /**
   * @brief  流设备 获取输入缓存区大小
   *
   * @return uint32_t 输入缓存区大小
   */
  static constexpr uint32_t input_buffer_size()
  {
    return 0;
  }

  /**
   * @brief  流设备 获取输出缓存区大小
   *
   * @return uint32_t 输出缓存区大小
   */
  static constexpr uint32_t output_buffer_size()
  {
    return 0;
  }

  /**
   * @brief  流设备 写入数据
   *
   * @param  data               数据缓存区指针
   * @param  size               数据大小
   * @param  timeout_ms         超时时间 - 毫秒
   * @return int64_t            写入数据大小
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
   * @brief  流设备 刷新输出缓存区数据
   *
   * @param  timeout_ms         超时时间 - 毫秒
   * @return Device_Error_Code  错误码
   */
  Device_Error_Code flush(uint32_t timeout_ms = TX_WAIT_FOREVER)
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

/**
 * @brief  流设备模版类 只写模式特化
 *
 * @tparam In_Buf_Size  输入缓存区大小
 * @tparam Out_Buf_Size 输出缓存区大小
 * @tparam In_Buf_Mode  输入缓存区模式
 */
template <uint32_t In_Buf_Size, uint32_t Out_Buf_Size, memory::Ring_Buffer_Mode In_Buf_Mode>
class Stream_Device<Stream_Type::WRITE_ONLY, In_Buf_Size, Out_Buf_Size, In_Buf_Mode> : public system_internal::device_internal::OutDevice_Base
{
  /// @warning 输入缓存区大小必须等于0
  static_assert(In_Buf_Size == 0, "In_Buf_Size must be 0");

  // 禁止拷贝和移动
  NO_COPY_MOVE(Stream_Device)

private:
  /// @brief 设备事件标志位
  using Bits = system_internal::device_internal::Device_Event_Bits;

protected:
  /// @brief 输出缓存区
  memory::Ring_Buffer<uint8_t, Out_Buf_Size, memory::Ring_Buffer_Mode::OUTPUT> m_output_buffer;

  /**
   * @brief  流设备 设备数据发送元方法 - 纯虚函数
   *
   * @param  data      数据指针
   * @param  size      数据大小
   * @return uint32_t  实际发送数据大小
   */
  virtual uint32_t send_impl(const uint8_t* data, uint32_t size) = 0;

  /**
   * @brief  流设备 获取输出缓存区指针
   *
   * @param  size     输出的数据大小 - 引用返回
   * @return uint8_t* 数据缓存区指针
   */
  uint8_t* output_start(uint32_t& size)
  {
    return m_output_buffer.output_start(size);
  }

  /**
   * @brief  流设备 输出完成
   */
  void output_complete(void)
  {
    m_output_buffer.output_complete();
    m_event_flags.set(static_cast<uint32_t>(Bits::Transmit_Finish));
  }

  /**
   * @brief  流设备 构造函数
   */
  explicit Stream_Device() {}

  /**
   * @brief  流设备 析构函数
   */
  virtual ~Stream_Device() {}

public:
  /**
   * @brief  流设备 获取输入缓存区大小
   *
   * @return uint32_t 输入缓存区大小
   */
  static constexpr uint32_t input_buffer_size()
  {
    return 0;
  }

  /**
   * @brief  流设备 获取输出缓存区大小
   *
   * @return uint32_t 输出缓存区大小
   */
  static constexpr uint32_t output_buffer_size()
  {
    return Out_Buf_Size;
  }

  /**
   * @brief  流设备 写入数据
   *
   * @param  data               数据缓存区指针
   * @param  size               数据大小
   * @param  timeout_ms         超时时间 - 毫秒
   * @return int64_t            写入数据大小
   */
  int64_t write(const void* data, uint32_t size, uint32_t timeout_ms = 0) override
  {
    int64_t        ret      = 0;
    const uint8_t* data_ptr = reinterpret_cast<const uint8_t*>(data);

    if (!m_opened)
    {
      ret = -1;
    }
    else if (0 != size)
    {
      uint32_t write_bytes = 0;
      uint32_t event_bits  = 0;
      while (write_bytes < size && m_opened)
      {
        if (m_output_buffer.full())
        {
          event_bits = m_event_flags.wait((static_cast<uint32_t>(Bits::Transmit_Finish) | static_cast<uint32_t>(Bits::Close)), timeout_ms);
          if ((0 == event_bits) || (static_cast<uint32_t>(Bits::Close) & event_bits))
          {
            break;
          }
        }

        if (m_opened)
        {
          write_bytes += m_output_buffer.write(data_ptr + write_bytes, std::min(size - write_bytes, m_output_buffer.space()));

          if (m_output_buffer.full())
          {
            m_event_flags.clear(static_cast<uint32_t>(Bits::Transmit_Finish));
            post_event(static_cast<uint32_t>(Bits::Enable_Transfer));
          }
        }
      }
      ret = write_bytes;
    }

    return ret;
  }

  /**
   * @brief  流设备 刷新输出缓存区数据
   *
   * @param  timeout_ms         超时时间 - 毫秒
   * @return Device_Error_Code  错误码
   */
  Device_Error_Code flush(uint32_t timeout_ms = TX_WAIT_FOREVER)
  {
    Device_Error_Code error_code = Device_Error_Code::OK;
    uint32_t          event_bits = 0;

    if (!m_opened)
    {
      error_code = Device_Error_Code::NOT_OPENED;
    }
    else
    {
      if (m_output_buffer.available())
      {
        post_event(static_cast<uint32_t>(Bits::Enable_Transfer));

        event_bits = m_event_flags.wait((static_cast<uint32_t>(Bits::Transmit_Finish) | static_cast<uint32_t>(Bits::Close)), timeout_ms);
        if (event_bits & static_cast<uint32_t>(Bits::Transmit_Finish))
        {
          m_event_flags.clear(static_cast<uint32_t>(Bits::Transmit_Finish));
        }
        else if (event_bits & static_cast<uint32_t>(Bits::Close))
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

/**
 * @brief  流设备模版类 读写模式特化 - 无输出缓存区
 *
 * @tparam In_Buf_Size  输入缓存区大小
 * @tparam In_Buf_Mode  输入缓存区模式
 */
template <uint32_t In_Buf_Size, memory::Ring_Buffer_Mode In_Buf_Mode>
class Stream_Device<Stream_Type::READ_WRITE, In_Buf_Size, 0, In_Buf_Mode> : public system_internal::device_internal::IODevice_Base
{
  /// @warning 输入缓存区大小必须大于0
  static_assert(In_Buf_Size > 0, "Input buffer size must be greater than 0");

  // 禁止拷贝和移动
  NO_COPY_MOVE(Stream_Device)

private:
  /// @brief 设备事件标志位
  using Bits = system_internal::device_internal::Device_Event_Bits;

protected:
  /// @brief 输入缓存区
  memory::Ring_Buffer<uint8_t, In_Buf_Size, In_Buf_Mode> m_input_buffer;

  /**
   * @brief  流设备 设备数据发送元方法 - 纯虚函数
   *
   * @param  data      数据指针
   * @param  size      数据大小
   * @return uint32_t  实际发送数据大小
   */
  virtual uint32_t send_impl(const uint8_t* data, uint32_t size) = 0;

  /**
   * @brief  流设备 推送数据进入缓存区
   *
   * @param  data               数据
   * @note   该方法仅在输入缓存区模式为INPUT_BYTES时有效
   */
  template <bool enable = (memory::Ring_Buffer_Mode::INPUT_BYTES == In_Buf_Mode), typename = std::enable_if_t<enable>>
  void input_buffer_push(const uint8_t data)
  {
    m_input_buffer.push(data);
  }

  /**
   * @brief  流设备 输入完成
   *
   * @note   该方法仅在输入缓存区模式为INPUT_BYTES时有效
   */
  template <bool enable = (memory::Ring_Buffer_Mode::INPUT_BYTES == In_Buf_Mode), typename = std::enable_if_t<enable>>
  void input_complete(void)
  {
    m_event_flags.set(static_cast<uint32_t>(Bits::Receive_Finish));
  }

  /**
   * @brief  流设备 获取输入缓存区指针
   *
   * @param  size     允许输入的最大数据大小 - 引用返回
   * @return uint8_t* 数据缓存区指针
   * @note   该方法仅在输入缓存区模式为INPUT_SINGLE_BUFFER时有效
   */
  template <bool enable = (memory::Ring_Buffer_Mode::INPUT_SINGLE_BUFFER == In_Buf_Mode), typename = std::enable_if_t<enable>>
  uint8_t* input_buffer_ptr(uint32_t& size)
  {
    return m_input_buffer.input_start(size);
  }

  /**
   * @brief  流设备 输入完成
   *
   * @param  size     成功输入的数据大小
   * @note   该方法仅在输入缓存区模式为INPUT_SINGLE_BUFFER或INPUT_DOUBLE_BUFFER时有效
   */
  template <bool enable = (memory::Ring_Buffer_Mode::INPUT_SINGLE_BUFFER == In_Buf_Mode || memory::Ring_Buffer_Mode::INPUT_DOUBLE_BUFFER == In_Buf_Mode), typename = std::enable_if_t<enable>>
  void input_complete(uint32_t size)
  {
    m_input_buffer.input_complete(size);
    if (size)
    {
      m_event_flags.set(static_cast<uint32_t>(Bits::Receive_Finish));
    }
  }

  /**
   * @brief  流设备 获取输入缓存区指针
   *
   * @param  ptr1     数据缓存区1指针
   * @param  ptr2     数据缓存区2指针
   * @return uint32_t 成功输入的数据大小
   * @note   该方法仅在输入缓存区模式为INPUT_DOUBLE_BUFFER时有效
   */
  template <bool enable = (memory::Ring_Buffer_Mode::INPUT_DOUBLE_BUFFER == In_Buf_Mode), typename = std::enable_if_t<enable>>
  uint32_t input_buffer_ptr(uint8_t*& ptr1, uint8_t*& ptr2)
  {
    return m_input_buffer.input_start(ptr1, ptr2);
  }

  /**
   * @brief  流设备 输入缓存区切换内存
   *
   * @note   该方法仅在输入缓存区模式为INPUT_DOUBLE_BUFFER时有效
   */
  template <bool enable = (memory::Ring_Buffer_Mode::INPUT_DOUBLE_BUFFER == In_Buf_Mode), typename = std::enable_if_t<enable>>
  void memory_switch(void)
  {
    m_input_buffer.switch_buffer();
    m_event_flags.set(static_cast<uint32_t>(Bits::Receive_Finish));
  }

  /**
   * @brief  流设备 输出完成
   */
  void output_complete(void)
  {
    m_event_flags.clear(static_cast<uint32_t>(Bits::Enable_Transfer));
    m_event_flags.set(static_cast<uint32_t>(Bits::Transmit_Finish));
  }

  /**
   * @brief  流设备 构造函数
   */
  explicit Stream_Device() {}

  /**
   * @brief  流设备 析构函数
   */
  virtual ~Stream_Device() {}

public:
  /**
   * @brief  流设备 获取输入缓存区大小
   *
   * @return uint32_t 输入缓存区大小
   */
  static constexpr uint32_t input_buffer_size()
  {
    return In_Buf_Size;
  }

  /**
   * @brief  流设备 获取输出缓存区大小
   *
   * @return uint32_t 输出缓存区大小
   */
  static constexpr uint32_t output_buffer_size()
  {
    return 0;
  }

  /**
   * @brief  流设备 读取数据
   *
   * @param  data               数据缓存区指针
   * @param  size               数据大小
   * @param  timeout_ms         超时时间 - 毫秒
   * @return int64_t            读取数据大小
   */
  int64_t read(void* data, uint32_t size, uint32_t timeout_ms = TX_WAIT_FOREVER) override
  {
    int64_t  ret      = 0;
    uint8_t* data_ptr = reinterpret_cast<uint8_t*>(data);

    if (!m_opened)
    {
      ret = -1;
    }
    else
    {
      uint32_t read_bytes = 0;
      uint32_t event_bits = 0;
      while (read_bytes < size && m_opened)
      {
        if (m_input_buffer.empty())
        {
          event_bits = m_event_flags.wait((static_cast<uint32_t>(Bits::Receive_Finish) | static_cast<uint32_t>(Bits::Close)), timeout_ms);
          if ((0 == event_bits) || (event_bits & static_cast<uint32_t>(Bits::Close)))
          {
            break;
          }
        }

        read_bytes += m_input_buffer.read(data_ptr + read_bytes, std::min(size - read_bytes, m_input_buffer.available()));

        if (m_input_buffer.empty())
        {
          m_event_flags.clear(static_cast<uint32_t>(Bits::Receive_Finish));
        }
      }
      ret = read_bytes;
    }

    return ret;
  }

  /**
   * @brief  流设备 写入数据
   *
   * @param  data               数据缓存区指针
   * @param  size               数据大小
   * @param  timeout_ms         超时时间 - 毫秒
   * @return int64_t            写入数据大小
   */
  int64_t write(const void* data, uint32_t size, uint32_t timeout_ms = 0) override
  {
    int64_t  ret           = 0;
    uint32_t event_bits    = 0;
    bool     need_transfer = false;
    const uint8_t* data_ptr = reinterpret_cast<const uint8_t*>(data);

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
   * @brief  流设备 刷新输出缓存区数据
   *
   * @param  timeout_ms         超时时间 - 毫秒
   * @return Device_Error_Code  错误码
   */
  Device_Error_Code flush(uint32_t timeout_ms = TX_WAIT_FOREVER)
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

  /**
   * @brief  流设备 获取输入缓存区当前存在的数据数量
   *
   * @return uint32_t           数据大小
   */
  uint32_t available(void) const override
  {
    uint32_t ret = 0;

    if (m_opened)
    {
      ret = m_input_buffer.available();
    }

    return ret;
  }

  /**
   * @brief  流设备 输入缓存区当前是否为空
   *
   * @return true               为空
   * @return false              不为空
   */
  bool empty(void) const override
  {
    bool ret = true;

    if (m_opened)
    {
      ret = m_input_buffer.empty();
    }

    return ret;
  }

  /**
   * @brief  流设备 输入缓存区当前是否已满
   *
   * @return true               已满
   * @return false              未满
   */
  bool full(void) const override
  {
    bool ret = false;

    if (m_opened)
    {
      ret = m_input_buffer.full();
    }

    return ret;
  }

  /**
   * @brief  流设备 清空输入缓存区数据
   *
   * @return Device_Error_Code 错误码
   */
  Device_Error_Code clear(void) override
  {
    Device_Error_Code error_code = Device_Error_Code::OK;

    if (!m_opened)
    {
      error_code = Device_Error_Code::NOT_OPENED;
    }
    else
    {
      m_input_buffer.clear();
    }

    return error_code;
  }

  /**
   * @brief  流设备 探视输入缓存区数据
   *
   * @param  data               数据缓存区指针
   * @param  request            请求数据大小
   * @param  timeout_ms         超时时间 - 毫秒
   * @return int64_t            查看数据大小
   */
  int64_t peek(void* data, uint32_t request, uint32_t timeout_ms = 0)
  {
    int64_t ret = 0;
    uint8_t * data_ptr = reinterpret_cast<uint8_t*>(data);

    if (!m_opened)
    {
      ret = -1;
    }
    else
    {
      uint32_t peek_bytes = 0;
      uint32_t event_bits = 0;
      while (peek_bytes < request && m_opened)
      {
        if (m_input_buffer.empty())
        {
          event_bits = m_event_flags.wait((static_cast<uint32_t>(Bits::Receive_Finish) | static_cast<uint32_t>(Bits::Close)), timeout_ms);
          if ((0 == event_bits) || (event_bits & static_cast<uint32_t>(Bits::Close)))
          {
            break;
          }
        }

        peek_bytes += m_input_buffer.peek(data_ptr + peek_bytes, std::min(request - peek_bytes, m_input_buffer.available()));
      }
      ret = peek_bytes;
    }

    return ret;
  }

  /**
   * @brief  流设备 输入缓存区数据回滚
   */
  void roll_back(void)
  {
    if (m_opened)
    {
      m_input_buffer.roll_back();
    }
  }
};

/**
 * @brief  流设备模版类 读写模式特化
 *
 * @tparam In_Buf_Size  输入缓存区大小
 * @tparam Out_Buf_Size 输出缓存区大小
 * @tparam In_Buf_Mode  输入缓存区模式
 */
template <uint32_t In_Buf_Size, uint32_t Out_Buf_Size, memory::Ring_Buffer_Mode In_Buf_Mode>
class Stream_Device<Stream_Type::READ_WRITE, In_Buf_Size, Out_Buf_Size, In_Buf_Mode> : public system_internal::device_internal::IODevice_Base
{
  /// @warning 输入缓存区大小必须大于0
  static_assert(In_Buf_Size > 0, "Input buffer size must be greater than 0");

  // 禁止拷贝和移动
  NO_COPY_MOVE(Stream_Device)

private:
  /// @brief 设备事件标志位
  using Bits = system_internal::device_internal::Device_Event_Bits;

protected:
  /// @brief 输入缓存区
  memory::Ring_Buffer<uint8_t, In_Buf_Size, In_Buf_Mode>                       m_input_buffer;
  /// @brief 输出缓存区
  memory::Ring_Buffer<uint8_t, Out_Buf_Size, memory::Ring_Buffer_Mode::OUTPUT> m_output_buffer;

  /**
   * @brief  流设备 设备数据发送元方法 - 纯虚函数
   *
   * @param  data      数据指针
   * @param  size      数据大小
   * @return uint32_t  实际发送数据大小
   */
  virtual uint32_t send_impl(const uint8_t* data, uint32_t size) = 0;

  /**
   * @brief  流设备 推送数据进入缓存区
   *
   * @param  data               数据
   * @note   该方法仅在输入缓存区模式为INPUT_BYTES时有效
   */
  template <bool enable = (memory::Ring_Buffer_Mode::INPUT_BYTES == In_Buf_Mode), typename = std::enable_if_t<enable>>
  void input_buffer_push(const uint8_t data)
  {
    m_input_buffer.push(data);
  }

  /**
   * @brief  流设备 输入完成
   *
   * @note   该方法仅在输入缓存区模式为INPUT_BYTES时有效
   */
  template <bool enable = (memory::Ring_Buffer_Mode::INPUT_BYTES == In_Buf_Mode), typename = std::enable_if_t<enable>>
  void input_complete(void)
  {
    m_event_flags.set(static_cast<uint32_t>(Bits::Receive_Finish));
  }

  /**
   * @brief  流设备 获取输入缓存区指针
   *
   * @param  size     允许输入的最大数据大小 - 引用返回
   * @return uint8_t* 数据缓存区指针
   * @note   该方法仅在输入缓存区模式为INPUT_SINGLE_BUFFER时有效
   */
  template <bool enable = (memory::Ring_Buffer_Mode::INPUT_SINGLE_BUFFER == In_Buf_Mode), typename = std::enable_if_t<enable>>
  uint8_t* input_buffer_ptr(uint32_t& size)
  {
    return m_input_buffer.input_start(size);
  }

  /**
   * @brief  流设备 输入完成
   *
   * @param  size     成功输入的数据大小
   * @note   该方法仅在输入缓存区模式为INPUT_SINGLE_BUFFER或INPUT_DOUBLE_BUFFER时有效
   */
  template <bool enable = (memory::Ring_Buffer_Mode::INPUT_SINGLE_BUFFER == In_Buf_Mode || memory::Ring_Buffer_Mode::INPUT_DOUBLE_BUFFER == In_Buf_Mode), typename = std::enable_if_t<enable>>
  void input_complete(uint32_t size)
  {
    m_input_buffer.input_complete(size);
    if (0 != size)
    {
      m_event_flags.set(static_cast<uint32_t>(Bits::Receive_Finish));
    }
  }

  /**
   * @brief  流设备 获取输入缓存区指针
   *
   * @param  ptr1     数据缓存区1指针
   * @param  ptr2     数据缓存区2指针
   * @return uint32_t 成功输入的数据大小
   * @note   该方法仅在输入缓存区模式为INPUT_DOUBLE_BUFFER时有效
   */
  template <bool enable = (memory::Ring_Buffer_Mode::INPUT_DOUBLE_BUFFER == In_Buf_Mode), typename = std::enable_if_t<enable>>
  uint32_t input_buffer_ptr(uint8_t*& ptr1, uint8_t*& ptr2)
  {
    return m_input_buffer.input_start(ptr1, ptr2);
  }

  /**
   * @brief  流设备 输入缓存区切换内存
   *
   * @note   该方法仅在输入缓存区模式为INPUT_DOUBLE_BUFFER时有效
   */
  template <bool enable = (memory::Ring_Buffer_Mode::INPUT_DOUBLE_BUFFER == In_Buf_Mode), typename = std::enable_if_t<enable>>
  void memory_switch(void)
  {
    m_input_buffer.switch_buffer();
    m_event_flags.set(static_cast<uint32_t>(Bits::Receive_Finish));
  }

  /**
   * @brief  流设备 获取输出缓存区指针
   *
   * @param  size     输出的数据大小 - 引用返回
   * @return uint8_t* 数据缓存区指针
   */
  uint8_t* output_start(uint32_t& size)
  {
    return m_output_buffer.output_start(size);
  }

  /**
   * @brief  流设备 输出完成
   */
  void output_complete(void)
  {
    m_output_buffer.output_complete();
    m_event_flags.set(static_cast<uint32_t>(Bits::Transmit_Finish));
  }

  /**
   * @brief  流设备 构造函数
   */
  explicit Stream_Device() {}

  /**
   * @brief  流设备 析构函数
   */
  virtual ~Stream_Device() {}

public:
  /**
   * @brief  流设备 获取输入缓存区大小
   *
   * @return uint32_t 输入缓存区大小
   */
  static constexpr uint32_t input_buffer_size()
  {
    return In_Buf_Size;
  }

  /**
   * @brief  流设备 获取输出缓存区大小
   *
   * @return uint32_t 输出缓存区大小
   */
  static constexpr uint32_t output_buffer_size()
  {
    return Out_Buf_Size;
  }

  /**
   * @brief  流设备 读取数据
   *
   * @param  data               数据缓存区指针
   * @param  size               数据大小
   * @param  timeout_ms         超时时间 - 毫秒
   * @return int64_t            读取数据大小
   */
  int64_t read(void* data, uint32_t size, uint32_t timeout_ms = TX_WAIT_FOREVER) override
  {
    int64_t ret = 0;
    uint8_t * data_ptr = reinterpret_cast<uint8_t*>(data);

    if (!m_opened)
    {
      ret = -1;
    }
    else
    {
      uint32_t read_bytes = 0;
      uint32_t event_bits = 0;
      while (read_bytes < size && m_opened)
      {
        if (m_input_buffer.empty())
        {
          event_bits = m_event_flags.wait((static_cast<uint32_t>(Bits::Receive_Finish) | static_cast<uint32_t>(Bits::Close)), timeout_ms);
          if ((0 == event_bits) || (event_bits & static_cast<uint32_t>(Bits::Close)))
          {
            break;
          }
        }

        read_bytes += m_input_buffer.read(data_ptr + read_bytes, std::min(size - read_bytes, m_input_buffer.available()));

        if (m_input_buffer.empty())
        {
          m_event_flags.clear(static_cast<uint32_t>(Bits::Receive_Finish));
        }
      }
      ret = read_bytes;
    }

    return ret;
  }

  /**
   * @brief  流设备 写入数据
   *
   * @param  data               数据缓存区指针
   * @param  size               数据大小
   * @param  timeout_ms         超时时间 - 毫秒
   * @return int64_t            写入数据大小
   */
  int64_t write(const void* data, uint32_t size, uint32_t timeout_ms = 0) override
  {
    int64_t ret = 0;
    const uint8_t* data_ptr = static_cast<const uint8_t*>(data);

    if (!m_opened)
    {
      ret = -1;
    }
    else if (0 != size)
    {
      uint32_t write_bytes = 0;
      uint32_t event_bits  = 0;
      while (write_bytes < size && m_opened)
      {
        if (m_output_buffer.full())
        {
          event_bits = m_event_flags.wait((static_cast<uint32_t>(Bits::Transmit_Finish) | static_cast<uint32_t>(Bits::Close)), timeout_ms);
          if ((0 == event_bits) || (static_cast<uint32_t>(Bits::Close) & event_bits))
          {
            break;
          }
        }

        if (m_opened)
        {
          write_bytes += m_output_buffer.write(data_ptr + write_bytes, std::min(size - write_bytes, m_output_buffer.space()));

          if (m_output_buffer.full())
          {
            m_event_flags.clear(static_cast<uint32_t>(Bits::Transmit_Finish));
            post_event(static_cast<uint32_t>(Bits::Enable_Transfer));
          }
        }
      }
      ret = write_bytes;
    }

    return ret;
  }

  /**
   * @brief  流设备 刷新输出缓存区数据
   *
   * @param  timeout_ms         超时时间 - 毫秒
   * @return Device_Error_Code  错误码
   */
  Device_Error_Code flush(uint32_t timeout_ms = TX_WAIT_FOREVER)
  {
    Device_Error_Code error_code = Device_Error_Code::OK;
    uint32_t          event_bits = 0;

    if (!m_opened)
    {
      error_code = Device_Error_Code::NOT_OPENED;
    }
    else
    {
      if (m_output_buffer.available())
      {
        post_event(static_cast<uint32_t>(Bits::Enable_Transfer));

        event_bits = m_event_flags.wait((static_cast<uint32_t>(Bits::Transmit_Finish) | static_cast<uint32_t>(Bits::Close)), timeout_ms);
        if (event_bits & static_cast<uint32_t>(Bits::Transmit_Finish))
        {
          m_event_flags.clear(static_cast<uint32_t>(Bits::Transmit_Finish));
        }
        else if (event_bits & static_cast<uint32_t>(Bits::Close))
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

  /**
   * @brief  流设备 获取输入缓存区当前存在的数据数量
   *
   * @return uint32_t           数据大小
   */
  uint32_t available(void) const override
  {
    uint32_t ret = 0;

    if (m_opened)
    {
      ret = m_input_buffer.available();
    }

    return ret;
  }

  /**
   * @brief  流设备 输入缓存区当前是否为空
   *
   * @return true               为空
   * @return false              不为空
   */
  bool empty(void) const override
  {
    bool ret = true;

    if (m_opened)
    {
      ret = m_input_buffer.empty();
    }

    return ret;
  }

  /**
   * @brief  流设备 输入缓存区当前是否已满
   *
   * @return true               已满
   * @return false              未满
   */
  bool full(void) const override
  {
    bool ret = false;

    if (m_opened)
    {
      ret = m_input_buffer.full();
    }

    return ret;
  }

  /**
   * @brief  流设备 清空输入缓存区数据
   *
   * @return Device_Error_Code 错误码
   */
  Device_Error_Code clear(void) override
  {
    Device_Error_Code error_code = Device_Error_Code::OK;

    if (!m_opened)
    {
      error_code = Device_Error_Code::NOT_OPENED;
    }
    else
    {
      m_input_buffer.clear();
    }

    return error_code;
  }

  /**
   * @brief  流设备 探视输入缓存区数据
   *
   * @param  data               数据缓存区指针
   * @param  request            请求数据大小
   * @param  timeout_ms         超时时间 - 毫秒
   * @return int64_t            查看数据大小
   */
  int64_t peek(void* data, uint32_t request, uint32_t timeout_ms = 0)
  {
    int64_t ret = 0;
    uint8_t * data_ptr = reinterpret_cast<uint8_t*>(data);

    if (!m_opened)
    {
      ret = -1;
    }
    else
    {
      uint32_t peek_bytes = 0;
      uint32_t event_bits = 0;
      while (peek_bytes < request && m_opened)
      {
        if (m_input_buffer.empty())
        {
          event_bits = m_event_flags.wait((static_cast<uint32_t>(Bits::Receive_Finish) | static_cast<uint32_t>(Bits::Close)), timeout_ms);
          if ((0 == event_bits) || (event_bits & static_cast<uint32_t>(Bits::Close)))
          {
            break;
          }
        }

        peek_bytes += m_input_buffer.peek(data_ptr + peek_bytes, std::min(request - peek_bytes, m_input_buffer.available()));
      }
      ret = peek_bytes;
    }

    return ret;
  }

  /**
   * @brief  流设备 输入缓存区数据回滚
   */
  void roll_back(void)
  {
    if (m_opened)
    {
      m_input_buffer.roll_back();
    }
  }
};
} /* namespace device */
} /* namespace system */
} /* namespace QAQ */

#endif /* __STREAMING_DEVICE_HPP__ */
