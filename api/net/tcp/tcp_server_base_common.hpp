#ifndef __TCP_SERVER_BASE_COMMON_HPP__
#define __TCP_SERVER_BASE_COMMON_HPP__

#include "tcp_server_client.hpp"

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
 * @brief  TCP 服务器 CPRT基类
 *
 * @tparam Client_Count   最大可支持的客户端数量
 * @tparam Type           流类型
 * @tparam Base_Device    设备基类
 * @tparam Derived        派生类
 * @tparam Stack_Size     服务器线程栈大小
 * @tparam Queue_Size     服务器线程信号队列大小
 */
template <uint8_t Client_Count, system::device::Stream_Type Type, typename Base_Device, typename Derived = void, uint32_t Stack_Size = TX_MINIMUM_STACK, uint32_t Queue_Size = 0>
class Tcp_Server_Base_Common : public system::thread::Thread<Stack_Size, Queue_Size, Tcp_Server_Base_Common<Client_Count, Type, Base_Device, Derived, Stack_Size, Queue_Size>, Queue_Size ? true : false>
{
  /// @warning TCP 服务器 只能是流设备
  static_assert(Base_Device::type() == system::device::Device_Type::STREAMING, "TCP Server must be streaming device");

  /// @brief 友元声明 服务器客户端
  template <system::device::Stream_Type Stream_Type, typename Server, typename Client>
  friend class Tcp_Server_Client_Base;

  /// @brief 服务器 事件
  using Server_Event = net_internal::Server_Event;
  /// @brief 服务器 自身类型
  using Item         = Tcp_Server_Base_Common<Client_Count, Type, Base_Device, Derived, Stack_Size, Queue_Size>;
  /// @brief 服务器 客户端类型
  using Client       = Tcp_Server_Client<Type, Base_Device, Item>;

  /// @brief 服务器 客户端事件包
  typedef struct
  {
    /// @brief 服务器 事件
    Server_Event event;
    /// @brief 服务器 客户端指针
    Client*      client;
  } client_event_packet;

  /// @brief 服务器 错误码
  using Tcp_Error_Code = system::device::Device_Error_Code;

  /// @brief 服务器 线程类型
  using Thread         = system::thread::Thread<Stack_Size, Queue_Size, Item, Queue_Size ? true : false>;
  /// @brief 服务器 定时器函数类型
  using Timer_Func     = void (*)(Item*);
  /// @brief 服务器 定时器类型
  using Timer          = system::Soft_Timer<Timer_Func, Item*>;
  /// @brief 服务器 消息队列类型
  using Queue          = system::kernel::Message_Queue<client_event_packet, (Client_Count > 1) ? Client_Count * 4 : 4>;

  /// @brief 服务器 定时器
  Timer            m_timer;
  /// @brief 服务器 消息队列
  Queue            m_queue;
  /// @brief 服务器 客户端数组
  Client           m_clients[Client_Count];
  /// @brief 服务器 监听客户端
  Client*          m_listen_client          = nullptr;
  /// @brief 服务器 已连接的客户端数量
  volatile uint8_t m_connected_client_count = 0;
  /// @brief 服务器 端口
  uint16_t         m_port                   = 0;
  /// @brief 服务器 超时次数
  uint32_t         m_timeout_count          = 0;
  /// @brief 服务器 信号通知
  bool             m_signal_notify          = false;

private:
  /**
   * @brief 服务器 发送事件
   *
   * @param client 客户端指针
   * @param event  事件
   */
  void post_event(Client* client, Server_Event event)
  {
    m_queue.send({ event, client }, 0);
  }

  /**
   * @brief 服务器 发送信号通知
   *
   */
  void signal_post_notify(void)
  {
    if constexpr (0 != Queue_Size)
    {
      if (!m_signal_notify)
      {
        m_signal_notify = true;
        this->post_event(nullptr, Server_Event::Signal_Post_Notify);
      }
    }
  }

  /**
   * @brief 服务器 定时器函数
   *
   * @param server 服务器指针
   */
  static void timer_function(Item* server)
  {
    if (0 == server->m_connected_client_count)
    {
      server->post_event(nullptr, Server_Event::Check_Timeout);
    }
  }

  THREAD_TASK
  {
    client_event_packet packet;
    while (1)
    {
      m_queue.receive(packet);

      if constexpr (0 != Queue_Size)
      {
        if (Server_Event::Signal_Post_Notify == packet.event)
        {
          this->process_signal(0);
          m_signal_notify = false;
          continue;
        }
      }

      if (Server_Event::Connect == packet.event)
      {
        client_connect_process(packet.client);
      }
      else if (Server_Event::Receive == packet.event)
      {
        client_receive_process(packet.client);
      }
      else if (Server_Event::Disconnect == packet.event)
      {
        client_disconnect_process(packet.client);
      }
      else if (Server_Event::Check_Timeout == packet.event)
      {
        client_check_timeout_process();
      }
    }
  }

  /**
   * @brief 服务器 客户端关闭处理句柄
   *
   */
  void client_close_handle(void)
  {
    m_connected_client_count = m_connected_client_count - 1;
  }

  /**
   * @brief 服务器 获取空闲客户端
   *
   * @param  client             客户端指针
   * @return Server_Error_Code  错误码
   */
  Server_Error_Code get_free_client(Client*& client)
  {
    Server_Error_Code ret = Server_Error_Code::NO_FREE_CLIENT;

    for (uint8_t i = 0; i < Client_Count; i++)
    {
      if (m_listen_client == &m_clients[i])
      {
        continue;
      }

      if (m_clients[i].is_free())
      {
        if (m_clients[i].is_opened())
        {
          m_clients[i].close();
        }
        else
        {
          m_clients[i].clean_timer_count();
        }

        client = &m_clients[i];
        ret    = Server_Error_Code::SUCCESS;
        break;
      }
    }

    return ret;
  }

  /**
   * @brief 服务器 重新监听
   *
   * @return Server_Error_Code 错误码
   */
  Server_Error_Code server_relisten()
  {
    Server_Error_Code ret    = Server_Error_Code::SUCCESS;
    Client*           client = nullptr;

    if (m_connected_client_count < Client_Count)
    {
      if (nullptr == m_listen_client)
      {
        ret = get_free_client(client);

        if (Server_Error_Code::SUCCESS == ret)
        {
          if (!client->relisten(m_port))
          {
            ret = Server_Error_Code::RELISTEN_FAILED;
          }
          else
          {
            m_listen_client = client;
          }
        }
      }
      else
      {
        if (m_listen_client->is_close())
        {
          if (!m_listen_client->relisten(m_port))
          {
            ret = Server_Error_Code::RELISTEN_FAILED;
          }
        }
      }
    }
    else
    {
      ret = Server_Error_Code::NO_FREE_CLIENT;
    }

    return ret;
  }

  /**
   * @brief 服务器 客户端连接处理
   *
   * @param client 客户端指针
   */
  void client_connect_process(Client* client)
  {
    client->clean_connect_flag();

    if (0 != m_port)
    {
      if (!client->is_opened())
      {
        if (client->accept())
        {
          m_listen_client          = nullptr;
          m_connected_client_count = m_connected_client_count + 1;
          client->connect_handle(m_timeout_count);
          static_cast<Derived*>(this)->client_connected(client);
        }
        else
        {
          client->unaccept();
        }

        server_relisten();
      }
    }
  }

  /**
   * @brief 服务器 客户端接收处理
   *
   * @param client 客户端指针
   */
  void client_receive_process(Client* client)
  {
    client->clean_receive_flag();

    if (client->is_receive())
    {
      if constexpr (system::device::Stream_Type::WRITE_ONLY == Type)
      {
        client->receive_handle();
      }
      else
      {
        client->receive_handle(
          [&, client]() -> void
          {
            static_cast<Derived*>(this)->client_received(client);
          });
      }
    }
  }

  /**
   * @brief 服务器 客户端断开处理
   *
   * @param client 客户端指针
   */
  void client_disconnect_process(Client* client)
  {
    client->clean_disconnect_flag();

    if (client->is_valid())
    {
      if (client->is_opened())
      {
        client->close();
        static_cast<Derived*>(this)->client_disconnected(client);
      }

      server_relisten();
    }
  }

  /**
   * @brief 服务器 客户端超时处理
   *
   */
  void client_check_timeout_process()
  {
    for (uint8_t i = 0; i < Client_Count; i++)
    {
      if (m_clients[i].is_close())
      {
        continue;
      }
      else
      {
        if (m_clients[i].is_timeout())
        {
          if (m_clients[i].delay_close_flag())
          {
            if (m_clients[i].is_opened())
            {
              m_clients[i].close();
            }
          }
          else
          {
            if (m_clients[i].is_opened())
            {
              uint32_t close_delay = static_cast<Derived*>(this)->client_timeout(&m_clients[i]);

              if (m_clients[i].is_opened())
              {
                if (0 == close_delay)
                {
                  m_clients[i].close();
                }
                else
                {
                  m_clients[i].set_timer_count(close_delay);
                  m_clients[i].set_delay_close_flag();

                  continue;
                }
              }
            }
          }

          server_relisten();
        }
        else
        {
          m_clients[i].timer_handle();
        }
      }
    }
  }

protected:
  /**
   * @brief 服务器 构造函数
   *
   */
  explicit Tcp_Server_Base_Common() : m_timer(1000, true, timer_function, this)
  {
    static_assert(std::is_base_of<Tcp_Server_Base_Common, Derived>::value, "Derived must be a subclass of Tcp_Server_Base_Common");
  }

  /**
   * @brief 服务器 获取超时时间
   *
   */
  const uint32_t get_timeout_count() const
  {
    return m_timeout_count;
  }

  /**
   * @brief 析构函数
   *
   */
  virtual ~Tcp_Server_Base_Common()
  {
    stop();
  }

public:
  /**
   * @brief 服务器 打开
   *
   * @param  name              服务器名称
   * @param  port              服务器端口号
   * @param  priority          服务器优先级
   * @param  timeout_count     服务器超时时间(s)
   * @return Server_Error_Code 错误码
   */
  Server_Error_Code start(const char* name, uint16_t port, uint16_t priority = 5, uint32_t timeout_count = 60)
  {
    Server_Error_Code ret    = Server_Error_Code::SUCCESS;
    Client*           client = nullptr;

    for (uint8_t i = 0; i < Client_Count; i++)
    {
      if (!m_clients[i].init(this))
      {
        ret = Server_Error_Code::INIT_ERROR;
        break;
      }
    }

    if (Server_Error_Code::SUCCESS == ret)
    {
      ret = get_free_client(client);
    }

    if (Server_Error_Code::SUCCESS == ret)
    {
      if (!client->listen(port, Client_Count))
      {
        ret = Server_Error_Code::LISTEN_FAILED;
      }
    }

    if (Server_Error_Code::SUCCESS == ret)
    {
      m_port          = port;
      m_listen_client = client;
      m_timeout_count = timeout_count;

      if (timeout_count)
      {
        m_timer.create((std::string(name) + " timer").c_str());
        m_timer.start();
      }

      Thread::create((std::string(name) + " thread").c_str(), priority);
      Thread::start();
    }

    return ret;
  }

  /**
   * @brief 服务器 获取已连接客户端数量
   *
   * @return uint8_t 已连接客户端数量
   */
  const uint8_t get_opened_client_count() const
  {
    return m_connected_client_count;
  }

  /**
   * @brief 服务器 获取已连接第一个客户端的引用
   *
   * @return Client& 已连接第一个客户端的引用
   */
  Client& get_first_opened_client(void)
  {
    bool    is_found = false;
    uint8_t i        = 0;

    for (i = 0; i < Client_Count; i++)
    {
      if (m_clients[i].is_opened())
      {
        is_found = true;
        break;
      }
    }

    if (is_found)
    {
      return m_clients[i];
    }
    else
    {
      return m_clients[0];
    }
  }

  /**
   * @brief 服务器 向所有已连接客户端发送数据
   *
   * @param data 数据指针
   * @param size 数据大小
   * @param timeout 超时时间
   */
  void write_all_opened_client(const uint8_t* data, uint32_t size, uint32_t timeout = TX_WAIT_FOREVER)
  {
    for (uint8_t i = 0; i < Client_Count; i++)
    {
      m_clients[i].write(data, size, timeout);
    }
  }

  /**
   * @brief 服务器 刷新所有已连接客户端输出缓存区
   *
   */
  void flush_all_opened_client()
  {
    for (uint8_t i = 0; i < Client_Count; i++)
    {
      m_clients[i].flush();
    }
  }

  /**
   * @brief 服务器 关闭
   *
   * @return Server_Error_Code 错误码
   */
  Server_Error_Code stop()
  {
    Server_Error_Code ret = Server_Error_Code::SUCCESS;

    if (0 != m_port)
    {
      this->suspend();
      m_timer.stop();
      m_clients[0].unlisten(m_port);

      for (uint8_t i = 0; i < Client_Count; i++)
      {
        m_clients[i].deinit();
      }
    }

    return ret;
  }
};
} /* namespace tcp_internal */
} /* namespace net_internal */
} /* namespace net */
} /* namespace QAQ */

#endif /* __TCP_SERVER_BASE_COMMON_HPP__ */
