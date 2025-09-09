#ifndef __NET_BASE_HPP__
#define __NET_BASE_HPP__

#include "system_include.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 网络
namespace net
{
/// @brief 服务器错误码
enum class Server_Error_Code : uint8_t
{
  SUCCESS        = 0x00, /* 成功 */
  NO_FREE_CLIENT = 0xf9, /* 没有空闲客户端 */
  CLIENT_NOT_CLOSE,      /* 客户端未关闭 */
  INIT_ERROR,            /* 初始化错误 */
  PORT_ERROR,            /* 端口错误 */
  LISTEN_FAILED,         /* 监听失败 */
  RELISTEN_FAILED,       /* 重新监听失败 */
  ACCEPT_FAILED,         /* 接受失败 */
};

/// @brief 网络 配置信息选项
class Config
{
public:
  /// @brief IP
  static constexpr uint32_t IP   = 0x01;
  /// @brief 端口号
  static constexpr uint32_t PORT = 0x02;
};

/// @brief 名称空间 网络 内部
namespace net_internal
{
/// @brief 服务器事件
enum class Server_Event : uint8_t
{
  Connect,            /* 连接 */
  Receive,            /* 接收 */
  Disconnect,         /* 断开 */
  Check_Timeout,      /* 检查超时 */
  Signal_Post_Notify, /* 信号通知 */
};
} /* namespace net_internal */
} /* namespace net */
} /* namespace QAQ */

#endif /* __NET_BASE_HPP__ */
