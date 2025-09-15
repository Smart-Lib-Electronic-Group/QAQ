#ifndef __TCP_SOCKET_HPP__
#define __TCP_SOCKET_HPP__

#include "net_manager.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 网络
namespace net
{
/// @brief 名称空间 网络 内部
namespace net_internal
{
/// @brief 名称空间 TCP 内部
namespace tcp_internal
{
/**
 * @brief  TCP套接字 连接回调
 *
 * @tparam T        子类类型(回调函数的所有者)
 * @param  socket   套接字指针
 */
template <typename T>
void tcp_socket_connect_callback(NX_TCP_SOCKET* socket, UINT port)
{
  (void)port;

  auto* t = static_cast<T*>(socket->nx_tcp_socket_user_data[0]);
  t->connect_callback();
}

/**
 * @brief  TCP套接字 接收回调
 *
 * @tparam T        子类类型(回调函数的所有者)
 * @param  socket   套接字指针
 */
template <typename T>
void tcp_socket_received_callback(NX_TCP_SOCKET* socket)
{
  auto* t = static_cast<T*>(socket->nx_tcp_socket_user_data[0]);
  t->receive_callback();
}

/**
 * @brief  TCP套接字 断开回调
 *
 * @tparam T        子类类型(回调函数的所有者)
 * @param  socket   套接字指针
 */
template <typename T>
void tcp_socket_disconnect_callback(NX_TCP_SOCKET* socket)
{
  auto* t = static_cast<T*>(socket->nx_tcp_socket_user_data[0]);
  t->disconnect_callback();
}

/**
 * @brief  TCP套接字 基类
 *
 * @tparam Derived  子类类型(回调函数的所有者)
 */
template <typename Derived>
class Tcp_Socket
{
private:
  /// @brief TCP套接字 结构体
  mutable NX_TCP_SOCKET m_socket = { 0 };

protected:
  /**
   * @brief  TCP套接字 判断套接字是否空闲
   *
   * @return true   空闲
   * @return false  未空闲
   */
  bool is_free(void)
  {
    return ((NX_TCP_CLOSED == m_socket.nx_tcp_socket_state) && (NX_NULL == m_socket.nx_tcp_socket_bound_next) && (NX_NULL == m_socket.nx_tcp_socket_bind_in_progress));
  }

  /**
   * @brief  TCP套接字 判断套接字是否接收数据
   *
   * @return true   有数据
   * @return false  无数据
   */
  bool is_receive(void)
  {
    return (m_socket.nx_tcp_socket_receive_queue_head);
  }

  /**
   * @brief  TCP套接字 判断套接字是否已建立
   *
   * @return true   已建立
   * @return false  未建立
   */
  bool is_established(void)
  {
    return (NX_TCP_ESTABLISHED < m_socket.nx_tcp_socket_state);
  }

  /**
   * @brief  TCP套接字 判断套接字是否处于发送状态
   *
   * @return true   处于发送状态
   * @return false  未处于发送状态
   */
  bool is_send(void)
  {
    return (NX_TCP_SYN_SENT > m_socket.nx_tcp_socket_state);
  }

  /**
   * @brief  TCP套接字 判断套接字是否处于关闭状态
   *
   * @return true   处于关闭状态
   * @return false  未处于关闭状态
   */
  bool is_close(void)
  {
    return (NX_TCP_CLOSED == m_socket.nx_tcp_socket_state);
  }

  /**
   * @brief  TCP套接字 创建套接字
   *
   * @param  name        套接字名称
   * @param  buffer_size 接收缓冲区大小
   * @param  ptr         回调函数的所有者指针
   * @return true        创建成功
   * @return false       创建失败
   */
  bool socket_create(const char* name, uint32_t buffer_size, Derived* ptr)
  {
    bool ret    = false;
    UINT status = NX_SUCCESS;

    status      = nx_tcp_socket_create(Net_Manager::instance().get_ip(), &m_socket, const_cast<char*>(name), NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, buffer_size, NX_NULL, tcp_socket_disconnect_callback<Derived>);

#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
    if (NX_SUCCESS != status)
    {
      QAQ_ERROR_LOG(status, "TCP Socket Create Failed");
    }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */

    if (NX_SUCCESS == status)
    {
      status = nx_tcp_socket_receive_notify(&m_socket, tcp_socket_received_callback<Derived>);

      if (NX_SUCCESS != status)
      {
        nx_tcp_socket_delete(&m_socket);

#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
        QAQ_ERROR_LOG(status, "TCP Socket Receive Notify Failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */
      }
    }

    if (NX_SUCCESS == status)
    {
      m_socket.nx_tcp_socket_user_data[0] = ptr;
      ret                                 = true;
    }

    return ret;
  }

  /**
   * @brief  TCP套接字 监听端口
   *
   * @param  port        端口号
   * @param  queue_size  连接队列大小
   * @return true        监听成功
   * @return false       监听失败
   */
  bool listen(uint16_t port, uint16_t queue_size)
  {
    bool ret    = false;
    UINT status = NX_SUCCESS;

    status      = nx_tcp_server_socket_listen(Net_Manager::instance().get_ip(), port, &m_socket, queue_size, tcp_socket_connect_callback<Derived>);

    if (NX_SUCCESS == status)
    {
      ret = true;
    }
#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
    else
    {
      QAQ_ERROR_LOG(status, "TCP Socket Listen Failed");
    }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */

    return ret;
  }

  /**
   * @brief  TCP套接字 重新监听端口
   *
   * @param  port        端口号
   * @return true        重新监听成功
   * @return false       重新监听失败
   */
  bool relisten(uint16_t port)
  {
    bool ret    = false;
    UINT status = NX_SUCCESS;

    status      = nx_tcp_server_socket_relisten(Net_Manager::instance().get_ip(), port, &m_socket);

    if (NX_SUCCESS == status || NX_CONNECTION_PENDING == status)
    {
      ret = true;
    }
#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
    else
    {
      QAQ_ERROR_LOG(status, "TCP Socket Relisten Failed");
    }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */

    return ret;
  }

  /**
   * @brief  TCP套接字 取消监听端口
   *
   * @param  port        端口号
   * @return true        取消监听成功
   * @return false       取消监听失败
   */
  bool unlisten(uint16_t port)
  {
    bool ret    = false;
    UINT status = NX_SUCCESS;

    status      = nx_tcp_server_socket_unlisten(Net_Manager::instance().get_ip(), port);

    if (NX_SUCCESS == status)
    {
      ret = true;
    }
#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
    else
    {
      QAQ_ERROR_LOG(status, "TCP Socket Unlisten Failed");
    }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */

    return ret;
  }

  /**
   * @brief  TCP套接字 接受连接
   *
   * @param  timeout  超时时间
   * @return true     接受成功
   * @return false    接受失败
   */
  bool accept(uint32_t timeout = NX_WAIT_FOREVER)
  {
    bool ret    = false;
    UINT status = NX_SUCCESS;

    status      = nx_tcp_server_socket_accept(&m_socket, timeout);

    if (NX_SUCCESS == status)
    {
      ret = true;
    }
#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
    else
    {
      QAQ_ERROR_LOG(status, "TCP Socket Accept Failed");
    }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */

    return ret;
  }

  /**
   * @brief 断开连接
   *
   * @return true    成功
   * @return false   失败
   */
  bool unaccept(void)
  {
    bool ret    = false;
    UINT status = NX_SUCCESS;

    status      = nx_tcp_server_socket_unaccept(&m_socket);

    if (NX_SUCCESS == status)
    {
      ret = true;
    }
#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
    else
    {
      QAQ_ERROR_LOG(status, "TCP Socket Unaccept Failed");
    }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */

    return ret;
  }

  /**
   * @brief  TCP套接字 绑定端口
   *
   * @param  port        端口号
   * @param  timeout     超时时间
   * @return true        绑定成功
   * @return false       绑定失败
   */
  bool bind(uint16_t port, uint32_t timeout = NX_WAIT_FOREVER)
  {
    bool ret    = false;
    UINT status = NX_SUCCESS;

    status      = nx_tcp_client_socket_bind(&m_socket, port, timeout);

    if (NX_SUCCESS == status)
    {
      ret = true;
    }
#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
    else
    {
      QAQ_ERROR_LOG(status, "TCP Socket Bind Failed");
    }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */

    return ret;
  }

  /**
   * @brief  TCP套接字 取消绑定端口
   *
   * @return true    成功
   * @return false   失败
   */
  bool unbind(void)
  {
    bool ret    = false;
    UINT status = NX_SUCCESS;

    status      = nx_tcp_client_socket_unbind(&m_socket);

    if (NX_SUCCESS == status)
    {
      ret = true;
    }
#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
    else
    {
      QAQ_ERROR_LOG(status, "TCP Socket Unbind Failed");
    }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */

    return ret;
  }

  /**
   * @brief  TCP套接字 连接服务器
   *
   * @param  ip         服务器IP地址(数组)
   * @param  port       服务器端口号
   * @param  timeout    超时时间
   * @return true       连接成功
   * @return false      连接失败
   */
  bool connect(uint8_t* ip, uint16_t port, uint32_t timeout = 1000)
  {
    bool        ret    = false;
    UINT        status = NX_SUCCESS;
    NXD_ADDRESS server_address;

    server_address.nxd_ip_version    = NX_IP_VERSION_V4;
    server_address.nxd_ip_address.v4 = IP_ADDRESS(ip[0], ip[1], ip[2], ip[3]);

    status                           = nxd_tcp_client_socket_connect(&m_socket, &server_address, port, timeout);

    if (NX_SUCCESS == status)
    {
      ret = true;
    }
#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
    else
    {
      QAQ_ERROR_LOG(status, "TCP Socket Connect Failed");
    }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */

    return ret;
  }

  /**
   * @brief  TCP套接字 连接服务器
   *
   * @param  ip         服务器IP地址(字符串)
   * @param  port       服务器端口号
   * @param  timeout    超时时间
   * @return true       连接成功
   * @return false      连接失败
   */
  bool connect(const char* ip, uint16_t port, uint32_t timeout = 1000)
  {
    bool    ret         = true;
    uint8_t ip_arrry[4] = { 0 };
    int     parse[4]    = { 0 };
    sscanf(ip, "%d.%d.%d.%d", &parse[0], &parse[1], &parse[2], &parse[3]);

    if (parse[0] < 0 || parse[0] > 255)
    {
      ret = false;
    }
    else
    {
      ip_arrry[0] = parse[0];
      ip_arrry[1] = parse[1];
      ip_arrry[2] = parse[2];
      ip_arrry[3] = parse[3];
    }

    if (ret)
    {
      ret = connect(ip_arrry, port, timeout);
    }

    return ret;
  }

  /**
   * @brief  TCP套接字 断开连接
   *
   * @param  timeout    超时时间
   * @return true       断开成功
   * @return false      断开失败
   */
  bool disconnect(uint32_t timeout = 1000)
  {
    bool ret    = false;
    UINT status = NX_SUCCESS;

    status      = nx_tcp_socket_disconnect(&m_socket, timeout);

    if (NX_SUCCESS == status)
    {
      ret = true;
    }
#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
    else if (NX_NOT_CONNECTED != status && NX_DISCONNECT_FAILED != status)
    {
      QAQ_ERROR_LOG(status, "TCP Socket Disconnect Failed");
    }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */

    return ret;
  }

  /**
   * @brief  TCP套接字 发送数据(高速)
   *
   * @param  data       数据指针
   * @param  size       数据大小
   * @return uint32_t   发送大小
   */
  uint32_t send(const uint8_t* data, uint32_t size)
  {
    UINT       status         = NX_SUCCESS;
    uint32_t   ret            = 0;
    ULONG      packet_size    = 0;
    NX_PACKET* packet         = nullptr;
    NX_PACKET* packet_ptr     = nullptr;
    NX_PACKET* last_packet    = nullptr;
    ULONG      remaining_size = 0;
    uint8_t*   data_ptr       = const_cast<uint8_t*>(data);

    status                    = nx_tcp_socket_mss_get(&m_socket, &packet_size);

    if (status != NX_SUCCESS)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
      QAQ_ERROR_LOG(status, "TCP Socket Get MSS Failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */
    }
    else
    {
      while (size)
      {
        status = nx_packet_allocate(Net_Manager::instance().get_pool(), &packet, NX_TCP_PACKET, NX_WAIT_FOREVER);

        if (status == NX_SUCCESS)
        {
          if (size < packet_size)
          {
            packet_size = size;
          }

          {
            if (packet->nx_packet_prepend_ptr + packet_size <= packet->nx_packet_data_end)
            {
              __builtin_memcpy(packet->nx_packet_prepend_ptr, data_ptr, packet_size);
              data_ptr                     += packet_size;
              size                         -= packet_size;

              packet->nx_packet_append_ptr  = packet->nx_packet_prepend_ptr + packet_size;
              remaining_size                = 0;
            }
            else
            {
              __builtin_memcpy(packet->nx_packet_prepend_ptr, data_ptr, packet->nx_packet_data_end - packet->nx_packet_prepend_ptr);
              data_ptr                     += packet->nx_packet_data_end - packet->nx_packet_prepend_ptr;
              size                         -= packet->nx_packet_data_end - packet->nx_packet_prepend_ptr;

              packet->nx_packet_append_ptr  = packet->nx_packet_data_end;
              remaining_size                = packet_size - (ULONG)(packet->nx_packet_append_ptr - packet->nx_packet_prepend_ptr);
              last_packet                   = packet;
            }

            packet->nx_packet_length = packet_size;
          }

          while (remaining_size)
          {
            status = nx_packet_allocate(Net_Manager::instance().get_pool(), &packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);

            if (status != NX_SUCCESS)
            {
              break;
            }

            last_packet->nx_packet_next = packet_ptr;
            last_packet                 = packet_ptr;

            if (remaining_size < (ULONG)(packet_ptr->nx_packet_data_end - packet_ptr->nx_packet_prepend_ptr))
            {
              __builtin_memcpy(packet_ptr->nx_packet_prepend_ptr, data_ptr, remaining_size);
              data_ptr                         += remaining_size;
              size                             -= remaining_size;

              packet_ptr->nx_packet_append_ptr  = packet_ptr->nx_packet_prepend_ptr + remaining_size;
              packet->nx_packet_length         += remaining_size;
            }
            else
            {
              __builtin_memcpy(packet_ptr->nx_packet_prepend_ptr, data_ptr, packet_ptr->nx_packet_data_end - packet_ptr->nx_packet_prepend_ptr);
              data_ptr                         += packet_ptr->nx_packet_data_end - packet_ptr->nx_packet_prepend_ptr;
              size                             -= packet_ptr->nx_packet_data_end - packet_ptr->nx_packet_prepend_ptr;

              packet_ptr->nx_packet_append_ptr  = packet_ptr->nx_packet_data_end;
              packet_ptr->nx_packet_length      = packet_ptr->nx_packet_data_end - packet_ptr->nx_packet_prepend_ptr;
            }

            remaining_size = remaining_size - (ULONG)(packet_ptr->nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr);
          }

          if (status != NX_SUCCESS)
          {
#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
            QAQ_ERROR_LOG(status, "TCP Socket Packet Allocate Failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */
            break;
          }

          status = nx_tcp_socket_send(&m_socket, packet, NX_WAIT_FOREVER);

          if (status)
          {
            nx_packet_release(packet);

#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
            if (NX_WINDOW_OVERFLOW != status && NX_NOT_CONNECTED != status)
            {
              QAQ_ERROR_LOG(status, "TCP Socket Send Failed");
            }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */

            break;
          }
          else
          {
            ret += packet_size;
          }
        }
      }
    }
    return ret;
  }

  /**
   * @brief  TCP套接字 发送数据
   *
   * @param  data       数据指针
   * @param  size       数据大小
   * @param  timeout    超时时间
   * @return uint32_t   发送大小
   */
  uint32_t send(uint8_t* data, uint32_t size, uint32_t timeout)
  {
    UINT       status = NX_SUCCESS;
    uint32_t   ret    = 0;
    NX_PACKET* packet = nullptr;

    status            = nx_packet_allocate(Net_Manager::instance().get_pool(), &packet, NX_TCP_PACKET, NX_WAIT_FOREVER);

#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
    if (NX_SUCCESS != status)
    {
      QAQ_ERROR_LOG(status, "TCP Socket Packet Allocate Failed");
    }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */

    if (NX_SUCCESS == status)
    {
      status = nx_packet_data_append(packet, const_cast<uint8_t*>(data), size, Net_Manager::instance().get_pool(), NX_WAIT_FOREVER);

      if (NX_SUCCESS != status)
      {
        nx_packet_release(packet);
#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
        QAQ_ERROR_LOG(status, "TCP Socket Packet Data Append Failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */
      }
    }

    if (NX_SUCCESS == status)
    {
      status = nx_packet_length_get(packet, &ret);
#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
      if (NX_SUCCESS != status)
      {
        ret = 0;
        nx_packet_release(packet);

        QAQ_ERROR_LOG(status, "TCP Socket Packet Length Get Failed");
      }
      else
      {
        if (ret != size)
        {
          QAQ_ERROR_LOG(status, "TCP Socket Packet Size Error");
        }
      }
#else
      if (NX_SUCCESS != status)
      {
        ret = 0;
        nx_packet_release(packet);
      }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */
    }

    if (NX_SUCCESS == status)
    {
      status = nx_tcp_socket_send(&m_socket, packet, timeout);

      if (NX_SUCCESS != status)
      {
        nx_packet_release(packet);

#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
        if (NX_WINDOW_OVERFLOW != status && NX_NOT_CONNECTED != status)
        {
          QAQ_ERROR_LOG(status, "TCP Socket Send Failed");
        }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */
      }
    }

    return ret;
  }

  /**
   * @brief  TCP套接字 接收数据(持续数据)
   *
   * @param  memory0_ptr  内存0指针
   * @param  memory1_ptr  内存1指针
   * @param  memory_size  内存大小
   * @param  switch_func  切换函数
   * @param  finish_func  完成函数
   * @return uint32_t     接收大小
   */
  template <typename LambdaA, typename LambdaB>
  uint32_t receive(uint8_t* memory0_ptr, uint8_t* memory1_ptr, uint32_t memory_size, LambdaA&& switch_func, LambdaB&& finish_func)
  {
    UINT       status           = NX_SUCCESS;
    NX_PACKET* packet           = nullptr;
    uint32_t   total_length     = 0;
    uint32_t   data_size        = 0;
    uint8_t*   data_ptr         = nullptr;
    uint8_t    memory_number    = 0;
    uint8_t*   memory_ptr       = memory0_ptr;
    uint32_t   memory_free_size = memory_size;

    while (1)
    {
      status = nx_tcp_socket_receive(&m_socket, &packet, NX_WAIT_FOREVER);

      if (NX_SUCCESS != status)
      {
        break;
      }

      data_size = packet->nx_packet_length;
      data_ptr  = packet->nx_packet_prepend_ptr;

      while (data_size)
      {
        if (data_size >= memory_free_size)
        {
          __builtin_memcpy(memory_ptr, data_ptr, memory_free_size);

          total_length += memory_free_size;
          data_size    -= memory_free_size;
          data_ptr     += memory_free_size;

          if (0 == memory_number)
          {
            memory_number    = 1;
            memory_ptr       = memory1_ptr;
            memory_free_size = memory_size;
          }
          else
          {
            memory_number    = 0;
            memory_ptr       = memory0_ptr;
            memory_free_size = memory_size;
          }

          switch_func();
        }
        else
        {
          __builtin_memcpy(memory_ptr, data_ptr, data_size);

          memory_ptr       += data_size;
          memory_free_size -= data_size;

          total_length     += data_size;
          data_size         = 0;
          data_ptr          = nullptr;
        }
      }

      nx_packet_release(packet);

      if (!is_receive())
      {
        break;
      }
    }

#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
    if (NX_SUCCESS != status)
    {
      if (NX_NOT_CONNECTED != status)
      {
        QAQ_ERROR_LOG(status, "TCP Socket Receive Failed");
      }
    }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */

    finish_func(memory_size - memory_free_size);

    return total_length;
  }

  /**
   * @brief  TCP套接字 接收数据
   *
   * @param  data       数据指针
   * @param  size       数据大小
   * @param  timeout    超时时间
   * @return uint32_t   接收大小
   */
  uint32_t receive(uint8_t* data, uint32_t size, uint32_t timeout)
  {
    UINT       status = NX_SUCCESS;
    NX_PACKET* packet = nullptr;
    uint32_t   length = 0;

    status            = nx_tcp_socket_receive(&m_socket, &packet, timeout);

#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
    if (NX_SUCCESS != status)
    {
      if (NX_NOT_CONNECTED != status)
      {
        QAQ_ERROR_LOG(status, "TCP Socket Receive Failed");
      }
    }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */

    if (NX_SUCCESS == status)
    {
      status = nx_packet_length_get(packet, &length);

      if (NX_SUCCESS != status)
      {
        length = 0;
        nx_packet_release(packet);

#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
        QAQ_ERROR_LOG(status, "TCP Socket Packet Length Get Failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */
      }
    }

    if (NX_SUCCESS == status)
    {
      status = nx_packet_data_extract_offset(packet, 0, data, size, &length);

      if (NX_SUCCESS != status)
      {
        length = 0;

#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
        QAQ_ERROR_LOG(status, "TCP Socket Packet Data Extract Failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */
      }

      nx_packet_release(packet);
    }

    return length;
  }

  /**
   * @brief  TCP套接字 接收数据(清理数据)
   *
   * @return true   接收成功
   * @return false  接收失败
   */
  bool receive_and_clean(void)
  {
    bool       ret    = false;
    UINT       status = NX_SUCCESS;
    NX_PACKET* packet = nullptr;

    while (1)
    {
      status = nx_tcp_socket_receive(&m_socket, &packet, NX_WAIT_FOREVER);

      if (NX_SUCCESS != status)
      {
        break;
      }

      nx_packet_release(packet);
      ret = true;

      if (!is_receive())
      {
        break;
      }
    }

    if (NX_SUCCESS != status)
    {
      if (NX_NOT_CONNECTED != status)
      {
        ret = false;
#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
        QAQ_ERROR_LOG(status, "TCP Socket Receive Failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */
      }
    }

    return ret;
  }

  /**
   * @brief  TCP套接字 获取对端IP
   *
   * @param  ip     对端IP(主机字节序)
   * @return true   获取成功
   * @return false  获取失败
   */
  bool get_peer_ip(uint32_t& ip) const
  {
    bool ret    = false;
    UINT status = NX_SUCCESS;

    status      = nx_tcp_socket_peer_info_get(&m_socket, &ip, NX_NULL);

    if (NX_SUCCESS == status)
    {
      ret = true;
    }
    else
    {
      ret = false;

#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
      if (NX_NOT_CONNECTED != status)
      {
        QAQ_ERROR_LOG(status, "TCP Socket Peer Info Get Failed");
      }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */
    }

    return ret;
  }

  /**
   * @brief  TCP套接字 获取对端端口
   *
   * @param  port     对端端口(主机字节序)
   * @return true     获取成功
   * @return false    获取失败
   */
  bool get_peer_port(uint32_t& port) const
  {
    bool ret    = false;
    UINT status = NX_SUCCESS;

    status      = nx_tcp_socket_peer_info_get(&m_socket, NX_NULL, &port);

    if (NX_SUCCESS == status)
    {
      ret = true;
    }
    else
    {
      ret = false;

#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
      if (NX_NOT_CONNECTED != status)
      {
        QAQ_ERROR_LOG(status, "TCP Socket Peer Info Get Failed");
      }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */
    }

    return ret;
  }

  /**
   * @brief  TCP套接字 获取客户端端口(仅用于客户端)
   *
   * @param  port     客户端端口(主机字节序)
   * @return true     获取成功
   * @return false    获取失败
   */
  bool get_client_port(uint32_t& port)
  {
    bool ret    = false;
    UINT status = NX_SUCCESS;

    status      = nx_tcp_client_socket_port_get(&m_socket, &port);

    if (NX_SUCCESS == status)
    {
      ret = true;
    }
    else
    {
      ret = false;

#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
      if (NX_NOT_CONNECTED != status)
      {
        QAQ_ERROR_LOG(status, "TCP Socket Client Info Get Failed");
      }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */
    }

    return ret;
  }

  /**
   * @brief  TCP套接字 删除
   *
   * @return true
   * @return false
   */
  bool socket_delete(void)
  {
    bool ret    = false;
    UINT status = NX_SUCCESS;

    status      = nx_tcp_socket_delete(&m_socket);

    if (NX_SUCCESS == status)
    {
      ret = true;
    }
#if (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE)
    else
    {
      QAQ_ERROR_LOG(status, "TCP Socket Delete Failed");
    }
#endif /* (SYSTEM_ERROR_LOG_ENABLE && TCP_SOCKET_ERROR_LOG_ENABLE) */

    return ret;
  }
};
} /* namespace tcp_internal */
} /* namespace net_internal */
} /* namespace net */
} /* namespace QAQ */

#endif /* __TCP_SOCKET_HPP__ */
