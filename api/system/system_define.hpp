#ifndef __SYSTEM_DEFINE_HPP__
#define __SYSTEM_DEFINE_HPP__

#include "tx_api.h"

extern TX_THREAD _tx_timer_thread;

/// @brief 宏定义 系统是否处于中断中
#define QAQ_IS_IN_ISR   (__get_IPSR() != 0)
/// @brief 宏定义 系统是否处于软件定时器中
#define QAQ_IS_IN_TIMER (tx_thread_identify() == &_tx_timer_thread)
/// @brief 宏定义 内联函数
#define QAQ_INLINE      inline __attribute__((always_inline))
/// @brief 宏定义 弱函数
#define QAQ_WEAK        __attribute__((weak))
/// @brief 宏定义 对齐至N字节
#define QAQ_ALIGN(N)    __attribute__((aligned(N)))
/// @brief 宏定义 结构体对齐
#define QAQ_PACKED      __attribute__((packed))
/// @brief 宏定义 -O3优化
#define QAQ_O3          __attribute__((optimize("O3")))
/// @brief 宏定义 置于DTCM中运行
#define QAQ_DTCM        __attribute__((section(".dtcm")))
/// @brief 宏定义 置于SRAM1中运行
#define QAQ_SRAM1       __attribute__((section(".sram1")))
/// @brief 宏定义 置于SRAM2中运行
#define QAQ_SRAM2       __attribute__((section(".sram2")))
/// @brief 宏定义 置于SRAM3中运行
#define QAQ_SRAM3       __attribute__((section(".sram3")))
/// @brief 宏定义 置于SRAM4中运行
#define QAQ_SRAM4       __attribute__((section(".sram4")))

/// @brief 宏定义 禁止拷贝
#define NO_COPY(NAME)                    \
  NAME(const NAME&)            = delete; \
  NAME& operator=(const NAME&) = delete;

/// @brief 宏定义 禁止移动
#define QAQ_NO_MOVE(NAME)           \
  NAME(NAME&&)            = delete; \
  NAME& operator=(NAME&&) = delete;

/// @brief 宏定义 禁止拷贝和移动
#define QAQ_NO_COPY_MOVE(NAME)           \
  NAME(const NAME&)            = delete; \
  NAME& operator=(const NAME&) = delete; \
  NAME(NAME&&)                 = delete; \
  NAME& operator=(NAME&&)      = delete;

#endif /* __SYSTEM_DEFINE_HPP__ */
