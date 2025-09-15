#ifndef __TCP_SERVER_CLIENT_HPP__
#define __TCP_SERVER_CLIENT_HPP__

#include "net_base.hpp"
#include "tcp_socket.hpp"
#include "streaming_device.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 网络
namespace net
{
/// @brief 命名空间 内部
namespace net_internal
{
/// @brief 名称空间 TCP 内部
namespace tcp_internal
{
/**
 * @brief  TCP 服务器 模版类
 *
 * @tparam Client_Count 客户端数量
 * @tparam Type         流类型
 * @tparam Device       设备类
 * @tparam Derived      派生类
 * @tparam Stack_Size   栈大小
 * @tparam Queue_Size   队列大小
 */
template <uint8_t Client_Count, system::device::Stream_Type Type, typename Device, typename Derived, uint32_t Stack_Size, uint32_t Queue_Size>
class Tcp_Server_Base_Common;

/**
 * @brief  TCP 服务器客户端 模版类
 *
 * @tparam Type        流类型
 * @tparam Server      服务器类
 * @tparam Base_Device 设备基类
 */
template <system::device::Stream_Type Type, typename Server, typename Base_Device>
class Tcp_Server_Client;

/**
 * @brief  TCP 服务器客户端 模版类 - 基类
 *
 * @tparam Type        流类型
 * @tparam Server      服务器类
 * @tparam Client      客户端类
 */
template <system::device::Stream_Type Type, typename Server, typename Client>
class Tcp_Server_Client_Base : protected Tcp_Socket<Tcp_Server_Client_Base<Type, Server, Client>>
{
  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(Tcp_Server_Client_Base)

private:
  /// @brief 服务器客户端 连接标志
  static constexpr uint32_t CONNECT_FLAG       = 0x0001;
  /// @brief 服务器客户端 断开连接标志
  static constexpr uint32_t DISCONNECT_FLAG    = 0x0002;
  /// @brief 服务器客户端 接收标志
  static constexpr uint32_t RECEIVE_FLAG       = 0x0004;
  /// @brief 服务器客户端 打开标志
  static constexpr uint32_t OPEN_FLAG          = 0x1000;

  /// @brief 服务器客户端 接受超时时间
  static constexpr uint32_t ACCEPT_TIMEOUT     = 5000;
  /// @brief 服务器客户端 断开连接超时时间
  static constexpr uint32_t DISCONNECT_TIMEOUT = 5000;

  /// @brief 友元函数 连接回调
  template <typename T>
  friend void tcp_socket_connect_callback(NX_TCP_SOCKET* socket, UINT);

  /// @brief 友元函数 接收回调
  template <typename T>
  friend void tcp_socket_received_callback(NX_TCP_SOCKET* socket);

  /// @brief 友元函数 断开连接回调
  template <typename T>
  friend void tcp_socket_disconnect_callback(NX_TCP_SOCKET* socket);

  /// @brief 服务器客户端 错误码
  using Tcp_Error_Code                 = system::device::Device_Error_Code;
  /// @brief 服务器客户端 事件
  using Server_Event                   = net_internal::Server_Event;
  /// @brief 服务器客户端 设备类型
  using Device_Type                    = system::device::Stream_Type;
  /// @brief 服务器客户端 套接字
  using Socket                         = Tcp_Socket<Tcp_Server_Client_Base<Type, Server, Client>>;

  /// @brief 服务器客户端 标志
  volatile uint32_t m_flag             = 0;
  /// @brief 服务器客户端 超时时间
  uint32_t          m_timer_count_max  = 0;
  /// @brief 服务器客户端 超时计数
  uint32_t          m_timer_count      = 0;
  /// @brief 服务器客户端 是否需要延迟关闭
  bool              m_delay_close_flag = false;
  /// @brief 服务器客户端 服务器指针
  Server*           m_server           = nullptr;

private:
  /**
   * @brief 服务器客户端 连接回调
   *
   */
  void connect_callback(void)
  {
    if (0 == (m_flag & CONNECT_FLAG))
    {
      m_flag = m_flag | CONNECT_FLAG;
      m_server->post_event(static_cast<Client*>(this), Server_Event::Connect);
    }
  }

  /**
   * @brief 服务器客户端 接收回调
   *
   */
  void receive_callback(void)
  {
    if (0 == (m_flag & RECEIVE_FLAG))
    {
      m_flag = m_flag | RECEIVE_FLAG;
      m_server->post_event(static_cast<Client*>(this), Server_Event::Receive);
    }
  }

  /**
   * @brief 服务器客户端 断开连接回调
   *
   */
  void disconnect_callback(void)
  {
    if (0 == (m_flag & DISCONNECT_FLAG))
    {
      m_flag = m_flag | DISCONNECT_FLAG;
      m_server->post_event(static_cast<Client*>(this), Server_Event::Disconnect);
    }
  }

protected:
  /**
   * @brief 服务器客户端 清理连接标志
   *
   */
  void clean_connect_flag(void)
  {
    m_flag = m_flag & ~CONNECT_FLAG;
  }

  /**
   * @brief 服务器客户端 清理断开连接标志
   *
   */
  void clean_disconnect_flag(void)
  {
    m_flag = m_flag & ~DISCONNECT_FLAG;
  }

  /**
   * @brief 服务器客户端 清理接收标志
   *
   */
  void clean_receive_flag(void)
  {
    m_flag = m_flag & ~RECEIVE_FLAG;
  }

  /**
   * @brief  服务器客户端 发送实现
   *
   * @param  data       数据指针
   * @param  size       数据大小
   * @return uint32_t   发送大小
   */
  template <bool enable = (Type != Device_Type::READ_ONLY), typename = std::enable_if_t<enable>>
  uint32_t send_handler(const uint8_t* data, uint32_t size)
  {
    uint32_t ret = 0;

    if ((m_flag & OPEN_FLAG) && size)
    {
      ret = Socket::send(data, size);
      static_cast<Client*>(this)->output_complete();
    }

    return ret;
  }

  /**
   * @brief 服务器客户端 打开实现
   *
   * @return Tcp_Error_Code 错误码
   */
  Tcp_Error_Code open_handler()
  {
    Tcp_Error_Code error_code = Tcp_Error_Code::OK;

    if (0 == (m_flag & OPEN_FLAG))
    {
      error_code = Tcp_Error_Code::WAIT_FOR_CONNECT;
    }

    return error_code;
  }

  /**
   * @brief 服务器客户端 关闭实现
   *
   * @return Tcp_Error_Code 错误码
   */
  Tcp_Error_Code close_handler()
  {
    Tcp_Error_Code error_code = Tcp_Error_Code::OK;

    if (static_cast<Client*>(this)->is_opened())
    {
      Socket::disconnect(DISCONNECT_TIMEOUT);
      Socket::unaccept();
      m_flag            = 0;
      m_timer_count     = 0;
      m_timer_count_max = 0;
      m_server->client_close_handle();
    }

    return error_code;
  }

  /**
   * @brief 服务器客户端 配置实现
   *
   * @param  param          参数
   * @param  value          值
   * @return Tcp_Error_Code 错误码
   */
  Tcp_Error_Code config_handler(uint32_t param, uint32_t value)
  {
    return Tcp_Error_Code::INVALID_PARAMETER;
  }

  /**
   * @brief 服务器客户端 获取配置实现
   *
   * @param  param          参数
   * @return uint32_t       值
   */
  uint32_t get_config_handler(uint32_t param) const
  {
    uint32_t ret = 0;

    if (Config::IP == param)
    {
      Socket::get_peer_ip(ret);
    }
    else if (Config::PORT == param)
    {
      Socket::get_peer_port(ret);
    }

    return ret;
  }

  /**
   * @brief 服务器客户端 事件处理实现
   *
   * @param  event     事件类型
   */
  void base_manger_handler(uint32_t event)
  {
    if constexpr (0 < Client::output_buffer_size())
    {
      if (event & static_cast<uint32_t>(system::system_internal::device_internal::Device_Event_Bits::Enable_Transfer))
      {
        if (m_flag & OPEN_FLAG)
        {
          uint32_t size = 0;
          uint8_t* ptr  = static_cast<Client*>(this)->output_start(size);

          send_handler(ptr, size);
        }
      }
    }
  }

  /**
   * @brief 服务器客户端 连接处理句柄
   *
   * @param  timer_count_max 超时计数
   */
  void connect_handle(uint32_t timer_count_max)
  {
    m_flag            = m_flag | OPEN_FLAG;
    m_timer_count_max = m_timer_count = timer_count_max;
    static_cast<Client*>(this)->open();

    if constexpr (Type != Device_Type::WRITE_ONLY)
    {
      static_cast<Client*>(this)->clear();
    }
  }

  /**
   * @brief 服务器客户端 接收处理句柄
   *
   * @param  func       回调函数
   * @return uint32_t   接收大小
   * @note   该函数仅在 Type 不为 Device_Type::WRITE_ONLY 时有效
   */
  template <typename Lambda, bool enable = (Type != Device_Type::WRITE_ONLY), typename = std::enable_if_t<enable>>
  uint32_t receive_handle(Lambda&& func)
  {
    uint8_t* memory0_ptr = nullptr;
    uint8_t* memory1_ptr = nullptr;
    uint32_t memory_size = static_cast<Client*>(this)->input_buffer_ptr(memory0_ptr, memory1_ptr);

    uint32_t size        = Socket::receive(
      memory0_ptr,
      memory1_ptr,
      memory_size,
      [&]() -> void
      {
        static_cast<Client*>(this)->memory_switch();
        func();
      },
      [&](uint32_t final_size) -> void
      {
        static_cast<Client*>(this)->input_complete(final_size);
        func();
      });

    if (size)
    {
      reset_timer_count();
    }

    return size;
  }

  /**
   * @brief 服务器客户端 接收处理句柄
   *
   * @note   该函数仅在 Type 为 Device_Type::WRITE_ONLY 时有效
   */
  template <bool enable = (Type == Device_Type::WRITE_ONLY), typename = std::enable_if_t<enable>>
  void receive_handle(void)
  {
    if (Socket::receive_and_clean())
    {
      reset_timer_count();
    }
  }

  /**
   * @brief 服务器客户端 定时器处理句柄
   *
   */
  void timer_handle(void)
  {
    if (m_timer_count > 0)
    {
      m_timer_count--;
    }
  }

protected:
  /**
   * @brief 服务器客户端 构造函数
   *
   */
  explicit Tcp_Server_Client_Base()
  {
    /// @warning 客户端必须继承于Tcp_Server_Client_Base
    static_assert(std::is_base_of_v<Tcp_Server_Client_Base<Type, Server, Client>, Client>, "Client must be derived from Tcp_Server_Client_Base");
  }

  /**
   * @brief  服务器客户端 是否有效
   *
   * @return false 无效
   */
  bool is_valid(void)
  {
    return (Socket::is_established() || (Socket::is_send() && m_timer_count));
  }

  /**
   * @brief 服务器客户端 是否超时
   *
   * @return true  超时
   * @return false 未超时
   */
  bool is_timeout(void)
  {
    return (m_timer_count == 0);
  }

  /**
   * @brief 服务器客户端 接受处理
   *
   * @return true  成功
   * @return false 失败
   */
  bool accept(void)
  {
    return Socket::accept(ACCEPT_TIMEOUT);
  }

  /**
   * @brief 服务器客户端 初始化
   *
   * @param  server 服务器指针
   * @param  client 客户端指针
   * @param  name   名称
   * @return true   成功
   * @return false  失败
   */
  bool init(Server* server, const char* name = "TCP_Server_Client")
  {
    m_server = server;
    return Socket::socket_create(name, ((Client::input_buffer_size() - 1) * 5), this);
  }

  /**
   * @brief 服务器客户端 设置超时计数
   *
   * @param timer_count_max 超时计数
   */
  void set_timer_count_max(uint32_t timer_count_max)
  {
    m_timer_count_max  = timer_count_max;
    m_delay_close_flag = false;
  }

  /**
   * @brief 服务器客户端 清理超时计数
   *
   */
  void clean_timer_count(void)
  {
    m_timer_count      = 0;
    m_timer_count_max  = 0;
    m_delay_close_flag = false;
  }

  /**
   * @brief 服务器客户端 重置超时计数
   *
   */
  void reset_timer_count(void)
  {
    m_timer_count      = m_timer_count_max;
    m_delay_close_flag = false;
  }

  /**
   * @brief 服务器客户端 设置超时计数
   *
   * @param timer_count 超时计数
   */
  void set_timer_count(uint32_t timer_count)
  {
    m_timer_count = timer_count;
  }

  /**
   * @brief 服务器客户端 设置需要延迟关闭标志
   *
   */
  void set_delay_close_flag()
  {
    m_delay_close_flag = true;
  }

  /**
   * @brief  服务器客户端 获取需要延迟关闭标志
   *
   * @return true  已经需要延迟关闭
   * @return false 未需要延迟关闭
   */
  bool delay_close_flag()
  {
    return m_delay_close_flag;
  }

  /**
   * @brief 服务器客户端 注销
   *
   * @return true  成功
   * @return false 失败
   */
  bool deinit(void)
  {
    static_cast<Client*>(this)->close();
    return Socket::socket_delete();
  }

  /**
   * @brief 服务器客户端 析构函数
   *
   */
  virtual ~Tcp_Server_Client_Base() {}
};

/**
 * @brief  TCP 服务器客户端 模版类 - 只读模式特化
 *
 * @tparam Server      服务器类
 * @tparam Base_Device 设备基类
 */
template <typename Server, typename Base_Device>
class Tcp_Server_Client<system::device::Stream_Type::READ_ONLY, Base_Device, Server> : public system::device::Stream_Device<Base_Device::stream_type(), Base_Device::input_buffer_size(), Base_Device::output_buffer_size(), system::memory::Ring_Buffer_Mode::INPUT_DOUBLE_BUFFER>, public Tcp_Server_Client_Base<system::device::Stream_Type::READ_ONLY, Server, Tcp_Server_Client<system::device::Stream_Type::READ_ONLY, Base_Device, Server>>
{
  /// @warning TCP 服务器客户端 只能是流设备
  static_assert(Base_Device::type() == system::device::Device_Type::STREAMING, "TCP Client must be streaming device");

  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(Tcp_Server_Client)

  /// @brief 友元声明 服务器客户端 基类
  template <system::device::Stream_Type Type, typename Tcp_Server, typename Client>
  friend class Tcp_Server_Client_Base;

  /// @brief 友元声明 服务器类
  template <uint8_t Client_Count, system::device::Stream_Type Type, typename Device, typename Derived, uint32_t Stack_Size, uint32_t Queue_Size>
  friend class Tcp_Server_Base_Common;

  /// @brief 服务器客户端 错误码
  using Tcp_Error_Code = system::device::Device_Error_Code;
  /// @brief 服务器客户端 基类类型
  using Base           = Tcp_Server_Client_Base<system::device::Stream_Type::READ_ONLY, Server, Tcp_Server_Client<system::device::Stream_Type::READ_ONLY, Base_Device, Server>>;
  /// @brief 服务器客户端 设备类型
  using Device         = system::device::Stream_Device<Base_Device::stream_type(), Base_Device::input_buffer_size(), Base_Device::output_buffer_size(), system::memory::Ring_Buffer_Mode::INPUT_DOUBLE_BUFFER>;

private:
  /**
   * @brief 服务器客户端 打开实现
   *
   * @return Tcp_Error_Code 错误码
   */
  Tcp_Error_Code open_impl() override
  {
    return Base::open_handler();
  }

  /**
   * @brief 服务器客户端 关闭实现
   *
   * @return Tcp_Error_Code 错误码
   */
  Tcp_Error_Code close_impl() override
  {
    return Base::close_handler();
  }

  /**
   * @brief 服务器客户端 配置实现
   *
   * @param  param          参数
   * @param  value          值
   * @return Tcp_Error_Code 错误码
   */
  Tcp_Error_Code config_impl(uint32_t param, uint32_t value) override
  {
    return Base::config_handler(param, value);
  }

  /**
   * @brief 服务器客户端 获取配置实现
   *
   * @param  param          参数
   * @return uint32_t       值
   */
  const uint32_t get_config_impl(uint32_t param) override
  {
    return Base::get_config_handler(param);
  }

  /**
   * @brief 服务器客户端 事件处理实现
   *
   * @param  event     事件类型
   */
  void manger_handler(uint32_t event) override {}

protected:
  /**
   * @brief 服务器客户端 构造函数
   *
   */
  explicit Tcp_Server_Client() {}

  /**
   * @brief 服务器客户端 析构函数
   *
   */
  virtual ~Tcp_Server_Client() {}

public:
  /**
   * @brief 获取设备
   *
   * @return Device& 设备
   */
  Device& device()
  {
    return *this;
  }
};

/**
 * @brief  TCP 服务器客户端 模版类 - 只写模式特化
 *
 * @tparam Server      服务器类
 * @tparam Base_Device 设备基类
 */
template <typename Server, typename Base_Device>
class Tcp_Server_Client<system::device::Stream_Type::WRITE_ONLY, Base_Device, Server> : public system::device::Stream_Device<Base_Device::stream_type(), Base_Device::input_buffer_size(), Base_Device::output_buffer_size(), system::memory::Ring_Buffer_Mode::INPUT_DOUBLE_BUFFER>, public Tcp_Server_Client_Base<system::device::Stream_Type::WRITE_ONLY, Server, Tcp_Server_Client<system::device::Stream_Type::WRITE_ONLY, Base_Device, Server>>
{
  /// @warning TCP 服务器客户端 只能是流设备
  static_assert(Base_Device::type() == system::device::Device_Type::STREAMING, "TCP Client must be streaming device");

  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(Tcp_Server_Client)

  /// @brief 服务器客户端 基类类型
  using Base = Tcp_Server_Client_Base<system::device::Stream_Type::WRITE_ONLY, Server, Tcp_Server_Client<system::device::Stream_Type::WRITE_ONLY, Base_Device, Server>>;

  /// @brief 友元声明 服务器类
  template <uint8_t Client_Count, system::device::Stream_Type Type, typename Device, typename Derived, uint32_t Stack_Size, uint32_t Queue_Size>
  friend class Tcp_Server_Base_Common;

  /// @brief 友元声明 服务器客户端 基类
  template <system::device::Stream_Type Type, typename Tcp_Server, typename Client>
  friend class Tcp_Server_Client_Base;

  /// @brief 服务器客户端 错误码
  using Tcp_Error_Code = system::device::Device_Error_Code;
  /// @brief 服务器客户端 设备类型
  using Device         = system::device::Stream_Device<Base_Device::stream_type(), Base_Device::input_buffer_size(), Base_Device::output_buffer_size(), system::memory::Ring_Buffer_Mode::INPUT_DOUBLE_BUFFER>;

private:
  /**
   * @brief  服务器客户端 发送实现
   *
   * @param  data       数据指针
   * @param  size       数据大小
   * @return uint32_t   发送大小
   */
  uint32_t send_impl(const uint8_t* data, uint32_t size) override
  {
    return Base::send_handler(data, size);
  }

  /**
   * @brief 服务器客户端 打开实现
   *
   * @return Tcp_Error_Code 错误码
   */
  Tcp_Error_Code open_impl() override
  {
    return Base::open_handler();
  }

  /**
   * @brief 服务器客户端 关闭实现
   *
   * @return Tcp_Error_Code 错误码
   */
  Tcp_Error_Code close_impl() override
  {
    return Base::close_handler();
  }

  /**
   * @brief 服务器客户端 配置实现
   *
   * @param  param          参数
   * @param  value          值
   * @return Tcp_Error_Code 错误码
   */
  Tcp_Error_Code config_impl(uint32_t param, uint32_t value) override
  {
    return Base::config_handler(param, value);
  }

  /**
   * @brief 服务器客户端 获取配置实现
   *
   * @param  param          参数
   * @return uint32_t       值
   */
  const uint32_t get_config_impl(uint32_t param) override
  {
    return Base::get_config_handler(param);
  }

  /**
   * @brief 服务器客户端 事件处理实现
   *
   * @param  event     事件类型
   */
  void manger_handler(uint32_t event) override
  {
    Base::base_manger_handler(event);
  }

protected:
  /**
   * @brief 服务器客户端 构造函数
   *
   */
  explicit Tcp_Server_Client() {}

  /**
   * @brief 服务器客户端 析构函数
   *
   */
  virtual ~Tcp_Server_Client() {}

public:
  /**
   * @brief 获取设备
   *
   * @return Device& 设备
   */
  Device& device()
  {
    return *this;
  }
};

/**
 * @brief  TCP 服务器客户端 模版类 - 读写模式特化
 *
 * @tparam Server      服务器类
 * @tparam Base_Device 设备基类
 */
template <typename Server, typename Base_Device>
class Tcp_Server_Client<system::device::Stream_Type::READ_WRITE, Base_Device, Server> : public system::device::Stream_Device<Base_Device::stream_type(), Base_Device::input_buffer_size(), Base_Device::output_buffer_size(), system::memory::Ring_Buffer_Mode::INPUT_DOUBLE_BUFFER>, public Tcp_Server_Client_Base<system::device::Stream_Type::READ_WRITE, Server, Tcp_Server_Client<system::device::Stream_Type::READ_WRITE, Base_Device, Server>>
{
  /// @warning TCP 服务器客户端 只能是流设备
  static_assert(Base_Device::type() == system::device::Device_Type::STREAMING, "TCP Client must be streaming device");

  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(Tcp_Server_Client)

  /// @brief 服务器客户端 基类类型
  using Base = Tcp_Server_Client_Base<system::device::Stream_Type::READ_WRITE, Server, Tcp_Server_Client<system::device::Stream_Type::READ_WRITE, Base_Device, Server>>;

  /// @brief 友元声明 服务器类
  template <uint8_t Client_Count, system::device::Stream_Type Type, typename Device, typename Derived, uint32_t Stack_Size, uint32_t Queue_Size>
  friend class Tcp_Server_Base_Common;

  /// @brief 友元声明 服务器客户端 基类
  template <system::device::Stream_Type Type, typename Tcp_Server, typename Client>
  friend class Tcp_Server_Client_Base;

  /// @brief 服务器客户端 错误码
  using Tcp_Error_Code = system::device::Device_Error_Code;
  /// @brief 服务器客户端 设备类型
  using Device         = system::device::Stream_Device<Base_Device::stream_type(), Base_Device::input_buffer_size(), Base_Device::output_buffer_size(), system::memory::Ring_Buffer_Mode::INPUT_DOUBLE_BUFFER>;

private:
  /**
   * @brief  服务器客户端 发送实现
   *
   * @param  data       数据指针
   * @param  size       数据大小
   * @return uint32_t   发送大小
   */
  uint32_t send_impl(const uint8_t* data, uint32_t size) override
  {
    return Base::send_handler(data, size);
  }

  /**
   * @brief 服务器客户端 打开实现
   *
   * @return Tcp_Error_Code 错误码
   */
  Tcp_Error_Code open_impl() override
  {
    return Base::open_handler();
  }

  /**
   * @brief 服务器客户端 关闭实现
   *
   * @return Tcp_Error_Code 错误码
   */
  Tcp_Error_Code close_impl() override
  {
    return Base::close_handler();
  }

  /**
   * @brief 服务器客户端 配置实现
   *
   * @param  param          参数
   * @param  value          值
   * @return Tcp_Error_Code 错误码
   */
  Tcp_Error_Code config_impl(uint32_t param, uint32_t value) override
  {
    return Base::config_handler(param, value);
  }

  /**
   * @brief 服务器客户端 获取配置实现
   *
   * @param  param          参数
   * @return uint32_t       值
   */
  uint32_t get_config_impl(uint32_t param) const override
  {
    return Base::get_config_handler(param);
  }

  /**
   * @brief 服务器客户端 事件处理实现
   *
   * @param  event     事件类型
   */
  void manger_handler(uint32_t event) override
  {
    Base::base_manger_handler(event);
  }

protected:
  /**
   * @brief 服务器客户端 构造函数
   *
   */
  explicit Tcp_Server_Client() {}

  /**
   * @brief 服务器客户端 析构函数
   *
   */
  virtual ~Tcp_Server_Client() {}

public:
  /**
   * @brief 获取设备
   *
   * @return Device& 设备
   */
  Device& device()
  {
    return *this;
  }
};
} /* namespace tcp_internal */
} /* namespace net_internal */
} /* namespace net */
} /* namespace QAQ */

#endif /* __TCP_SERVER_CLIENT_HPP__ */
