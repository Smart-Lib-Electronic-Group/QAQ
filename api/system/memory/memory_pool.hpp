#ifndef __MEMORY_POOL_HPP__
#define __MEMORY_POOL_HPP__

#include "system_include.hpp"
#include "tx_byte_pool.h"
#include "tx_block_pool.h"
#include <new>

/// @brief 宏定义 对齐至N字节
#ifndef QAQ_ALIGN
  #define QAQ_ALIGN(N) __attribute__((aligned(N)))
#endif

/// @brief 宏定义 -O3优化
#ifndef QAQ_O3
  #define QAQ_O3 __attribute__((optimize("QAQ_O3")))
#endif

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 系统
namespace system
{
/// @brief 命名空间 内部
namespace system_internal
{
/// @brief 命名空间 内存 内部
namespace memory_internal
{
/// @brief 内存检查开关
#if MEMORY_SAFETY_CHECKS
constexpr bool enable_checks = true;
#else
constexpr bool enable_checks = false;
#endif

/// @brief 内存池头部魔数
constexpr uint32_t FRONT_MAGIC   = 0xDEADBEEF;
/// @brief 内存池尾部魔数
constexpr uint32_t REAR_MAGIC    = 0xCAFEBABE;
/// @brief 无效魔数
constexpr uint32_t INVALID_MAGIC = 0xBAADF00D;

/// @brief 内存错误代码
enum class Memory_Error_Code : uint8_t
{
  ALLOC_FAILED = 0x25,         /* 内存分配失败 */
  MAGIC_CORRUPTED,             /* 内存池头部或尾部魔数错误 */
  DEALLOC_FAILED,              /* 内存池释放失败 */
  MEMORY_POOL_DESTROYED_ERROR, /* 内存池销毁错误 */
};

/**
 * @brief  内存池基本数据模版结构体
 *
 *
 * @tparam N          内存池大小
 * @tparam Block_Size 块型内存池块大小
 * @tparam T          内存池数据类型
 */
template <uint32_t N, uint32_t Block_Size, typename T>
struct Memory_Data;

/**
 * @brief  内存池基本数据模版结构体 - 字节型特化
 *
 * @tparam N 内存池大小
 */
template <uint32_t N>
struct Memory_Data<N, 1, void>
{
  // 大小检查
  static_assert(N > TX_BYTE_POOL_MIN, "memory pool size too small");

  /// @brief 内存池头部
  struct Header
  {
    uint32_t magic;     /* 内存池头部魔数 */
    uint32_t user_size; /* 用户数据大小 */
  };

  /// @brief 内存池尾部
  struct Footer
  {
    uint32_t magic; /* 内存池尾部魔数 */
  };

  /// @brief 字节型内存池ThreadX对象
  using Pool_t                           = TX_BYTE_POOL;
  /// @brief 内存池是否为块型
  static constexpr bool     is_block     = false;
  /// @brief 内存池大小
  static constexpr uint32_t size         = N;
  /// @brief 内存池体积
  static constexpr uint32_t storage_size = N;

#if MEMORY_SAFETY_CHECKS
  /// @brief 内存池额外数据大小
  static constexpr uint32_t add_size = sizeof(Header) + sizeof(Footer);
#else
  /// @brief 内存池额外数据大小
  static constexpr uint32_t add_size = 0;
#endif
};

/**
 * @brief  内存池基本数据模版结构体 - 块型特化
 *
 * @tparam N           内存池大小
 * @tparam Block_Size  块型内存池块大小
 */
template <uint32_t N, uint32_t Block_Size>
struct Memory_Data<N, Block_Size, void>
{
  // 大小检查
  static_assert(N > 0, "memory pool size too small");
  // 块大小检查
  static_assert(Block_Size > 0, "Block size must be positive");

  /// @brief 内存池头部
  struct Header
  {
    uint32_t magic; /* 内存池头部魔数 */
  };

  /// @brief 内存池尾部
  struct Footer
  {
    uint32_t magic; /* 内存池尾部魔数 */
  };

  /// @brief 块型内存池ThreadX对象
  using Pool_t                   = TX_BLOCK_POOL;
  /// @brief 内存池是否为块型
  static constexpr bool is_block = true;

#if MEMORY_SAFETY_CHECKS
  /// @brief 内存池块大小
  static constexpr uint32_t pool_block_size = sizeof(Header) + Block_Size + sizeof(Footer);
#else
  static constexpr uint32_t pool_block_size = Block_Size;
#endif

  /// @brief 内存池大小
  static constexpr uint32_t size         = N;
  /// @brief 内存池体积
  static constexpr uint32_t storage_size = N * pool_block_size;
};

/**
 * @brief  内存池基本数据模版结构体 - 结构型特化
 *
 * @tparam T 内存池数据类型
 * @tparam N 内存池大小
 */
template <uint32_t N, uint32_t Block_Size, typename T>
struct Memory_Data
{
  // 大小检查
  static_assert(N > 0, "memory pool size too small");
  // 块大小检查
  static_assert(Block_Size > 0, "Block size must be positive");
  // 数据类型大小检查
  static_assert(Block_Size == sizeof(T), "Block size must equal sizeof(T) for struct pool");
  // 数据类型构造检查
  static_assert(std::is_nothrow_default_constructible_v<T>, "T must be nothrow constructible");
  // 数据类型析构检查
  static_assert(std::is_nothrow_destructible_v<T>, "T must be nothrow destructible");

  /// @brief 内存池头部
  struct Header
  {
    uint32_t magic; /* 内存池头部魔数 */
  };

  /// @brief 内存池尾部
  struct Footer
  {
    uint32_t magic; /* 内存池尾部魔数 */
  };

  /// @brief 块型内存池ThreadX对象
  using Pool_t                   = TX_BLOCK_POOL;
  /// @brief 内存池是否为块型
  static constexpr bool is_block = true;

#if MEMORY_SAFETY_CHECKS
  /// @brief 内存池块大小
  static constexpr uint32_t pool_block_size = sizeof(Header) + sizeof(T) + sizeof(Footer);
#else
  /// @brief 内存池块大小
  static constexpr uint32_t pool_block_size = sizeof(T);
#endif

  /// @brief 内存池大小
  static constexpr uint32_t size         = N;
  /// @brief 内存池体积
  static constexpr uint32_t storage_size = N * pool_block_size;
};
} /* namespace memory_internal */
} /* namespace system_internal */

/// @brief 名称空间 内存
namespace memory
{
/**
 * @brief  内存池模板类
 *
 * @tparam N          内存池大小
 * @tparam Block_Size 块型内存池块大小(默认1 - 当池块大小为1时,对应字节型内存池;不为1时,对应块型内存池或结构型内存池)
 * @tparam T          内存池数据类型(默认为void - 为void时,且当池块大小为1时,对应字节型内存池;为void时,且当池块大小不为1时,对应块型内存池;否则,对应结构型内存池)
 * @tparam Align      内存对齐方式 - 默认为32字节对齐
 */
template <uint32_t N, uint32_t Block_Size = 1, typename T = void, uint32_t Align = 32>
class Memory_Pool
{
  // 内存对齐检查
  static_assert(Align >= alignof(uint64_t), "Under-aligned type");
  // 禁止拷贝与移动
  QAQ_NO_COPY_MOVE(Memory_Pool)

private:
  /// @brief 内存池默认名称
  static constexpr const char* default_name = "Memory_Pool";
  /// @brief 内存池基本数据
  using Memory_Base_Data                    = system_internal::memory_internal::Memory_Data<N, Block_Size, T>;
  /// @brief 内存池ThreadX对象类型
  using Memory_Pool_Struct                  = typename Memory_Base_Data::Pool_t;
  /// @brief 内存池错误码
  using Memory_Error_Code                   = system_internal::memory_internal::Memory_Error_Code;

  /// @brief 内存池对象
  mutable Memory_Pool_Struct m_pool_struct;
  /// @brief 内存池数据区
  QAQ_ALIGN(Align) uint8_t m_storage[Memory_Base_Data::storage_size];

private:
  /**
   * @brief  内存池 分配字节型内存
   *
   * @param  size     内存大小
   * @param  timeout  超时时间
   * @return void*    内存指针
   */
  void* allocate_bytes(size_t size, uint32_t timeout) noexcept
  {
    constexpr size_t add_size   = Memory_Base_Data::add_size;
    const size_t     total_size = size + add_size;

    void*      ptr              = nullptr;
    const UINT status           = tx_byte_allocate(&m_pool_struct, &ptr, total_size, timeout);

    if (TX_SUCCESS != status)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      QAQ_ERROR_LOG(Memory_Error_Code::ALLOC_FAILED, "Byte Memory Pool Alloc Failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
      ptr = nullptr;
    }
    else
    {
      if constexpr (system_internal::memory_internal::enable_checks)
      {
        auto* head      = reinterpret_cast<typename Memory_Base_Data::Header*>(ptr);
        head->magic     = system_internal::memory_internal::FRONT_MAGIC;
        head->user_size = size;

        auto* foot      = reinterpret_cast<typename Memory_Base_Data::Footer*>(reinterpret_cast<uint8_t*>(ptr) + sizeof(typename Memory_Base_Data::Header) + size);
        foot->magic     = system_internal::memory_internal::REAR_MAGIC;

        ptr             = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(ptr) + sizeof(typename Memory_Base_Data::Header));
      }
    }

    return ptr;
  }

  /**
   * @brief  内存池 释放字节型内存
   *
   * @param  ptr   内存指针
   */
  void deallocate_bytes(void* ptr) noexcept
  {
    if (nullptr != ptr)
    {
      uint8_t* p = reinterpret_cast<uint8_t*>(ptr);

      if constexpr (system_internal::memory_internal::enable_checks)
      {
        const auto* head = reinterpret_cast<typename Memory_Base_Data::Header*>(p - sizeof(typename Memory_Base_Data::Header));
        if (system_internal::memory_internal::FRONT_MAGIC != head->magic)
        {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
          QAQ_ERROR_LOG(Memory_Error_Code::MAGIC_CORRUPTED, "Byte Memory Pool Dealloc Corrupted");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
          return;
        }

        const auto* foot = reinterpret_cast<typename Memory_Base_Data::Footer*>(p + head->user_size);
        if (system_internal::memory_internal::REAR_MAGIC != foot->magic)
        {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
          QAQ_ERROR_LOG(Memory_Error_Code::MAGIC_CORRUPTED, "Byte Memory Pool Dealloc Corrupted");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
          return;
        }

        head->magic = system_internal::memory_internal::INVALID_MAGIC;
        foot->magic = system_internal::memory_internal::INVALID_MAGIC;
        p           = p - sizeof(typename Memory_Base_Data::Header);
      }

#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      if (TX_SUCCESS != tx_byte_release(p))
      {
        QAQ_ERROR_LOG(Memory_Error_Code::DEALLOC_FAILED, "Byte Memory Pool Dealloc Failed");
      }
#else
      tx_byte_release(p);
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
    }
  }

  /**
   * @brief  内存池 分配块型内存
   *
   * @param  timeout  超时时间
   * @return void*    内存指针
   */
  void* allocate_block(uint32_t timeout) noexcept
  {
    void*      ptr    = nullptr;
    const UINT status = tx_block_allocate(&m_pool_struct, &ptr, timeout);

    if (TX_SUCCESS != status)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      QAQ_ERROR_LOG(Memory_Error_Code::ALLOC_FAILED, "Block Memory Pool Alloc Failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
      ptr = nullptr;
    }
    else
    {
      if constexpr (system_internal::memory_internal::enable_checks)
      {
        auto* head  = reinterpret_cast<typename Memory_Base_Data::Header*>(ptr);
        head->magic = system_internal::memory_internal::FRONT_MAGIC;

        auto* foot  = reinterpret_cast<typename Memory_Base_Data::Footer*>(reinterpret_cast<uint8_t*>(ptr) + Memory_Base_Data::pool_block_size - sizeof(typename Memory_Base_Data::Footer));
        foot->magic = system_internal::memory_internal::REAR_MAGIC;

        ptr         = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(ptr) + sizeof(typename Memory_Base_Data::Header));
      }
    }

    return ptr;
  }

  /**
   * @brief 内存池 释放块型内存
   *
   * @param ptr   内存指针
   */
  void deallocate_block(void* ptr) noexcept
  {
    if (nullptr != ptr)
    {
      uint8_t* p = reinterpret_cast<uint8_t*>(ptr);

      if constexpr (system_internal::memory_internal::enable_checks)
      {
        auto* head = reinterpret_cast<typename Memory_Base_Data::Header*>(p - sizeof(typename Memory_Base_Data::Header));
        if (system_internal::memory_internal::FRONT_MAGIC != head->magic)
        {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
          QAQ_ERROR_LOG(Memory_Error_Code::MAGIC_CORRUPTED, "Block Memory Pool Dealloc Corrupted");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
          return;
        }

        auto* foot = reinterpret_cast<typename Memory_Base_Data::Footer*>(p - sizeof(typename Memory_Base_Data::Header) + Memory_Base_Data::pool_block_size - sizeof(typename Memory_Base_Data::Footer));
        if (system_internal::memory_internal::REAR_MAGIC != foot->magic)
        {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
          QAQ_ERROR_LOG(Memory_Error_Code::MAGIC_CORRUPTED, "Block Memory Pool Dealloc Corrupted");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
          return;
        }

        head->magic = system_internal::memory_internal::INVALID_MAGIC;
        foot->magic = system_internal::memory_internal::INVALID_MAGIC;
        p           = p - sizeof(typename Memory_Base_Data::Header);
      }

#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      if (TX_SUCCESS != tx_block_release(p))
      {
        QAQ_ERROR_LOG(Memory_Error_Code::DEALLOC_FAILED, "Block Memory Pool Dealloc Failed");
      }
#else
      tx_block_release(p);
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
    }
  }

  /**
   * @brief  内存池 分配结构型内存
   *
   * @tparam Args     构造模版参数
   * @param  timeout  超时时间
   * @param  args     构造参数
   * @return T*       内存指针
   */
  template <typename... Args>
  T* allocate_struct(uint32_t timeout, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
  {
    static_assert(std::is_nothrow_constructible_v<T, Args...>, "T must be nothrow constructible with given args");

    void*      ptr    = nullptr;
    const UINT status = tx_block_allocate(&m_pool_struct, &ptr, timeout);

    if (TX_SUCCESS != status)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      QAQ_ERROR_LOG(Memory_Error_Code::ALLOC_FAILED, "Block Memory Pool Alloc Failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
      ptr = nullptr;
    }
    else
    {
      if constexpr (system_internal::memory_internal::enable_checks)
      {
        auto* head  = reinterpret_cast<typename Memory_Base_Data::Header*>(ptr);
        head->magic = system_internal::memory_internal::FRONT_MAGIC;

        auto* foot  = reinterpret_cast<typename Memory_Base_Data::Footer*>(reinterpret_cast<uint8_t*>(ptr) + Memory_Base_Data::pool_block_size - sizeof(typename Memory_Base_Data::Footer));
        foot->magic = system_internal::memory_internal::REAR_MAGIC;

        ptr         = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(ptr) + sizeof(typename Memory_Base_Data::Header));
      }
    }

    return (nullptr == ptr) ? nullptr : new (ptr) T(std::forward<Args>(args)...);
  }

  /**
   * @brief  内存池 释放结构型内存
   *
   * @param  block 内存指针
   */
  void deallocate_struct(T* block) noexcept
  {
    if (nullptr != block)
    {
      uint8_t* ptr = reinterpret_cast<uint8_t*>(block);

      if constexpr (system_internal::memory_internal::enable_checks)
      {
        auto* head = reinterpret_cast<typename Memory_Base_Data::Header*>(ptr - sizeof(typename Memory_Base_Data::Header));
        if (system_internal::memory_internal::FRONT_MAGIC != head->magic)
        {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
          QAQ_ERROR_LOG(Memory_Error_Code::MAGIC_CORRUPTED, "Block Memory Pool Dealloc Corrupted");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
          return;
        }

        auto* foot = reinterpret_cast<typename Memory_Base_Data::Footer*>(ptr - sizeof(typename Memory_Base_Data::Header) + Memory_Base_Data::pool_block_size - sizeof(typename Memory_Base_Data::Footer));
        if (system_internal::memory_internal::REAR_MAGIC != foot->magic)
        {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
          QAQ_ERROR_LOG(Memory_Error_Code::MAGIC_CORRUPTED, "Block Memory Pool Dealloc Corrupted");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
          return;
        }

        head->magic = system_internal::memory_internal::INVALID_MAGIC;
        foot->magic = system_internal::memory_internal::INVALID_MAGIC;
        ptr         = ptr - sizeof(typename Memory_Base_Data::Header);
      }

      block->~T();

#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      if (TX_SUCCESS != tx_block_release(ptr))
      {
        QAQ_ERROR_LOG(Memory_Error_Code::DEALLOC_FAILED, "Block Memory Pool Dealloc Failed");
      }
#else
      tx_block_release(ptr);
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
    }
  }

  /**
   * @brief  内存池 判断指针是否在内存池物理存储范围内
   *
   * @brief  ptr   指针
   * @return true  是
   * @return false 否
   */
  bool QAQ_O3 in_storage_range(const void* ptr) const noexcept
  {
    return (ptr >= m_storage) && (ptr < (m_storage + sizeof(m_storage)));
  }

  /**
   * @brief  内存池 判断指针是否属于当前内存池
   *
   * @brief  ptr   指针
   * @return true  是
   * @return false 否
   */
  bool QAQ_O3 owns_internal(const void* ptr) const noexcept
  {
    if (!in_storage_range(ptr))
      return false;

    if constexpr (Memory_Base_Data::is_block)
    {
      if constexpr (system_internal::memory_internal::enable_checks)
      {
        const uint8_t* p    = reinterpret_cast<const uint8_t*>(ptr);
        const auto*    head = reinterpret_cast<const typename Memory_Base_Data::Header*>(p - sizeof(typename Memory_Base_Data::Header));
        return (head->magic == system_internal::memory_internal::FRONT_MAGIC);
      }
      else
      {
        const uint8_t* p      = reinterpret_cast<const uint8_t*>(ptr);
        const uint8_t* base   = m_storage;
        ptrdiff_t      offset = p - base;
        return (offset % Memory_Base_Data::pool_block_size) == sizeof(typename Memory_Base_Data::Header);
      }
    }
    else
    {
      return true;
    }
  }

public:
  /**
   * @brief  内存池 构造函数
   *
   * @param  name     内存池名称
   */
  explicit Memory_Pool(const char* name = default_name) noexcept
  {
    if (nullptr == name)
    {
      name = default_name;
    }

    if constexpr (system_internal::memory_internal::enable_checks)
    {
      memset(&m_storage, 0, sizeof(m_storage));
    }

    memset(&m_pool_struct, 0, sizeof(m_pool_struct));

    if constexpr (Memory_Base_Data::is_block)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      UINT status = tx_block_pool_create(&m_pool_struct, const_cast<CHAR*>(name), Memory_Base_Data::pool_block_size, m_storage, Memory_Base_Data::storage_size);
      system::System_Monitor::check_status(status, "Memory Pool Creat Failed");
#else
      tx_block_pool_create(&m_pool_struct, const_cast<CHAR*>(name), Memory_Base_Data::pool_block_size, m_storage, Memory_Base_Data::storage_size);
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
    }
    else
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      UINT status = tx_byte_pool_create(&m_pool_struct, const_cast<CHAR*>(name), m_storage, Memory_Base_Data::storage_size);
      system::System_Monitor::check_status(status, "Memory Pool Creat Failed");
#else
      tx_byte_pool_create(&m_pool_struct, const_cast<CHAR*>(name), m_storage, Memory_Base_Data::storage_size);
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
    }
  }

  /**
   * @brief  内存池 申请内存 - 字节型内存池
   *
   * @param  size     内存大小
   * @param  timeout  超时时间
   * @return void*    内存指针
   */
  template <bool Enable = !Memory_Base_Data::is_block>
  std::enable_if_t<Enable, void*> QAQ_O3 allocate(size_t size, uint32_t timeout = TX_NO_WAIT) noexcept
  {
    return allocate_bytes(size, timeout);
  }

  /**
   * @brief  内存池 申请内存 - 块型内存池
   *
   * @param  timeout  超时时间
   * @return void*    内存指针
   */
  template <bool Enable = (Memory_Base_Data::is_block && std::is_same_v<T, void>)>
  std::enable_if_t<Enable, void*> QAQ_O3 allocate(uint32_t timeout = TX_NO_WAIT) noexcept
  {
    return allocate_block(timeout);
  }

  /**
   * @brief  内存池 申请内存 - 结构型内存池
   *
   * @tparam Args     构造模版参数
   * @param  timeout  超时时间
   * @param  args     构造参数
   * @return T*       内存指针
   */
  template <typename... Args, typename U = T, bool Enable = (Memory_Base_Data::is_block && !std::is_same_v<T, void>)>
  std::enable_if_t<Enable, U*> QAQ_O3 allocate(uint32_t timeout = TX_NO_WAIT, Args&&... args)
  {
    static_assert(std::is_same_v<U, T>, "Block type mismatch");
    return allocate_struct(timeout, std::forward<Args>(args)...);
  }

  /**
   * @brief  内存池 释放内存 - 字节型内存池
   *
   * @param  ptr   内存指针
   */
  template <bool Enable = !Memory_Base_Data::is_block>
  std::enable_if_t<Enable, void> QAQ_O3 deallocate(void* ptr) noexcept
  {
    deallocate_bytes(ptr);
  }

  /**
   * @brief  内存池 释放内存 - 块型内存池
   *
   * @param  ptr   内存指针
   */
  template <bool Enable = Memory_Base_Data::is_block>
  std::enable_if_t<Enable && std::is_same_v<T, void>, void> QAQ_O3 deallocate(void* ptr) noexcept
  {
    deallocate_block(ptr);
  }

  /**
   * @brief  内存池 释放内存 - 结构型内存池
   *
   * @param  ptr   内存指针
   */
  template <typename U = T, bool Enable = Memory_Base_Data::is_block>
  std::enable_if_t<Enable && !std::is_same_v<T, void>, void> QAQ_O3 deallocate(U* ptr) noexcept
  {
    static_assert(std::is_same_v<U, T>, "Block type mismatch");
    deallocate_struct(ptr);
  }

  /**
   * @brief  内存池 获取内存池容量
   *
   * @return uint32_t   内存池容量
   */
  static constexpr uint32_t get_total_capacity(void) noexcept
  {
    return Memory_Base_Data::size;
  }

  /**
   * @brief  内存池 获取内存块大小
   *
   * @return uint32_t   内存块大小
   */
  static constexpr uint32_t get_block_size(void) noexcept
  {
    if constexpr (Memory_Base_Data::is_block)
    {
      return Memory_Base_Data::pool_block_size;
    }
    else
    {
      return 1;
    }
  }

  /**
   * @brief  内存池 获取存储区大小
   *
   * @return uint32_t   存储区大小
   */
  static constexpr uint32_t storage_bytes(void) noexcept
  {
    return Memory_Base_Data::storage_size;
  }

  /**
   * @brief  内存池 获取内存池剩余大小
   *
   * @return uint32_t   剩余大小
   */
  uint32_t get_available(void) const noexcept
  {
    ULONG available = 0;

    if constexpr (Memory_Base_Data::is_block)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      const UINT status = tx_block_pool_info_get(&m_pool_struct, reinterpret_cast<CHAR**>(TX_NULL), &available, reinterpret_cast<ULONG*>(TX_NULL), reinterpret_cast<TX_THREAD**>(TX_NULL), reinterpret_cast<ULONG*>(TX_NULL), reinterpret_cast<TX_BLOCK_POOL**>(TX_NULL));

      if (TX_SUCCESS != status)
      {
        system::System_Monitor::check_status(status, "Memory Pool Info Get Failed");
        available = 0;
      }
#else
      tx_block_pool_info_get(&m_pool_struct, reinterpret_cast<CHAR**>(TX_NULL), &available, reinterpret_cast<ULONG*>(TX_NULL), reinterpret_cast<TX_THREAD**>(TX_NULL), reinterpret_cast<ULONG*>(TX_NULL), reinterpret_cast<TX_BLOCK_POOL**>(TX_NULL));
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
    }
    else
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      const UINT status = tx_byte_pool_info_get(&m_pool_struct, reinterpret_cast<CHAR**>(TX_NULL), &available, reinterpret_cast<ULONG*>(TX_NULL), reinterpret_cast<TX_THREAD**>(TX_NULL), reinterpret_cast<ULONG*>(TX_NULL), reinterpret_cast<TX_BYTE_POOL**>(TX_NULL));

      if (TX_SUCCESS != status)
      {
        system::System_Monitor::check_status(status, "Memory Pool Info Get Failed");
        available = 0;
      }
#else
      tx_byte_pool_info_get(&m_pool_struct, reinterpret_cast<CHAR**>(TX_NULL), &available, reinterpret_cast<ULONG*>(TX_NULL), reinterpret_cast<TX_THREAD**>(TX_NULL), reinterpret_cast<ULONG*>(TX_NULL), reinterpret_cast<TX_BYTE_POOL**>(TX_NULL));
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
    }

    return static_cast<uint32_t>(available);
  }

  /**
   * @brief   内存池 获取字节池中空闲内存区域的数量（即外部碎片数量）。数量越多，碎片越严重。
   *
   * @warning 仅字节池支持；块型池无此概念。
   * @return  uint32_t   空闲内存块数量（碎片数）
   */
  uint32_t get_fragments(void) const noexcept
  {
    static_assert(!Memory_Base_Data::is_block, "get_fragments() is only available for byte pools");

    ULONG fragments = 0;
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
    const UINT status = tx_byte_pool_info_get(&m_pool_struct, reinterpret_cast<CHAR**>(TX_NULL), reinterpret_cast<ULONG*>(TX_NULL), &fragments, reinterpret_cast<TX_THREAD**>(TX_NULL), reinterpret_cast<ULONG*>(TX_NULL), reinterpret_cast<TX_BYTE_POOL**>(TX_NULL));

    if (TX_SUCCESS != status)
    {
      QAQ_ERROR_LOG(status, "Memory Pool Info Get Failed");
      fragments = 0;
    }
#else
    tx_byte_pool_info_get(&m_pool_struct, reinterpret_cast<CHAR**>(TX_NULL), reinterpret_cast<ULONG*>(TX_NULL), &fragments, reinterpret_cast<TX_THREAD**>(TX_NULL), reinterpret_cast<ULONG*>(TX_NULL), reinterpret_cast<TX_BYTE_POOL**>(TX_NULL));
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
    return fragments;
  }

  /**
   * @brief  内存池 获取内存池已用大小
   *
   * @return uint32_t   已用大小
   */
  uint32_t QAQ_O3 get_used(void) const noexcept
  {
    return get_total_capacity() - get_available();
  }

  /**
   * @brief  内存池 获取内存池空闲率(百分比)
   *
   * @return float      空闲率(百分比)
   */
  float QAQ_O3 get_available_percent(void) const noexcept
  {
    return static_cast<float>(get_available()) / static_cast<float>(get_total_capacity()) * 100.0f;
  }

  /**
   * @brief  内存池 获取内存池使用率(百分比)
   *
   * @return float      使用率(百分比)
   */
  float QAQ_O3 get_used_percent(void) const noexcept
  {
    return static_cast<float>(get_used()) / static_cast<float>(get_total_capacity()) * 100.0f;
  }

  /**
   * @brief  内存池 判断指针是否位于当前内存池管理的物理地址范围内
   *
   * @param  ptr    待检测指针
   * @return true   指针位于当前内存池管理物理地址范围内
   * @return false  指针不在当前内存池管理物理地址范围内
   */
  bool QAQ_O3 is_contains(const void* ptr) const noexcept
  {
    return in_storage_range(ptr);
  }

  /**
   * @brief  内存池 判断指针是否由当前内存池分配（即“属于”当前池）
   *
   * @warning 字节池无法进行魔数检测，此时该函数等效于 is_contains()
   * @param   ptr    待检测指针
   * @return  true   指针由当前内存池分配
   * @return  false  指针不属于当前内存池
   */
  bool QAQ_O3 is_owns(const void* ptr) const noexcept
  {
    return owns_internal(ptr);
  }

  /**
   * @brief  内存池 判断内存池是否已满（无可用块/字节）
   *
   * @return bool
   */
  bool QAQ_O3 is_full(void) const noexcept
  {
    return get_available() == 0;
  }

  /**
   * @brief  内存池 判断内存池是否为空（全未分配）
   *
   * @return bool
   */
  bool QAQ_O3 is_empty(void) const noexcept
  {
    return get_available() == get_total_capacity();
  }

  /**
   * @brief  内存池 判断当前内存池是否为块状内存池
   *
   * @return true  块状内存池
   * @return false 非块状内存池
   */
  static constexpr bool is_block_pool(void) noexcept
  {
    return Memory_Base_Data::is_block;
  }

  /**
   * @brief  内存池 析构函数
   *
   */
  ~Memory_Pool() noexcept
  {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
    uint32_t available = get_available();
    if (available != get_total_capacity())
    {
      QAQ_ERROR_LOG(Memory_Error_Code::MEMORY_POOL_DESTROYED_ERROR, "Memory pool destroyed with blocks still allocated!");
    }
#endif

    if constexpr (Memory_Base_Data::is_block)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      const UINT status = tx_block_pool_delete(&m_pool_struct);
      system::System_Monitor::check_status(status, "Memory Pool Delete Failed");
#else
      tx_block_pool_delete(&m_pool_struct);
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
    }
    else
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      const UINT status = tx_byte_pool_delete(&m_pool_struct);
      system::System_Monitor::check_status(status, "Memory Pool Delete Failed");
#else
      tx_byte_pool_delete(&m_pool_struct);
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
    }
  }
};

template <uint32_t Size, uint32_t Align = 32>
using Byte_Memory_Pool = Memory_Pool<Size, 1, void, Align>;

template <uint32_t Size, uint32_t Block_Size, uint32_t Align = 32>
using Block_Memory_Pool = Memory_Pool<Size, Block_Size, void, Align>;

template <uint32_t Size, typename T, uint32_t Align = 32>
using Struct_Memory_Pool = Memory_Pool<Size, sizeof(T), T, Align>;

} /* namespace memory */
} /* namespace system */
} /* namespace QAQ */

#endif /* __MEMORY_POOL_HPP__ */
