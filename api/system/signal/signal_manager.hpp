#ifndef __SIGNAL_MANAGER_HPP__
#define __SIGNAL_MANAGER_HPP__

#include "memory_pool.hpp"
#include "read_write_lock.hpp"
#include "object_base.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 系统
namespace system
{
/// @brief 名称空间 信号
namespace signal
{
/// @brief 信号类
template <typename... Args>
class Signal;
} /* namespace signal */

/// @brief 名称空间 内部
namespace system_internal
{
/// @brief 名称空间 信号 内部
namespace signal_internal
{
/// @brief 信号管理器
class Signal_Manager;
/// @brief 连接组结构体
struct Connection_Group;

/// @brief 信号 哈希管理器 哈希桶数量
constexpr uint32_t HASH_TABLE_SIZE                       = 64;
/// @brief 信号 最大连接组数量
constexpr uint32_t MAX_CONNECTION_GROUP                  = 64;
/// @brief 信号 最大接收对象节点数量
constexpr uint32_t MAX_RECEIVER_NODE                     = 256;
/// @brief 信号 管理器 小块内存池 块大小
constexpr uint32_t MANAGER_MEMORY_POOL_SMALL_BLOCK_SIZE  = 32;
/// @brief 信号 管理器 小块内存池 块数量
constexpr uint32_t MANAGER_MEMORY_POOL_SMALL_BLOCK_COUNT = 64;
/// @brief 信号 管理器 大块内存池 块大小
constexpr uint32_t MANAGER_MEMORY_POOL_LARGE_BLOCK_SIZE  = 64;
/// @brief 信号 管理器 大块内存池 块数量
constexpr uint32_t MANAGER_MEMORY_POOL_LARGE_BLOCK_COUNT = 32;
/// @brief 信号 管理器 字节内存池 大小
constexpr uint32_t SIGNAL_MEMORY_POOL_BYTE_SIZE          = 1024;
/// @brief 信号 管理器 信号量内存池 大小
constexpr uint32_t SIGNAL_MEMORY_POOL_SEMAPHORE_SIZE     = 32;

/// @brief 信号投递信号方式
enum class Execute_Type : uint8_t
{
  Direct,          /* 直接执行 */
  Object,          /* 投递至对象队列 */
  Thread,          /* 投递至线程队列 */
  Blocking_Direct, /* 阻塞模式 直接执行 */
  Blocking_Object, /* 阻塞模式 投递至对象队列 */
  Blocking_Thread, /* 阻塞模式 投递至线程队列 */
};

/// @brief 接收对象节点结构体
struct Receiver_Node
{
  /// @brief 接收对象指针
  object_internal::Object_Base* receiver;
  /// @brief 关联函数句柄指针
  void*                         handle;
  /// @brief 连接类型
  signal::Connection_Type       type;
  /// @brief 下一个节点指针
  Receiver_Node*                next_node;
  /// @brief 连接组指针
  Connection_Group*             group;

  Receiver_Node() noexcept : receiver(nullptr), handle(nullptr), type(signal::Connection_Type::Direct_Connection), next_node(nullptr), group(nullptr) {}
  ~Receiver_Node() noexcept {}
};

/// @brief 连接组结构体
struct Connection_Group
{
  /// @brief 信号指针
  Signal_Base*      signal;
  /// @brief 接收对象节点指针
  Receiver_Node*    first_receiver;
  /// @brief 下一个连接组指针
  Connection_Group* next_group;
  /// @brief 阻塞连接计数
  uint32_t          blocking_count;

  Connection_Group() noexcept : signal(nullptr), first_receiver(nullptr), next_group(nullptr) {}
  ~Connection_Group() noexcept {}
};

/// @brief 信号量结构体
struct Signal_Semaphore
{
  kernel::Semaphore semaphore;  /* 信号量 */
  uint32_t          total;      /* 信号量总数 */
  uint32_t          used;       /* 信号量已用数 */
  std::atomic_bool  is_timeout; /* 超时标志 */

  Signal_Semaphore() noexcept : semaphore(), total(0), used(0), is_timeout(false) {}
  ~Signal_Semaphore() noexcept {}
};
/// @brief 信号 哈希管理器
class Signal_Hash_Table
{
  // 禁止拷贝与移动
  QAQ_NO_COPY_MOVE(Signal_Hash_Table)

  /// @brief 友元声明 信号管理器
  friend class Signal_Manager;

private:
  /// @brief 哈希桶数组
  Connection_Group*                                                  buckets[HASH_TABLE_SIZE] = { nullptr };
  /// @brief 内存池 管理连接组结构体
  memory::Struct_Memory_Pool<MAX_CONNECTION_GROUP, Connection_Group> m_group_pool;
  /// @brief 内存池 管理接收对象节点结构体
  memory::Struct_Memory_Pool<MAX_RECEIVER_NODE, Receiver_Node>       m_receiver_pool;
  /// @brief 读写锁
  kernel::Read_Write_Lock                                            m_lock;

  /// @brief 共用体 用于提取函数指针
  template <typename Handler>
  union function_extractor
  {
    Handler handler;
    void*   p;
  };

private:
  /**
   * @brief  哈希管理器 构造函数
   *
   */
  explicit Signal_Hash_Table() : m_group_pool("Signal Hash Table Group Pool"), m_receiver_pool("Signal Hash Table Receiver Pool"), m_lock("Signal Hash Table Lock") {}

  /**
   * @brief  哈希管理器 获取信号哈希值
   *
   * @param  signal   信号指针
   * @return uint32_t 哈希值
   */
  QAQ_INLINE static QAQ_O3 uint32_t get_hash(Signal_Base* signal) noexcept
  {
    uintptr_t ptr_val = reinterpret_cast<uintptr_t>(signal);
    return (ptr_val ^ (ptr_val >> 9) ^ (ptr_val >> 18)) & (HASH_TABLE_SIZE - 1);
  }

  /**
   * @brief  信号 哈希管理器 添加连接 (成员函数)
   *
   * @tparam Handler            连接函数类型
   * @param  signal             信号指针
   * @param  receiver           接收对象指针
   * @param  handler            连接函数指针
   * @param  type               连接类型
   * @return Signal_Error_Code  连接结果
   */
  template <typename Handler>
  signal::Signal_Error_Code QAQ_O3 add_connection(Signal_Base* signal, object_internal::Object_Base* receiver, Handler handler, signal::Connection_Type type) noexcept
  {
    if (nullptr == receiver || nullptr == signal || nullptr == handler)
    {
      return signal::Signal_Error_Code::NULL_POINTER;
    }

    kernel::Write_Guard         guard(m_lock);
    function_extractor<Handler> function;
    function.handler        = handler;
    const uint32_t    hash  = get_hash(signal);
    Connection_Group* group = buckets[hash];

    while (group && group->signal != signal)
    {
      group = group->next_group;
    }

    if (nullptr == group)
    {
      group = m_group_pool.allocate();
      if (nullptr == group)
      {
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
        QAQ_ERROR_LOG(signal::Signal_Error_Code::OUT_OF_MEMORY, "Out of memory for connection group.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
        return signal::Signal_Error_Code::OUT_OF_MEMORY;
      }
      group->signal     = signal;
      group->next_group = buckets[hash];
      buckets[hash]     = group;
    }

    Receiver_Node* node = group->first_receiver;
    while (nullptr != node)
    {
      if (node->receiver == receiver && node->handle == function.p)
      {
        return signal::Signal_Error_Code::ALREADY_CONNECTED;
      }
      node = node->next_node;
    }

    Receiver_Node* new_node = m_receiver_pool.allocate();
    if (nullptr == new_node)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
      QAQ_ERROR_LOG(signal::Signal_Error_Code::OUT_OF_MEMORY, "Out of memory for receiver node.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
      return signal::Signal_Error_Code::OUT_OF_MEMORY;
    }

    new_node->receiver    = receiver;
    new_node->handle      = function.p;
    new_node->group       = group;
    new_node->next_node   = group->first_receiver;
    new_node->type        = type;
    group->first_receiver = new_node;

    if (type == signal::Connection_Type::Blocking_Queue_Connection)
      group->blocking_count++;

    return signal::Signal_Error_Code::SUCCESS;
  }

  /**
   * @brief  信号 哈希管理器 添加连接 (全局函数)
   *
   * @tparam Handler            连接函数类型
   * @param  signal             信号指针
   * @param  handler            连接函数指针
   * @return Signal_Error_Code  连接结果
   */
  template <typename Handler>
  signal::Signal_Error_Code QAQ_O3 add_connection(Signal_Base* signal, Handler handler) noexcept
  {
    if (nullptr == signal || nullptr == handler)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
      QAQ_ERROR_LOG(signal::Signal_Error_Code::NULL_POINTER, "Null pointer for signal or handler.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
      return signal::Signal_Error_Code::NULL_POINTER;
    }

    kernel::Write_Guard         guard(m_lock);
    function_extractor<Handler> function;
    function.handler        = handler;
    const uint32_t    hash  = get_hash(signal);
    Connection_Group* group = buckets[hash];
    while (group && group->signal != signal)
    {
      group = group->next_group;
    }

    if (nullptr == group)
    {
      group = m_group_pool.allocate();
      if (nullptr == group)
      {
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
        QAQ_ERROR_LOG(signal::Signal_Error_Code::OUT_OF_MEMORY, "Out of memory for connection group.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
        return signal::Signal_Error_Code::OUT_OF_MEMORY;
      }
      group->signal     = signal;
      group->next_group = buckets[hash];
      buckets[hash]     = group;
    }

    Receiver_Node* node = group->first_receiver;
    while (nullptr != node)
    {
      if (node->receiver == nullptr && node->handle == function.p)
      {
        return signal::Signal_Error_Code::ALREADY_CONNECTED;
      }
      node = node->next_node;
    }

    Receiver_Node* new_node = m_receiver_pool.allocate();
    if (nullptr == new_node)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
      QAQ_ERROR_LOG(signal::Signal_Error_Code::OUT_OF_MEMORY, "Out of memory for receiver node.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
      return signal::Signal_Error_Code::OUT_OF_MEMORY;
    }
    new_node->receiver    = nullptr;
    new_node->handle      = function.p;
    new_node->group       = group;
    new_node->next_node   = group->first_receiver;
    group->first_receiver = new_node;
    return signal::Signal_Error_Code::SUCCESS;
  }

  /**
   * @brief  信号 哈希管理器 获取阻塞连接数量
   *
   * @param  signal   信号指针
   * @return uint32_t 阻塞连接数量
   */
  uint32_t QAQ_O3 get_blocking_connection_count(Signal_Base* signal) noexcept
  {
    if (nullptr == signal)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
      QAQ_ERROR_LOG(signal::Signal_Error_Code::NULL_POINTER, "Null pointer for signal.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
      return 0;
    }

    kernel::Read_Guard guard(m_lock);
    const uint32_t     hash = get_hash(signal);

    for (Connection_Group* group = buckets[hash]; group; group = group->next_group)
    {
      if (group->signal == signal)
      {
        return group->blocking_count;
      }
    }

    return 0;
  }

  /**
   * @brief  信号 哈希管理器 执行连接
   *
   * @tparam Lambda             执行事件Lambda类型
   * @param  signal             信号指针
   * @param  executor           执行事件Lambda
   * @return Signal_Error_Code  执行结果
   */
  template <typename Lambda>
  signal::Signal_Error_Code QAQ_O3 for_each_connection(Signal_Base* signal, Lambda&& executor) noexcept
  {
    if (nullptr == signal)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
      QAQ_ERROR_LOG(signal::Signal_Error_Code::NULL_POINTER, "Null pointer for signal.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
      return signal::Signal_Error_Code::NULL_POINTER;
    }

    kernel::Read_Guard        guard(m_lock);
    signal::Signal_Error_Code result = signal::Signal_Error_Code::RECEIVE_NO_FIND;
    signal::Signal_Error_Code ret    = signal::Signal_Error_Code::SUCCESS;
    const uint32_t            hash   = get_hash(signal);

    for (Connection_Group* group = buckets[hash]; group; group = group->next_group)
    {
      if (group->signal == signal)
      {
        for (Receiver_Node* node = group->first_receiver; node; node = node->next_node)
        {
          result = executor(node);
          if (result != signal::Signal_Error_Code::SUCCESS)
            ret = result;
        }
        break;
      }
    }

    return (result == signal::Signal_Error_Code::RECEIVE_NO_FIND) ? result : ret;
  }

  /**
   * @brief  信号 哈希管理器 移除连接 (成员函数)
   *
   * @tparam Handler  连接函数类型
   * @param  signal   信号指针
   * @param  receiver 接收对象指针
   * @param  handler  连接函数指针
   * @return uint32_t 移除连接数量
   */
  template <typename Handler>
  uint32_t QAQ_O3 remove_connection(Signal_Base* signal, object_internal::Object_Base* receiver, Handler handler) noexcept
  {
    if (nullptr == receiver || nullptr == signal || nullptr == handler)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
      QAQ_ERROR_LOG(signal::Signal_Error_Code::NULL_POINTER, "Null pointer for signal or handler.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
      return 0;
    }

    kernel::Write_Guard         guard(m_lock);
    function_extractor<Handler> function;
    function.handler     = handler;
    uint32_t       count = 0;
    const uint32_t hash  = get_hash(signal);

    for (Connection_Group* group = buckets[hash]; group; group = group->next_group)
    {
      if (group->signal != signal)
      {
        continue;
      }

      Receiver_Node** prev_node = &group->first_receiver;
      while (nullptr != *prev_node)
      {
        if ((*prev_node)->receiver == receiver && (*prev_node)->handle == function.p)
        {
          Receiver_Node* node = *prev_node;
          *prev_node          = node->next_node;
          m_receiver_pool.deallocate(node);

          if (node->type == signal::Connection_Type::Blocking_Queue_Connection)
            group->blocking_count--;

          ++count;
        }
        else
        {
          prev_node = &(*prev_node)->next_node;
        }
      }
    }
    return count;
  }

  /**
   * @brief  信号 哈希管理器 移除连接 (全局函数)
   *
   * @tparam Handler  连接函数类型
   * @param  signal   信号指针
   * @param  handler  连接函数指针
   * @return uint32_t 移除连接数量
   */
  template <typename Handler>
  uint32_t QAQ_O3 remove_connection(Signal_Base* signal, Handler handler) noexcept
  {
    if (nullptr == signal || nullptr == handler)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
      QAQ_ERROR_LOG(signal::Signal_Error_Code::NULL_POINTER, "Null pointer for signal or handler.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
      return 0;
    }

    kernel::Write_Guard         guard(m_lock);
    function_extractor<Handler> function;
    function.handler     = handler;
    uint32_t       count = 0;
    const uint32_t hash  = get_hash(signal);

    for (Connection_Group* group = buckets[hash]; group; group = group->next_group)
    {
      if (group->signal != signal)
      {
        continue;
      }

      Receiver_Node** prev_node = &group->first_receiver;
      while (nullptr != *prev_node)
      {
        if ((*prev_node)->receiver == nullptr && (*prev_node)->handle == function.p)
        {
          Receiver_Node* node = *prev_node;
          *prev_node          = node->next_node;
          m_receiver_pool.deallocate(node);
          ++count;
        }
        else
        {
          prev_node = &(*prev_node)->next_node;
        }
      }
    }
    return count;
  }

  /**
   * @brief  信号 哈希管理器 移除信号
   *
   * @param  signal   信号指针
   * @return uint32_t 移除连接数量
   */
  uint32_t QAQ_O3 remove_signal(Signal_Base* signal) noexcept
  {
    if (nullptr == signal)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
      QAQ_ERROR_LOG(signal::Signal_Error_Code::NULL_POINTER, "Null pointer for signal.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
      return 0;
    }

    kernel::Write_Guard guard(m_lock);
    uint32_t            count      = 0;
    const uint32_t      hash       = get_hash(signal);
    Connection_Group**  prev_group = &buckets[hash];
    while (nullptr != *prev_group)
    {
      if ((*prev_group)->signal == signal)
      {
        Connection_Group* group = *prev_group;
        *prev_group             = group->next_group;

        Receiver_Node* node     = group->first_receiver;
        while (nullptr != node)
        {
          Receiver_Node* tmp = node;
          node               = node->next_node;
          m_receiver_pool.deallocate(tmp);
        }

        m_group_pool.deallocate(group);
        ++count;
        break;
      }
      else
      {
        prev_group = &(*prev_group)->next_group;
      }
    }

    return count;
  }

  /**
   * @brief  信号 哈希管理器 移除接收对象
   *
   * @param  receiver 接收对象指针
   * @return uint32_t 移除连接数量
   */
  uint32_t QAQ_O3 remove_receiver(object_internal::Object_Base* receiver) noexcept
  {
    if (nullptr == receiver)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
      QAQ_ERROR_LOG(signal::Signal_Error_Code::NULL_POINTER, "Null pointer for receiver.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
      return 0;
    }

    kernel::Write_Guard guard(m_lock);
    uint32_t            count = 0;

    for (uint32_t i = 0; i < HASH_TABLE_SIZE; ++i)
    {
      for (Connection_Group* group = buckets[i]; group; group = group->next_group)
      {
        Receiver_Node** prev_node = &group->first_receiver;
        while (nullptr != *prev_node)
        {
          if ((*prev_node)->receiver == receiver)
          {
            Receiver_Node* node = *prev_node;
            *prev_node          = node->next_node;

            if (node->type == signal::Connection_Type::Blocking_Queue_Connection)
              group->blocking_count--;

            m_receiver_pool.deallocate(node);
            ++count;
            break;
          }
          else
          {
            prev_node = &(*prev_node)->next_node;
          }
        }
      }
    }

    return count;
  }

  /**
   * @brief  哈希管理器 析构函数
   *
   */
  ~Signal_Hash_Table()
  {
    for (uint32_t i = 0; i < HASH_TABLE_SIZE; ++i)
    {
      Connection_Group* group = buckets[i];
      while (nullptr != group)
      {
        Connection_Group* next_group = group->next_group;
        Receiver_Node*    node       = group->first_receiver;
        while (nullptr != node)
        {
          Receiver_Node* next_node = node->next_node;
          m_receiver_pool.deallocate(node);
          node = next_node;
        }
        m_group_pool.deallocate(group);
        group = next_group;
      }
    }
  }
};

/**
 * @brief  信号数据结构体 - 非阻塞版本
 *
 * @tparam Args 信号参数类型
 */
template <typename... Args>
struct Signal_Data : public Signal_Data_Base
{
  /// @brief 接收对象指针
  object_internal::Object_Base* receiver;
  /// @brief 连接函数共用体
  union function_type
  {
    void* handle;                                                   /* 源指针 */
    void (object_internal::Object_Base::*member_function)(Args...); /* 成员函数指针 */
    void (*global_function)(Args...);                               /* 全局函数指针 */
  } function;
  /// @brief 信号参数
  std::tuple<Args...> args;

  /// @brief 构造函数
  Signal_Data(Args&&... args) : receiver(nullptr), function { nullptr }, args { std::forward<Args>(args)... } {}

  /**
   * @brief 执行函数
   *
   * @return true  执行成功
   * @return false 执行失败 - 对象已销毁
   */
  bool QAQ_O3 execute(void) override
  {
    if (nullptr == receiver)
    {
      function.global_function(std::get<Args>(args)...);
    }
    else
    {
      if (receiver->is_valid())
        (receiver->*function.member_function)(std::get<Args>(args)...);
      else
        return false;
    }

    return true;
  }

  /**
   * @brief 内存释放
   *
   */
  void QAQ_O3 destroy(void) override
  {
    deallocate(static_cast<void*>(this), sizeof(Signal_Data<Args...>));
  }

  /**
   * @brief 析构函数
   *
   */
  virtual ~Signal_Data() {}
};

/**
 * @brief  信号数据结构体 - 阻塞版本
 *
 * @tparam Args 信号参数类型
 */
template <typename... Args>
struct Signal_Blocking_Data : public Signal_Data_Base
{
  /// @brief 接收对象指针
  object_internal::Object_Base* receiver;
  /// @brief 连接函数共用体
  union function_type
  {
    void* handle;                                                   /* 源指针 */
    void (object_internal::Object_Base::*member_function)(Args...); /* 成员函数指针 */
  } function;
  /// @brief 信号参数
  std::tuple<Args...> args;
  /// @brief 信号量指针
  Signal_Semaphore*   semaphore;

  /// @brief 构造函数
  Signal_Blocking_Data(Signal_Semaphore* semaphore, Args&&... args) : receiver(nullptr), function { nullptr }, args { std::forward<Args>(args)... }, semaphore(semaphore) {}

  /**
   * @brief 执行函数
   *
   * @return true  执行成功
   * @return false 执行失败 - 对象已销毁
   */
  bool QAQ_O3 execute(void) override
  {
    if (receiver->is_valid())
      (receiver->*function.member_function)(std::get<Args>(args)...);
    else
      return false;

    return true;
  }

  /**
   * @brief 信号量释放
   *
   */
  void QAQ_O3 release(void)
  {
    semaphore->semaphore.release();
    semaphore->used++;

    if (semaphore->is_timeout)
    {
      if (semaphore->used == semaphore->total)
        deallocate_semaphore(semaphore);
    }
  }

  /**
   * @brief 内存释放
   *
   */
  void QAQ_O3 destroy(void) override
  {
    release();

    deallocate(static_cast<void*>(this), sizeof(Signal_Blocking_Data<Args...>));
  }

  /**
   * @brief 析构函数
   *
   */
  virtual ~Signal_Blocking_Data() {}
};

/**
 * @brief  信号管理器
 *
 */
class Signal_Manager : public Signal_Manager_Base
{
  // 禁止拷贝和移动
  QAQ_NO_COPY_MOVE(Signal_Manager)

  /// @brief 友元声明 信号类
  template <typename... Args>
  friend class signal::Signal;

private:
  /// @brief 哈希管理器
  Signal_Hash_Table                                                                                      m_hash_table;
  /// @brief 小块内存池
  memory::Block_Memory_Pool<MANAGER_MEMORY_POOL_SMALL_BLOCK_COUNT, MANAGER_MEMORY_POOL_SMALL_BLOCK_SIZE> m_small_block_pool;
  /// @brief 大块内存池
  memory::Block_Memory_Pool<MANAGER_MEMORY_POOL_LARGE_BLOCK_COUNT, MANAGER_MEMORY_POOL_LARGE_BLOCK_SIZE> m_large_block_pool;
  /// @brief 字节内存池
  memory::Byte_Memory_Pool<SIGNAL_MEMORY_POOL_BYTE_SIZE>                                                 m_byte_pool;
  /// @brief 信号量内存池
  memory::Struct_Memory_Pool<SIGNAL_MEMORY_POOL_SEMAPHORE_SIZE, Signal_Semaphore>                        m_semaphore_pool;

private:
  /**
   * @brief 信号管理器 内存申请
   *
   * @param  size    申请大小
   * @return void*   指针
   */
  void* QAQ_O3 allocate(uint32_t size) noexcept
  {
    if (size <= MANAGER_MEMORY_POOL_SMALL_BLOCK_COUNT)
    {
      return m_small_block_pool.allocate();
    }
    else if (size <= MANAGER_MEMORY_POOL_LARGE_BLOCK_SIZE)
    {
      return m_large_block_pool.allocate();
    }
    else
    {
      return m_byte_pool.allocate(size);
    }
  }

  /**
   * @brief 信号管理器 内存释放
   *
   * @param ptr   释放指针
   * @param size  释放大小
   */
  void QAQ_O3 deallocate(void* ptr, uint32_t size) noexcept override
  {
    if (ptr)
    {
      if (size <= MANAGER_MEMORY_POOL_SMALL_BLOCK_COUNT)
      {
        m_small_block_pool.deallocate(ptr);
      }
      else if (size <= MANAGER_MEMORY_POOL_LARGE_BLOCK_SIZE)
      {
        m_large_block_pool.deallocate(ptr);
      }
      else
      {
        m_byte_pool.deallocate(ptr);
      }
    }
  }

  /**
   * @brief 信号管理器 释放信号量
   *
   * @param semaphore 信号量指针
   */
  void QAQ_O3 deallocate_semaphore(void* semaphore) noexcept override
  {
    m_semaphore_pool.deallocate(static_cast<Signal_Semaphore*>(semaphore));
  }

  /**
   * @brief  信号管理器 分析连接类型
   *
   * @param  receiver 接收对象指针
   * @param  type     连接类型
   * @param  current_thread  当前线程
   * @return Execute_Type   连接类型
   */
  Execute_Type determine_execute_type(object_internal::Object_Base* receiver, signal::Connection_Type type, object_internal::Object_Base* current_thread) noexcept
  {
    if (receiver == nullptr)
    {
      return Execute_Type::Direct;
    }
    else
    {
      switch (type)
      {
        case signal::Connection_Type::Direct_Connection :
          return Execute_Type::Direct;
        case signal::Connection_Type::Object_Queue_Connection :
          return (receiver->has_signal_queue()) ? Execute_Type::Object : Execute_Type::Direct;
        case signal::Connection_Type::Thread_Queue_Connection :
          if (receiver->has_affinity_thread())
          {
            if (receiver->get_affinity_thread()->has_signal_queue() && !receiver->is_affinity_thread(current_thread))
              return Execute_Type::Thread;
          }

          return Execute_Type::Direct;
        case signal::Connection_Type::Blocking_Queue_Connection :
          if (receiver->has_affinity_thread())
          {
            if (receiver->is_affinity_thread(current_thread))
              return Execute_Type::Blocking_Direct;

            if (receiver->get_affinity_thread()->has_signal_queue() && !receiver->is_affinity_thread(current_thread))
              return Execute_Type::Blocking_Thread;
          }
          else if (receiver->has_signal_queue())
            return Execute_Type::Blocking_Object;

          return Execute_Type::Blocking_Direct;
        case signal::Connection_Type::Auto_Connection :
        default :
          if (receiver->has_affinity_thread())
          {
            if (receiver->get_affinity_thread()->has_signal_queue() && !receiver->is_affinity_thread(current_thread))
              return Execute_Type::Thread;
          }
          else if (receiver->has_signal_queue())
            return Execute_Type::Object;

          return Execute_Type::Direct;
      }
    }
  }

  /**
   * @brief  信号管理器 执行信号
   *
   * @tparam Args               信号参数类型
   * @param  semaphore          信号量结构体指针
   * @param  receiver           接收对象指针
   * @param  type               执行方式
   * @param  handle             连接函数指针
   * @param  args               信号参数
   * @return Signal_Error_Code  执行结果
   */
  template <typename... Args>
  signal::Signal_Error_Code execute_signal(Signal_Semaphore* semaphore, object_internal::Object_Base* receiver, Execute_Type type, void* handle, Args&&... args) noexcept
  {
    if (type == Execute_Type::Blocking_Direct)
    {
      Signal_Blocking_Data<Args...> package(semaphore, std::forward<Args>(args)...);

      package.function.handle = handle;
      package.receiver        = receiver;

      if (package.execute())
      {
        package.release();
        return signal::Signal_Error_Code::SUCCESS;
      }
      else
      {
        package.release();
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
        QAQ_ERROR_LOG(signal::Signal_Error_Code::OBJECT_DESTROYED, "Object destroyed.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
        return signal::Signal_Error_Code::OBJECT_DESTROYED;
      }
    }
    else if (type == Execute_Type::Blocking_Object || type == Execute_Type::Blocking_Thread)
    {
      constexpr uint32_t size = sizeof(Signal_Blocking_Data<Args...>);
      void*              ptr  = allocate(size);

      if (nullptr == ptr)
      {
        semaphore->semaphore.release();
        semaphore->used++;

        if (semaphore->is_timeout)
        {
          if (semaphore->used == semaphore->total)
            deallocate_semaphore(semaphore);
        }

#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
        QAQ_ERROR_LOG(signal::Signal_Error_Code::OUT_OF_MEMORY, "Out of memory for signal.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
        return signal::Signal_Error_Code::OUT_OF_MEMORY;
      }

      Signal_Blocking_Data<Args...>* package = new (ptr) Signal_Blocking_Data<Args...>(semaphore, std::forward<Args>(args)...);
      package->function.handle               = handle;
      package->receiver                      = receiver;

      if (type == Execute_Type::Blocking_Object)
      {
        if (receiver->post_signal(package))
          return signal::Signal_Error_Code::SUCCESS;
        else
        {
          package->destroy();
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
          QAQ_ERROR_LOG(signal::Signal_Error_Code::QUEUE_FULL, "Signal queue full.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
          return signal::Signal_Error_Code::QUEUE_FULL;
        }
      }
      else if (type == Execute_Type::Blocking_Thread)
      {
        if (receiver->get_affinity_thread()->post_signal(package))
          return signal::Signal_Error_Code::SUCCESS;
        else
        {
          package->destroy();
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
          QAQ_ERROR_LOG(signal::Signal_Error_Code::QUEUE_FULL, "Signal queue full.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
          return signal::Signal_Error_Code::QUEUE_FULL;
        }
      }

      package->destroy();
    }
    else if (type == Execute_Type::Direct)
    {
      Signal_Data<Args...> package(std::forward<Args>(args)...);

      package.function.handle = handle;
      package.receiver        = receiver;

      if (package.execute())
        return signal::Signal_Error_Code::SUCCESS;
      else
      {
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
        QAQ_ERROR_LOG(signal::Signal_Error_Code::OBJECT_DESTROYED, "Object destroyed.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
        return signal::Signal_Error_Code::OBJECT_DESTROYED;
      }
    }
    else if (type == Execute_Type::Object || type == Execute_Type::Thread)
    {
      constexpr uint32_t size = sizeof(Signal_Data<Args...>);
      void*              ptr  = allocate(size);

      if (nullptr == ptr)
      {
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
        QAQ_ERROR_LOG(signal::Signal_Error_Code::OUT_OF_MEMORY, "Out of memory for signal.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
        return signal::Signal_Error_Code::OUT_OF_MEMORY;
      }

      Signal_Data<Args...>* package = new (ptr) Signal_Data<Args...>(std::forward<Args>(args)...);
      package->function.handle      = handle;
      package->receiver             = receiver;

      if (type == Execute_Type::Object)
      {
        if (receiver->post_signal(package))
          return signal::Signal_Error_Code::SUCCESS;
        else
        {
          package->destroy();
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
          QAQ_ERROR_LOG(signal::Signal_Error_Code::QUEUE_FULL, "Signal queue full.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
          return signal::Signal_Error_Code::QUEUE_FULL;
        }
      }
      else if (type == Execute_Type::Thread)
      {
        if (receiver->get_affinity_thread()->post_signal(package))
          return signal::Signal_Error_Code::SUCCESS;
        else
        {
          package->destroy();
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
          QAQ_ERROR_LOG(signal::Signal_Error_Code::QUEUE_FULL, "Signal queue full.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
          return signal::Signal_Error_Code::QUEUE_FULL;
        }
      }

      package->destroy();
    }

    return signal::Signal_Error_Code::TYPE_ERROR;
  }

  /**
   * @brief  信号管理器 分析连接类型是否合法
   *
   * @param  receiver           接收对象指针
   * @param  type               连接类型
   * @return Signal_Error_Code  连接类型是否合法
   */
  signal::Signal_Error_Code check_receiver_and_type(object_internal::Object_Base* receiver, signal::Connection_Type type) noexcept
  {
    if (nullptr == receiver)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
      QAQ_ERROR_LOG(signal::Signal_Error_Code::NULL_POINTER, "Receiver is null pointer.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
      return signal::Signal_Error_Code::NULL_POINTER;
    }

    if (type == signal::Connection_Type::Blocking_Queue_Connection)
    {
      if (receiver->has_affinity_thread())
      {
        if (!receiver->get_affinity_thread()->has_signal_queue())
        {
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
          QAQ_ERROR_LOG(signal::Signal_Error_Code::RECEIVE_AFFINITY_THREAD_NO_QUEUE, "Receiver affinity thread has no signal queue.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
          return signal::Signal_Error_Code::RECEIVE_AFFINITY_THREAD_NO_QUEUE;
        }
      }
      else
      {
        if (!receiver->has_signal_queue())
        {
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
          QAQ_ERROR_LOG(signal::Signal_Error_Code::RECEIVE_NO_QUEUE, "Receiver has no signal queue.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
          return signal::Signal_Error_Code::RECEIVE_NO_QUEUE;
        }
      }
    }
    else if (type == signal::Connection_Type::Object_Queue_Connection)
    {
      if (!receiver->has_signal_queue())
      {
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
        QAQ_ERROR_LOG(signal::Signal_Error_Code::RECEIVE_NO_QUEUE, "Receiver has no signal queue.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
        return signal::Signal_Error_Code::RECEIVE_NO_QUEUE;
      }
    }
    else if (type == signal::Connection_Type::Thread_Queue_Connection)
    {
      if (receiver->has_affinity_thread())
      {
        if (!receiver->get_affinity_thread()->has_signal_queue())
        {
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
          QAQ_ERROR_LOG(signal::Signal_Error_Code::RECEIVE_AFFINITY_THREAD_NO_QUEUE, "Receiver affinity thread has no signal queue.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
          return signal::Signal_Error_Code::RECEIVE_AFFINITY_THREAD_NO_QUEUE;
        }
      }
      else
      {
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
        QAQ_ERROR_LOG(signal::Signal_Error_Code::RECEIVE_NO_AFFINITY_THREAD, "Receiver has no affinity thread.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
        return signal::Signal_Error_Code::RECEIVE_NO_AFFINITY_THREAD;
      }
    }

    return signal::Signal_Error_Code::SUCCESS;
  }

  /**
   * @brief 信号管理器 信号释放
   *
   * @param signal   信号指针
   */
  void QAQ_O3 disconnect_signal(Signal_Base* signal) noexcept override
  {
    m_hash_table.remove_signal(signal);
  }

  /**
   * @brief 信号管理器 接收对象释放
   *
   * @param receiver 接收对象指针
   */
  void QAQ_O3 disconnect_receiver(object_internal::Object_Base* receiver) noexcept override
  {
    m_hash_table.remove_receiver(receiver);
  }

  /**
   * @brief  信号管理器 连接 (成员函数)
   *
   * @tparam Signal             信号类型
   * @tparam Receiver           接收对象类型
   * @tparam Handler            连接函数类型
   * @param  signal             信号指针
   * @param  receiver           接收对象指针
   * @param  handler            连接函数指针
   * @param  type               连接类型
   * @return Signal_Error_Code  连接结果
   */
  template <typename Signal, typename Receiver, typename Handler>
  system::signal::Signal_Error_Code QAQ_O3 connect(Signal* signal, Receiver* receiver, Handler handler, system::signal::Connection_Type type) noexcept
  {
    /// @note 类型检查 是否为信号
    static_assert(std::is_base_of_v<Signal_Base, Signal>, "Signal type mismatch");

    system::signal::Signal_Error_Code error_code = check_receiver_and_type(receiver, type);

    return (system::signal::Signal_Error_Code::SUCCESS == error_code) ? m_hash_table.add_connection(signal, receiver, handler, type) : error_code;
  }

  /**
   * @brief  信号管理器 连接 (全局函数)
   *
   * @tparam Signal             信号类型
   * @tparam Handler            连接函数类型
   * @param  signal             信号指针
   * @param  handler            连接函数指针
   * @return Signal_Error_Code  连接结果
   */
  template <typename Signal, typename Handler>
  system::signal::Signal_Error_Code QAQ_O3 connect(Signal* signal, Handler handler) noexcept
  {
    /// @note 类型检查 是否为信号
    static_assert(std::is_base_of_v<Signal_Base, Signal>, "Signal type mismatch");

    return m_hash_table.add_connection(signal, handler);
  }

  /**
   * @brief  信号管理器 断开连接 (成员函数)
   *
   * @tparam Signal   信号类型
   * @tparam Receiver 接收对象类型
   * @tparam Handler  连接函数类型
   * @param  signal   信号指针
   * @param  receiver 接收对象指针
   * @param  handler  连接函数指针
   * @return uint32_t 移除连接数量
   */
  template <typename Signal, typename Receiver, typename Handler>
  uint32_t QAQ_O3 disconnect(Signal* signal, Receiver* receiver, Handler handler) noexcept
  {
    /// @note 类型检查 是否为信号
    static_assert(std::is_base_of_v<Signal_Base, Signal>, "Signal type mismatch");

    return m_hash_table.remove_connection(signal, receiver, handler);
  }

  /**
   * @brief  信号管理器 断开连接 (全局函数)
   *
   * @tparam Signal   信号类型
   * @tparam Handler  连接函数类型
   * @param  signal   信号指针
   * @param  handler  连接函数指针
   * @return uint32_t 移除连接数量
   */
  template <typename Signal, typename Handler>
  uint32_t QAQ_O3 disconnect(Signal* signal, Handler handler) noexcept
  {
    /// @note 类型检查 是否为信号
    static_assert(std::is_base_of_v<Signal_Base, Signal>, "Signal type mismatch");

    return m_hash_table.remove_connection(signal, handler);
  }

  /**
   * @brief  信号管理器 发送信号
   *
   * @tparam Signal             信号类型
   * @tparam Args               信号参数类型
   * @param  semaphore          信号量结构体指针
   * @param  signal             信号指针
   * @param  args               信号参数
   * @return Signal_Error_Code  发送结果
   */
  template <typename Signal, typename... Args>
  system::signal::Signal_Error_Code QAQ_O3 emit(Signal_Semaphore** semaphore, Signal* signal, Args&&... args) noexcept
  {
    /// @note 类型检查 是否为信号
    static_assert(std::is_base_of_v<Signal_Base, Signal>, "Signal type mismatch");

    kernel::Read_Guard guard(m_hash_table.m_lock);

    uint32_t blocking_count = m_hash_table.get_blocking_connection_count(signal);
    if (0 != blocking_count)
    {
      *semaphore = m_semaphore_pool.allocate();
      if (nullptr == *semaphore)
      {
        blocking_count = 0;
#if (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE)
        QAQ_ERROR_LOG(system::signal::Signal_Error_Code::OUT_OF_MEMORY, "Out of memory for semaphore.");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && SIGNAL_ERROR_LOG_ENABLE) */
        return system::signal::Signal_Error_Code::OUT_OF_MEMORY;
      }
      else
      {
        (*semaphore)->total = blocking_count;
      }
    }

    system_internal::object_internal::Object_Base* current_thread = thread_internal::get_thread_object();
    return m_hash_table.for_each_connection(signal,
                                            [&](Receiver_Node* node) -> system::signal::Signal_Error_Code
                                            {
                                              return execute_signal(*semaphore, node->receiver, determine_execute_type(node->receiver, node->type, current_thread), node->handle, std::forward<Args>(args)...);
                                            });
  }

  /**
   * @brief  信号管理器 构造函数
   *
   */
  explicit Signal_Manager() {}

  /**
   * @brief  信号管理器 析构函数
   *
   */
  ~Signal_Manager() {}

public:
  /**
   * @brief  信号管理器 获取单例
   *
   * @return Signal_Manager 单例引用
   */
  static Signal_Manager& instance(void) noexcept
  {
    static Signal_Manager instance;
    return instance;
  }
};

Signal_Manager_Base* __signal_manager_base = &Signal_Manager::instance();

} /* namespace signal_internal */
} /* namespace system_internal */
} /* namespace system */
} /* namespace QAQ */

#endif /* __SIGNAL_MANAGER_HPP__ */
