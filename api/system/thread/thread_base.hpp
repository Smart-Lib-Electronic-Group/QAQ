#ifndef __THREAD_BASE_HPP__
#define __THREAD_BASE_HPP__

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 系统
namespace system
{
/// @brief 命名空间 内部
namespace system_internal
{
/// @brief 命名空间 线程内部
namespace thread_internal
{
/**
 * @brief 线程基类
 *
 */
class Thread_Base
{
  // 禁止拷贝和移动
  NO_COPY_MOVE(Thread_Base)

public:
  /**
   * @brief 线程基类 构造函数
   *
   */
  Thread_Base() {}
  /**
   * @brief 线程基类 析构函数
   *
   */
  virtual ~Thread_Base() {}
};
} /* namespace thread_internal */
} /* namespace system_internal */
} /* namespace system */
} /* namespace QAQ */

#endif /* __THREAD_BASE_HPP__ */
