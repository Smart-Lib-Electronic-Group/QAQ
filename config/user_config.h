#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#if __cplusplus
extern "C"
{
#endif

#define DMA_REGION_START_ADDRESSES 0x30000000
#define DMA_REGION_END_ADDRESSES   0x3001FFFF

/// @brief 内存安全检查
#define MEMORY_SAFETY_CHECKS 0

/// @brief 系统错误日志
#define SYSTEM_ERROR_LOG_ENABLE 1

#if SYSTEM_ERROR_LOG_ENABLE

  /// @brief 事件标志错误日志
  #define EVENT_FLAGS_ERROR_LOG_ENABLE    1
  /// @brief 信号量错误日志
  #define SEMAPHORE_ERROR_LOG_ENABLE      1
  /// @brief 互斥锁错误日志
  #define MUTEX_ERROR_LOG_ENABLE          1
  /// @brief 消息队列错误日志
  #define MESSAGE_QUEUE_ERROR_LOG_ENABLE  1
  /// @brief 内存池错误日志
  #define MEMORY_POOL_ERROR_LOG_ENABLE    1
  /// @brief 线程错误日志
  #define THREAD_ERROR_LOG_ENABLE         1
  /// @brief 软件定时器错误日志
  #define SOFTWARE_TIMER_ERROR_LOG_ENABLE 1
  /// @brief 信号错误日志
  #define SIGNAL_ERROR_LOG_ENABLE         1
  /// @brief 网络管理器错误日志
  #define NET_MANAGER_ERROR_LOG_ENABLE    1
  /// @brief TCP套接字错误日志
  #define TCP_SOCKET_ERROR_LOG_ENABLE     1

#endif /* SYSTEM_ERROR_LOG_ENABLE */

#if __cplusplus
}
#endif

#endif /* __USER_CONFIG_H__ */
