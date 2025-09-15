#ifndef __SOFT_TIMER_HPP__
#define __SOFT_TIMER_HPP__

#include "system_include.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 系统
namespace system
{
/// @brief 命名空间 内部
namespace system_internal
{
/// @brief 命名空间 软件定时器 内部
namespace soft_timer_internal
{
/**
 * @brief 定时器回调函数模板
 *
 * @tparam T    定时器类型
 * @param  arg  定时器参数
 */
template <typename T>
void timer_callback(ULONG arg)
{
  auto* timer = reinterpret_cast<T*>(arg);
  timer->callback();
}

/**
 * @brief  软件定时器基类 - CRTP基类模板
 *
 * @tparam Derived 派生类类型
 */
template <typename Derived>
class Soft_Timer_Base
{
  // 禁止拷贝与移动
  QAQ_NO_COPY_MOVE(Soft_Timer_Base)

private:
  /// @brief 定时器句柄
  TX_TIMER m_timer;
  /// @brief 周期计数
  uint32_t m_cycle_ticks;
  /// @brief 是否自动重载
  bool     m_is_auto_reload;

protected:
  /**
   * @brief 软件定时器基类 构造函数
   *
   * @param cycle_ticks     周期计数
   * @param is_auto_reload  是否自动重载
   */
  explicit Soft_Timer_Base(uint32_t cycle_ticks, bool is_auto_reload = true) noexcept : m_cycle_ticks(cycle_ticks), m_is_auto_reload(is_auto_reload) {}

  /**
   * @brief  线程基类 定时器删除
   *
   */
  virtual ~Soft_Timer_Base() noexcept
  {
    stop();
    system::System_Monitor::safe_execute(tx_timer_delete, &m_timer);
  }

public:
  /**
   * @brief 线程基类 定时器创建
   *
   * @param  name             定时器名称(使用宏定义或静态数组定义)
   * @param  is_auto_activate 是否自动激活
   * @return Derived&         定时器对象
   */
  Derived& QAQ_O3 create(const char* name, bool is_auto_activate = false) noexcept
  {
    const UINT status = tx_timer_create(&m_timer, const_cast<CHAR*>(name), system_internal::soft_timer_internal::timer_callback<Derived>, reinterpret_cast<ULONG>(static_cast<Derived*>(this)), m_cycle_ticks, m_is_auto_reload ? m_cycle_ticks : 0, is_auto_activate ? TX_AUTO_ACTIVATE : TX_NO_ACTIVATE);
    system::System_Monitor::check_status(status, "Failed to create timer");
    return static_cast<Derived&>(*this);
  }

  /**
   * @brief  线程基类 定时器激活
   *
   * @return Derived&         定时器对象
   */
  Derived& QAQ_O3 start(void) noexcept
  {
    system::System_Monitor::safe_execute(tx_timer_activate, &m_timer);
    return static_cast<Derived&>(*this);
  }

  /**
   * @brief  线程基类 定时器停止
   *
   * @return Derived&         定时器对象
   */
  Derived& QAQ_O3 stop(void) noexcept
  {
    system::System_Monitor::safe_execute(tx_timer_deactivate, &m_timer);
    return static_cast<Derived&>(*this);
  }

  /**
   * @brief  线程基类 定时器周期设置
   *
   * @param  cycle_ticks      周期计数
   * @return Derived&         定时器对象
   */
  Derived& QAQ_O3 set_cycle_ticks(uint32_t cycle_ticks) noexcept
  {
    m_cycle_ticks = cycle_ticks;
    system::System_Monitor::safe_execute(tx_timer_change, &m_timer, m_cycle_ticks, m_is_auto_reload ? m_cycle_ticks : 0);
    return static_cast<Derived&>(*this);
  }

  /**
   * @brief  线程基类 定时器是否激活
   *
   * @note   该函数不可在回调函数中调用
   * @return true             定时器激活
   * @return false            定时器未激活
   */
  bool QAQ_O3 is_active(void) const noexcept
  {
    UINT active;
    system::System_Monitor::safe_execute(tx_timer_info_get, &m_timer, TX_NULL, active, TX_NULL, TX_NULL, TX_NULL);
    return active != TX_FALSE;
  }

  /**
   * @brief  线程基类 定时器当前计数值
   *
   * @return uint32_t         定时器当前计数值
   */
  uint32_t QAQ_O3 now_tick(void) const noexcept
  {
    ULONG ticks;
    system::System_Monitor::safe_execute(tx_timer_info_get, &m_timer, TX_NULL, TX_NULL, &ticks, TX_NULL, TX_NULL);
    return ticks;
  }
};
} /* namespace soft_timer_internal */
} /* namespace system_internal */

/**
 * @brief  软件定时器模板
 *
 * @tparam Callable 回调函数类型
 * @tparam Args     回调函数参数类型
 */
template <typename Callable, typename... Args>
class Soft_Timer;

/**
 * @brief  软件定时器模板 - CRTP特化
 *
 * @tparam Derived 派生类类型
 * @note   软件定时器模板的CRTP特化版,用于创建软件定时器对象,派生类需实现callback()函数
 */
template <typename Derived>
class Soft_Timer<Derived> : public system_internal::soft_timer_internal::Soft_Timer_Base<Derived>
{
  // 禁止拷贝与移动
  QAQ_NO_COPY_MOVE(Soft_Timer)

public:
  /**
   * @brief  软件定时器 构造函数
   *
   * @param  cycle_ticks     周期计数
   * @param  is_auto_reload  是否自动重载
   */
  explicit Soft_Timer(uint32_t cycle_ticks, bool is_auto_reload) noexcept : system_internal::soft_timer_internal::Soft_Timer_Base<Derived>(cycle_ticks, is_auto_reload) {}

  /**
   * @brief  软件定时器 析构函数
   *
   */
  virtual ~Soft_Timer() {}
};

/**
 * @brief  软件定时器模板 - 函数指针特化
 *
 * @tparam Params 回调函数参数类型
 * @tparam Args   回调函数参数类型
 */
template <typename... Params, typename... Args>
class Soft_Timer<void (*)(Params...), Args...> : public system_internal::soft_timer_internal::Soft_Timer_Base<Soft_Timer<void (*)(Params...), Args...>>
{
  // 检查是否为函数指针
  static_assert(std::is_pointer_v<void (*)(Params...)>, "Function pointer required");
  // 检查函数指针参数数量是否一致
  static_assert(sizeof...(Params) == sizeof...(Args), "Function pointer parameter count mismatch");

  // 禁止拷贝与移动
  QAQ_NO_COPY_MOVE(Soft_Timer)

private:
  /// @brief 友元函数声明 定时器回调函数模板
  template <typename T>
  friend void system_internal::soft_timer_internal::timer_callback(ULONG arg);

  // 回调函数类型
  using func_t = void (*)(Params...);

  /// @brief 回调函数
  func_t              m_func;
  /// @brief 回调函数参数
  std::tuple<Args...> m_args;

private:
  /**
   * @brief  软件定时器 回调函数
   *
   */
  void QAQ_O3 callback(void)
  {
    m_func(std::get<Args>(m_args)...);
  }

public:
  /**
   * @brief 软件定时器模板 构造函数
   *
   * @param cycle_ticks     周期计数
   * @param is_auto_reload  是否自动重载
   * @param func            回调函数
   * @param args            回调函数参数
   */
  explicit Soft_Timer(uint32_t cycle_ticks, bool is_auto_reload, func_t func, Args&&... args) noexcept : system_internal::soft_timer_internal::Soft_Timer_Base<Soft_Timer<void (*)(Params...), Args...>>(cycle_ticks, is_auto_reload), m_func(func), m_args(std::forward<Args>(args)...) {}

  /**
   * @brief  软件定时器 回调函数设置
   *
   * @param  func         回调函数
   * @param  args         回调函数参数
   * @return Soft_Timer&  定时器对象
   */
  Soft_Timer& QAQ_O3 set_function(func_t func, Args&&... args) noexcept
  {
    bool is_active = this->is_active();
    if (is_active)
    {
      this->stop();
    }

    m_func = func;
    m_args = std::tuple<Args...>(std::forward<Args>(args)...);

    if (is_active)
    {
      this->start();
    }

    return *this;
  }

  /**
   * @brief  软件定时器 执行回调函数
   *
   * @param  args         回调函数参数
   * @return Soft_Timer&  定时器对象
   */
  Soft_Timer& QAQ_O3 operator()(Args&&... args) noexcept
  {
    m_func(std::get<Args>(m_args)...);
    return *this;
  }

  /**
   * @brief  软件定时器 析构函数
   *
   */
  virtual ~Soft_Timer() {}
};

/**
 * @brief  软件定时器模板 - Lambda特化
 *
 * @tparam Lambda Lambda表达式类型
 * @tparam Args   Lambda表达式参数类型
 */
template <typename Lambda, typename... Args>
class Soft_Timer : public system_internal::soft_timer_internal::Soft_Timer_Base<Soft_Timer<Lambda, Args...>>
{
  // 检查是否为Lambda表达式
  static_assert(std::is_same_v<Lambda, std::decay_t<Lambda>>, "Lambda expression required");
  // 检查Lambda表达式返回类型是否为void
  static_assert(std::is_same_v<void, std::result_of_t<Lambda(Args...)>>, "Lambda expression must return void");

  // 禁止拷贝与移动
  QAQ_NO_COPY_MOVE(Soft_Timer)

private:
  /// @brief 友元函数声明 定时器回调函数模板
  template <typename T>
  friend void system_internal::soft_timer_internal::timer_callback(ULONG arg);

  /// @brief Lambda表达式
  Lambda              m_lambda;
  /// @brief Lambda表达式参数
  std::tuple<Args...> m_args;

private:
  /**
   * @brief  软件定时器 回调函数
   *
   */
  void callback(void)
  {
    m_lambda(std::get<Args>(m_args)...);
  }

public:
  /**
   * @brief 软件定时器 构造函数
   *
   * @param cycle_ticks     周期计数
   * @param is_auto_reload  是否自动重载
   * @param lambda          Lambda表达式
   * @param args            Lambda表达式参数
   */
  explicit Soft_Timer(uint32_t cycle_ticks, bool is_auto_reload, Lambda&& lambda, Args&&... args) noexcept : system_internal::soft_timer_internal::Soft_Timer_Base<Soft_Timer<Lambda, Args...>>(cycle_ticks, is_auto_reload), m_lambda(std::forward<Lambda>(lambda)), m_args(std::forward<Args>(args)...) {}

  /**
   * @brief  软件定时器 Lambda表达式设置
   *
   * @param  lambda       Lambda表达式
   * @param  args         Lambda表达式参数
   * @return Soft_Timer&  定时器对象
   */
  Soft_Timer& QAQ_O3 set_lambda(Lambda&& lambda, Args&&... args) noexcept
  {
    bool is_active = this->is_active();
    if (is_active)
    {
      this->stop();
    }

    m_lambda = std::forward<Lambda>(lambda);
    m_args   = std::tuple<Args...>(std::forward<Args>(args)...);

    if (is_active)
    {
      this->start();
    }

    return *this;
  }

  /**
   * @brief  软件定时器 执行Lambda表达式
   *
   * @param  args         Lambda表达式参数
   * @return Soft_Timer&  定时器对象
   */
  Soft_Timer& QAQ_O3 operator()(Args&&... args) noexcept
  {
    m_lambda(std::get<Args>(m_args)...);
    return *this;
  }

  /**
   * @brief  软件定时器 析构函数
   *
   */
  virtual ~Soft_Timer() {}
};

/**
 * @brief  软件定时器模板 - 函数指针模版推导引导
 *
 * @tparam Params 回调函数参数类型
 * @tparam Args   回调函数参数类型
 */
template <typename... Params, typename... Args>
Soft_Timer(ULONG, bool, void (*)(Params...), Args...) -> Soft_Timer<void (*)(Params...), Args...>;

/**
 * @brief  软件定时器模板 - Lambda模版推导引导
 *
 * @tparam Lambda  Lambda表达式类型
 * @tparam Args    Lambda表达式参数类型
 */
template <typename Lambda, typename... Args>
Soft_Timer(ULONG, bool, Lambda, Args...) -> Soft_Timer<Lambda, Args...>;

} /* namespace system */
} /* namespace QAQ */

#endif /* __SOFT_TIMER_HPP__ */
