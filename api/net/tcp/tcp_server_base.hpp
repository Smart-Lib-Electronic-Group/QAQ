#ifndef __TCP_SERVER_BASE_HPP__
#define __TCP_SERVER_BASE_HPP__

#include "tcp_server_base_common.hpp"
#include "signal.hpp"

/// @brief TCP 服务器 友元类声明
#define TCP_SERVER_FRIEND                                                                                    \
  template <uint8_t C, QAQ::system::device::Stream_Type T, typename B, typename D, uint32_t SS, uint32_t QS> \
friend class QAQ::net::net_internal::tcp_internal::Tcp_Server_Base_Common;

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
 * @brief  TCP 服务器模板基类
 *
 * @tparam Client_Count   最大可支持的客户端数量
 * @tparam Type           流类型
 * @tparam Base_Device    设备基类
 * @tparam Stack_Size     服务器线程栈大小
 * @tparam Queue_Size     服务器线程信号队列大小
 * @tparam Derived        派生类
 */
template <uint8_t Client_Count, system::device::Stream_Type Type, typename Base_Device, uint32_t Stack_Size = TX_MINIMUM_STACK, uint32_t Queue_Size = 0, typename Derived = void>
class Tcp_Server_Base;

/**
 * @brief  TCP 服务器模板类 - 信号模式只读特化
 *
 * @tparam Client_Count   最大可支持的客户端数量
 * @tparam Base_Device    设备基类
 * @tparam Stack_Size     服务器线程栈大小
 * @tparam Queue_Size     服务器线程信号队列大小
 */
template <uint8_t Client_Count, typename Base_Device, uint32_t Stack_Size, uint32_t Queue_Size>
class Tcp_Server_Base<Client_Count, system::device::Stream_Type::WRITE_ONLY, Base_Device, Stack_Size, Queue_Size, void> : public Tcp_Server_Base_Common<Client_Count, system::device::Stream_Type::WRITE_ONLY, Base_Device, Tcp_Server_Base<Client_Count, system::device::Stream_Type::WRITE_ONLY, Base_Device, Stack_Size, Queue_Size, void>, Stack_Size, Queue_Size>
{
  /// @brief 友元类声明
  TCP_SERVER_FRIEND

private:
  /// @brief TCP 服务器 自身类型定义
  using Item        = Tcp_Server_Base<Client_Count, system::device::Stream_Type::WRITE_ONLY, Base_Device, Stack_Size, Queue_Size, void>;
  /// @brief TCP 服务器 CPRT基类类型定义
  using Cprt_Server = Tcp_Server_Base_Common<Client_Count, system::device::Stream_Type::WRITE_ONLY, Base_Device, Item, Stack_Size, Queue_Size>;

public:
  /// @brief TCP 服务器 客户端 类型定义
  using Client = Tcp_Server_Client<system::device::Stream_Type::WRITE_ONLY, Base_Device, Cprt_Server>;
  /// @brief TCP 服务器 客户端 设备类型定义
  using Device = system::device::Stream_Device<Base_Device::stream_type(), Base_Device::input_buffer_size(), Base_Device::output_buffer_size(), system::memory::Ring_Buffer_Mode::INPUT_DOUBLE_BUFFER>;

private:
  /**
   * @brief TCP 服务器 客户端连接事件处理函数
   *
   * @param client 客户端指针
   */
  void client_connected(Client* client)
  {
    signal_client_connected.emit(client);
  }

  /**
   * @brief TCP 服务器 客户端断开事件处理函数
   *
   * @param client 客户端指针
   */
  void client_disconnected(Client* client)
  {
    signal_client_disconnected.emit(client);
  }

  /**
   * @brief TCP 服务器 客户端超时事件处理函数
   *
   * @param  client   客户端指针
   * @return uint32_t 超时后保持连接的时间(ms)
   */
  uint32_t client_timeout(Client* client)
  {
    signal_client_timeout.emit(client);
    return this->get_timeout_count();
  }

public:
  /// @brief TCP 服务器 客户端连接信号
  system::signal::Signal<Device*> signal_client_connected;
  /// @brief TCP 服务器 客户端断开信号
  system::signal::Signal<Device*> signal_client_disconnected;
  /// @brief TCP 服务器 客户端超时信号
  system::signal::Signal<Device*> signal_client_timeout;

  /**
   * @brief TCP 服务器 构造函数
   *
   */
  explicit Tcp_Server_Base() {}

  /**
   * @brief TCP 服务器 析构函数
   *
   */
  virtual ~Tcp_Server_Base() {}
};

/**
 * @brief  TCP 服务器 模板类 - 信号模式只写/读写特化
 *
 * @tparam Client_Count   最大可支持的客户端数量
 * @tparam Type           流类型
 * @tparam Base_Device    设备基类
 * @tparam Stack_Size     服务器线程栈大小
 * @tparam Queue_Size     服务器线程信号队列大小
 */
template <uint8_t Client_Count, system::device::Stream_Type Type, typename Base_Device, uint32_t Stack_Size, uint32_t Queue_Size>
class Tcp_Server_Base<Client_Count, Type, Base_Device, Stack_Size, Queue_Size, void> : public Tcp_Server_Base_Common<Client_Count, Type, Base_Device, Tcp_Server_Base<Client_Count, Type, Base_Device, Stack_Size, Queue_Size, void>, Stack_Size, Queue_Size>
{
  /// @brief 友元类声明
  TCP_SERVER_FRIEND

private:
  /// @brief TCP 服务器 自身类型定义
  using Item        = Tcp_Server_Base<Client_Count, Type, Base_Device, Stack_Size, Queue_Size, void>;
  /// @brief TCP 服务器 CPRT基类类型定义
  using Cprt_Server = Tcp_Server_Base_Common<Client_Count, Type, Base_Device, Item, Stack_Size, Queue_Size>;

public:
  /// @brief TCP 服务器 客户端 类型定义
  using Client = Tcp_Server_Client<Type, Base_Device, Cprt_Server>;
  /// @brief TCP 服务器 客户端 设备类型定义
  using Device = system::device::Stream_Device<Base_Device::stream_type(), Base_Device::input_buffer_size(), Base_Device::output_buffer_size(), system::memory::Ring_Buffer_Mode::INPUT_DOUBLE_BUFFER>;

private:
  /**
   * @brief TCP 服务器 客户端连接事件处理函数
   *
   * @param client 客户端指针
   */
  void client_connected(Client* client)
  {
    signal_client_connected.emit(client);
  }

  /**
   * @brief TCP 服务器 客户端接收事件处理函数
   *
   * @param client 客户端指针
   */
  void client_received(Client* client)
  {
    signal_client_received.emit(client);
  }

  /**
   * @brief TCP 服务器 客户端断开事件处理函数
   *
   * @param client 客户端指针
   */
  void client_disconnected(Client* client)
  {
    signal_client_disconnected.emit(client);
  }

  /**
   * @brief TCP 服务器 客户端超时事件处理函数
   *
   * @param client 客户端指针
   * @return uint32_t 超时后保持连接的时间(ms)
   */
  uint32_t client_timeout(Client* client)
  {
    signal_client_timeout.emit(client);
    return this->get_timeout_count();
  }

public:
  /// @brief TCP 服务器 客户端连接信号
  system::signal::Signal<Device*> signal_client_connected;
  /// @brief TCP 服务器 客户端接收信号
  system::signal::Signal<Device*> signal_client_received;
  /// @brief TCP 服务器 客户端断开信号
  system::signal::Signal<Device*> signal_client_disconnected;
  /// @brief TCP 服务器 客户端超时信号
  system::signal::Signal<Device*> signal_client_timeout;

  /**
   * @brief TCP 服务器 构造函数
   *
   */
  explicit Tcp_Server_Base() {}

  /**
   * @brief TCP 服务器 析构函数
   *
   */
  virtual ~Tcp_Server_Base() {}
};

template <uint8_t Client_Count, system::device::Stream_Type Type, typename Base_Device, uint32_t Stack_Size, uint32_t Queue_Size, typename Derived>
class Tcp_Server_Base : public Tcp_Server_Base_Common<Client_Count, Type, Base_Device, Derived, Stack_Size, Queue_Size>
{
protected:
  /// @brief TCP 服务器 CPRT基类类型定义
  using Cprt_Server = Tcp_Server_Base_Common<Client_Count, Type, Base_Device, Derived, Stack_Size, Queue_Size>;

public:
  /// @brief TCP 服务器 客户端 类型定义
  using Client = Tcp_Server_Client<Type, Base_Device, Cprt_Server>;

public:
  /**
   * @brief TCP 服务器 构造函数
   *
   */
  explicit Tcp_Server_Base() {}

  /**
   * @brief TCP 服务器 析构函数
   *
   */
  virtual ~Tcp_Server_Base() {}
};
} /* namespace tcp_internal */
} /* namespace net_internal */
} /* namespace net */
} /* namespace QAQ */

#endif /* __TCP_SERVER_BASE_HPP__ */
