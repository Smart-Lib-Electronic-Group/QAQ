#ifndef __NET_MANAGER_HPP__
#define __NET_MANAGER_HPP__

#include "system_include.hpp"
#include "nx_stm32_eth_driver.h"
#include "nx_api.h"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 网络
namespace net
{
/// @brief 名称空间 网络 内部
namespace net_internal
{
/**
 * @brief 网络管理器
 *
 */
class Net_Manager
{
private:
  /// @brief 网络管理器 网络数据包数量
  static constexpr uint32_t PACKET_COUNT                       = 80;
  /// @brief 网络管理器 ARP缓存数量
  static constexpr uint32_t ARP_COUNT                          = 24;

  /// @brief 网络管理器 IP地址池大小
  static constexpr uint32_t IP_POOL_SIZE                       = 2048;
  /// @brief 网络管理器 ARP缓存池大小
  static constexpr uint32_t ARP_POOL_SIZE                      = 52 * ARP_COUNT;
  /// @brief 网络管理器 网络数据包池大小
  static constexpr uint32_t PACKET_POOL_SIZE                   = PACKET_COUNT * (1536 + sizeof(NX_PACKET));

  /// @brief 网络管理器 网络数据包池名称
  static constexpr char packet_pool_name[]                     = "NetX Main Packet Pool";
  /// @brief 网络管理器 IP实例名称
  static constexpr char ip_name[]                              = "NetX IP Instance";

  /// @brief 网络管理器 网络数据包池内存
  uint8_t m_packet_pool_memory[PACKET_POOL_SIZE] QAQ_ALIGN(32) = { 0 };
  /// @brief 网络管理器 IP地址池内存
  uint8_t m_ip_pool_memory[IP_POOL_SIZE] QAQ_ALIGN(32)         = { 0 };
  /// @brief 网络管理器 ARP缓存池内存
  uint8_t m_arp_pool_memory[ARP_POOL_SIZE] QAQ_ALIGN(32)       = { 0 };

  /// @brief 网络管理器 网络数据包池
  NX_PACKET_POOL m_packet_pool                                 = { 0 };
  /// @brief 网络管理器 IP实例
  NX_IP          m_ip                                          = { 0 };
  /// @brief 网络管理器 初始化状态
  bool           m_init                                        = false;

#ifdef NX_ENABLE_DUAL_PACKET_POOL
  /// @brief 网络管理器 附属网络数据包池数量
  static constexpr uint32_t DUAL_PACKET_COUNT                                   = PACKET_COUNT / 4;
  /// @brief 网络管理器 附属网络数据包池大小
  static constexpr uint32_t DUAL_PACKET_POOL_SIZE                               = DUAL_PACKET_COUNT * (200 + sizeof(NX_PACKET));
  /// @brief 网络管理器 附属网络数据包池名称
  static constexpr char     dual_packet_pool_name[]                             = "NetX Dual Packet Pool";

  /// @brief 网络管理器 附属网络数据包池内存
  uint8_t        m_dual_packet_pool_memory[DUAL_PACKET_POOL_SIZE] QAQ_ALIGN(32) = { 0 };
  /// @brief 网络管理器 附属网络数据包池
  NX_PACKET_POOL m_dual_packet_pool                                             = { 0 };
#endif /* NX_ENABLE_DUAL_PACKET_POOL */

protected:
  /// @brief 网络管理器 构造函数
  explicit Net_Manager() {}
  /// @brief 网络管理器 析构函数
  ~Net_Manager() {}

public:
  /**
   * @brief  网络管理器 获取单例
   *
   * @return Net_Manager& 网络管理器 单例
   */
  static Net_Manager& instance()
  {
    static Net_Manager instance;
    return instance;
  }

  /**
   * @brief  网络管理器 初始化
   *
   * @param  ip     IP地址(数组)
   * @param  mask   子网掩码(数组)
   * @return true   初始化成功
   * @return false  初始化失败
   */
  bool init(const uint8_t ip[4], const uint8_t mask[4])
  {
    bool     ret        = true;
    uint32_t error_code = 0;

    if (!m_init)
    {
      nx_system_initialize();

      error_code = nx_packet_pool_create(&m_packet_pool, const_cast<char*>(packet_pool_name), (1536 + sizeof(NX_PACKET)), m_packet_pool_memory, PACKET_POOL_SIZE);

      if (NX_SUCCESS != error_code)
      {
#if (SYSTEM_ERROR_LOG_ENABLE && NET_MANAGER_ERROR_LOG_ENABLE)
        QAQ_ERROR_LOG(error_code, "NetX packet pool create error!");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && NET_MANAGER_ERROR_LOG_ENABLE) */
        ret = false;
      }

#ifdef NX_ENABLE_DUAL_PACKET_POOL

      error_code = nx_packet_pool_create(&m_dual_packet_pool, const_cast<char*>(dual_packet_pool_name), (200 + sizeof(NX_PACKET)), m_dual_packet_pool_memory, DUAL_PACKET_POOL_SIZE);

      if (NX_SUCCESS != error_code)
      {
  #if (SYSTEM_ERROR_LOG_ENABLE && NET_MANAGER_ERROR_LOG_ENABLE)
        QAQ_ERROR_LOG(error_code, "NetX dual packet pool create error!");
  #endif /* (SYSTEM_ERROR_LOG_ENABLE && NET_MANAGER_ERROR_LOG_ENABLE) */
        ret = false;
      }

#endif /* NX_ENABLE_DUAL_PACKET_POOL */

      if (ret)
      {
        error_code = nx_ip_create(&m_ip, const_cast<char*>(ip_name), IP_ADDRESS(ip[0], ip[1], ip[2], ip[3]), IP_ADDRESS(mask[0], mask[1], mask[2], mask[3]), &m_packet_pool, nx_stm32_eth_driver, m_ip_pool_memory, IP_POOL_SIZE, 1);

        if (NX_SUCCESS != error_code)
        {
#if (SYSTEM_ERROR_LOG_ENABLE && NET_MANAGER_ERROR_LOG_ENABLE)
          QAQ_ERROR_LOG(error_code, "NetX ip create error!");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && NET_MANAGER_ERROR_LOG_ENABLE) */
          ret = false;
        }
      }

#ifdef NX_ENABLE_DUAL_PACKET_POOL

      if (ret)
      {
        error_code = nx_ip_auxiliary_packet_pool_set(&m_ip, &m_dual_packet_pool);

        if (NX_SUCCESS != error_code)
        {
  #if (SYSTEM_ERROR_LOG_ENABLE && NET_MANAGER_ERROR_LOG_ENABLE)
          QAQ_ERROR_LOG(error_code, "NetX dual packet pool set error!");
  #endif /* (SYSTEM_ERROR_LOG_ENABLE && NET_MANAGER_ERROR_LOG_ENABLE) */
          ret = false;
        }
      }

#endif /* NX_ENABLE_DUAL_PACKET_POOL */

      if (ret)
      {
        error_code = nx_arp_enable(&m_ip, m_arp_pool_memory, ARP_POOL_SIZE);

        if (NX_SUCCESS != error_code)
        {
#if (SYSTEM_ERROR_LOG_ENABLE && NET_MANAGER_ERROR_LOG_ENABLE)
          QAQ_ERROR_LOG(error_code, "NetX arp create error!");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && NET_MANAGER_ERROR_LOG_ENABLE) */
          ret = false;
        }
      }

      if (ret)
      {
        error_code = nx_ip_fragment_enable(&m_ip);

        if (NX_SUCCESS != error_code)
        {
#if (SYSTEM_ERROR_LOG_ENABLE && NET_MANAGER_ERROR_LOG_ENABLE)
          QAQ_ERROR_LOG(error_code, "NetX ip fragment enable error!");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && NET_MANAGER_ERROR_LOG_ENABLE) */
          ret = false;
        }
      }

      if (ret)
      {
        error_code = nx_icmp_enable(&m_ip);

        if (NX_SUCCESS != error_code)
        {
#if (SYSTEM_ERROR_LOG_ENABLE && NET_MANAGER_ERROR_LOG_ENABLE)
          QAQ_ERROR_LOG(error_code, "NetX icmp enable error!");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && NET_MANAGER_ERROR_LOG_ENABLE) */
          ret = false;
        }
      }

      if (ret)
      {
        error_code = nx_igmp_enable(&m_ip);

        if (NX_SUCCESS != error_code)
        {
#if (SYSTEM_ERROR_LOG_ENABLE && NET_MANAGER_ERROR_LOG_ENABLE)
          QAQ_ERROR_LOG(error_code, "NetX igmp enable error!");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && NET_MANAGER_ERROR_LOG_ENABLE) */
          ret = false;
        }
      }
    }

    if (ret)
    {
      m_init = true;
    }

    return ret;
  }

  /**
   * @brief  网络管理器 初始化
   *
   * @param  ip     IP地址(字符串)
   * @return true   初始化成功
   * @return false  初始化失败
   */
  bool init(const uint8_t ip[4])
  {
    const uint8_t mask[4] = { 255, 255, 255, 0 };
    return init(ip, mask);
  }

  /**
   * @brief  网络管理器 初始化
   *
   * @param  ip     IP地址(字符串)
   * @param  mask   子网掩码(字符串)
   * @return true   初始化成功
   * @return false  初始化失败
   */
  bool init(const char* ip = "192.168.1.10", const char* mask = "255.255.255.0")
  {
    uint8_t ip_arr[4]   = { 0 };
    uint8_t mask_arr[4] = { 0 };
    int     parse[4]    = { 0 };
    bool    ret         = true;

    sscanf(ip, "%d.%d.%d.%d", &parse[0], &parse[1], &parse[2], &parse[3]);

    if (parse[0] < 0 || parse[0] > 255)
    {
      ret = false;
    }
    else
    {
      ip_arr[0] = parse[0];
      ip_arr[1] = parse[1];
      ip_arr[2] = parse[2];
      ip_arr[3] = parse[3];
    }

    if (ret)
    {
      sscanf(mask, "%d.%d.%d.%d", &parse[0], &parse[1], &parse[2], &parse[3]);

      if (parse[0] < 0 || parse[0] > 255)
      {
        ret = false;
      }
      else
      {
        mask_arr[0] = parse[0];
        mask_arr[1] = parse[1];
        mask_arr[2] = parse[2];
        mask_arr[3] = parse[3];
      }
    }

    if (ret)
    {
      ret = init(ip_arr, mask_arr);
    }

    return ret;
  }

  /**
   * @brief  网络管理器 启用UDP
   *
   * @return true  启用成功
   * @return false 启用失败
   */
  bool enable_udp()
  {
    bool     ret        = true;
    uint32_t error_code = 0;

    error_code          = nx_udp_enable(&m_ip);

    if (NX_SUCCESS != error_code)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && NET_MANAGER_ERROR_LOG_ENABLE)
      QAQ_ERROR_LOG(error_code, "NetX udp enable error!");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && NET_MANAGER_ERROR_LOG_ENABLE) */
      ret = false;
    }

    return ret;
  }

  /**
   * @brief  网络管理器 启用TCP
   *
   * @return true  启用成功
   * @return false 启用失败
   */
  bool enable_tcp()
  {
    bool     ret        = true;
    uint32_t error_code = 0;

    error_code          = nx_tcp_enable(&m_ip);

    if (NX_SUCCESS != error_code)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && NET_MANAGER_ERROR_LOG_ENABLE)
      QAQ_ERROR_LOG(error_code, "NetX tcp enable error!");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && NET_MANAGER_ERROR_LOG_ENABLE) */
      ret = false;
    }

    return ret;
  }

  /**
   * @brief  网络管理器 获取数据包池指针
   *
   * @return NX_PACKET_POOL* 数据包池指针
   */
  NX_PACKET_POOL* get_pool()
  {
    return &m_packet_pool;
  }

  /**
   * @brief  网络管理器 获取IP指针
   *
   * @return NX_IP* IP指针
   */
  NX_IP* get_ip()
  {
    return &m_ip;
  }
};
} /* namespace net_internal */
} /* namespace net */
} /* namespace QAQ */

#endif /* __NET_MANAGER_HPP__ */
