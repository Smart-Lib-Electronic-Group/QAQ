#ifndef __MEMORY_POOL_HPP__
#define __MEMORY_POOL_HPP__

#include "system_include.hpp"
#include <new>

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
namespace merory_internal
{
/// @brief 内存检查开关
#if MEMORY_SAFETY_CHECKS
constexpr bool enable_checks = true;
#else
constexpr bool enable_checks = false;
#endif

/// @brief 内存池头部魔数
constexpr uint32_t FRONT_MAGIC = 0xDEADBEEF;
/// @brief 内存池尾部魔数
constexpr uint32_t REAR_MAGIC  = 0xCAFEBABE;

/// @brief 内存错误代码
enum class Memory_Error_Code : uint8_t
{
  ALLOC_FAILED = 0x25, /* 内存分配失败 */
  MAGIC_CORRUPTED,     /* 内存池头部或尾部魔数错误 */
  DEALLOC_FAILED,      /* 内存池释放失败 */
};

/**
 * @brief  内存池基本数据模版结构体
 *
 * @tparam T          内存池数据类型
 * @tparam N          内存池大小
 * @tparam Block_Size 块型内存池块大小
 */
template <typename T, uint32_t N, uint32_t Block_Size>
struct Memory_Data;

/**
 * @brief  内存池基本数据模版结构体 - 字节型特化
 *
 * @tparam N 内存池大小
 */
template <uint32_t N>
struct Memory_Data<void, N, 1>
{
  // 大小检查
  static_assert(N > 100, "memory pool size too small");

  /// @brief 内存池头部
  struct header
  {
    uint32_t magic;     /* 内存池头部魔数 */
    uint32_t user_size; /* 用户数据大小 */
  };

  /// @brief 内存池尾部
  struct footer
  {
    uint32_t magic; /* 内存池尾部魔数 */
  };

  /// @brief 字节型内存池ThreadX对象
  using pool_t                           = TX_BYTE_POOL;
  /// @brief 内存池是否为块型
  static constexpr bool     is_block     = false;
  /// @brief 内存池大小
  static constexpr uint32_t size         = N;
  /// @brief 内存池体积
  static constexpr uint32_t storage_size = N;

#if MEMORY_SAFETY_CHECKS
  /// @brief 内存池额外数据大小
  static constexpr uint32_t add_size = sizeof(header) + sizeof(footer);
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
struct Memory_Data<void, N, Block_Size>
{
  // 大小检查
  static_assert(N > 0, "memory pool size too small");

  /// @brief 内存池头部
  struct header
  {
    uint32_t magic; /* 内存池头部魔数 */
  };

  /// @brief 内存池尾部
  struct footer
  {
    uint32_t magic; /* 内存池尾部魔数 */
  };

  /// @brief 块型内存池ThreadX对象
  using pool_t                   = TX_BLOCK_POOL;
  /// @brief 内存池是否为块型
  static constexpr bool is_block = true;

#if MEMORY_SAFETY_CHECKS
  /// @brief 内存池块大小
  static constexpr uint32_t block_size = sizeof(header) + Block_Size + sizeof(footer);
#else
  static constexpr uint32_t block_size = Block_Size;
#endif

  /// @brief 内存池大小
  static constexpr uint32_t size         = N;
  /// @brief 内存池体积
  static constexpr uint32_t storage_size = N * block_size;
};

/**
 * @brief  内存池基本数据模版结构体 - 结构型特化
 *
 * @tparam T 内存池数据类型
 * @tparam N 内存池大小
 */
template <typename T, uint32_t N, uint32_t Block_Size = sizeof(T)>
struct Memory_Data
{
  // 大小检查
  static_assert(N > 0, "memory pool size too small");

  /// @brief 内存池头部
  struct header
  {
    uint32_t magic; /* 内存池头部魔数 */
  };

  /// @brief 内存池尾部
  struct footer
  {
    uint32_t magic; /* 内存池尾部魔数 */
  };

  /// @brief 块型内存池ThreadX对象
  using pool_t                   = TX_BLOCK_POOL;
  /// @brief 内存池是否为块型
  static constexpr bool is_block = true;

#if MEMORY_SAFETY_CHECKS
  /// @brief 内存池块大小
  static constexpr uint32_t block_size = sizeof(header) + sizeof(T) + sizeof(footer);
#else
  /// @brief 内存池块大小
  static constexpr uint32_t block_size = sizeof(T);
#endif

  /// @brief 内存池大小
  static constexpr uint32_t size         = N;
  /// @brief 内存池体积
  static constexpr uint32_t storage_size = N * block_size;
};
} /* namespace merory_internal */
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
class Memory_Pool final
{
  // 内存对齐检查
  static_assert(Align >= alignof(uint64_t), "Under-aligned type");
  // 禁止拷贝与移动
  NO_COPY_MOVE(Memory_Pool)

private:
  /// @brief 内存池默认名称
  static constexpr const char* default_name = "Memory_Pool";
  /// @brief 内存池基本数据
  using base_date                           = system_internal::merory_internal::Memory_Data<T, N, Block_Size>;
  /// @brief 内存池ThreadX对象类型
  using pool_struct                         = typename base_date::pool_t;
  /// @brief 内存池对象
  pool_struct m_pool_struct;

  /// @brief 内存池数据区
  ALIGN(Align) struct Storage
  {
    uint8_t data[base_date::storage_size];
  } m_storage;

private:
  /**
   * @brief  内存池 分配字节型内存
   *
   * @param  size  内存大小
   * @return void* 内存指针
   */
  void* O3 allocate_bytes(size_t size) noexcept
  {
    constexpr size_t add_size   = base_date::add_size;
    const size_t     total_size = size + add_size;

    void*      ptr              = nullptr;
    const UINT status           = tx_byte_allocate(&m_pool_struct, &ptr, total_size, TX_NO_WAIT);

    if (TX_SUCCESS != status)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      system::System_Monitor::log_error(system_internal::merory_internal::Memory_Error_Code::ALLOC_FAILED, "Byte Memory Pool Alloc Failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
      ptr = nullptr;
    }
    else
    {
      if constexpr (system_internal::merory_internal::enable_checks)
      {
        auto* head      = reinterpret_cast<typename base_date::header*>(ptr);
        head->magic     = system_internal::merory_internal::FRONT_MAGIC;
        head->user_size = size;

        auto* footer    = reinterpret_cast<typename base_date::footer*>(reinterpret_cast<uint8_t*>(ptr) + sizeof(typename base_date::header) + size);
        footer->magic   = system_internal::merory_internal::REAR_MAGIC;

        ptr             = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(ptr) + sizeof(typename base_date::header));
      }
    }

    return ptr;
  }

  /**
   * @brief  内存池 释放字节型内存
   *
   * @param  ptr   内存指针
   */
  void O3 deallocate_bytes(void* ptr) noexcept
  {
    if (nullptr != ptr)
    {
      uint8_t* p = reinterpret_cast<uint8_t*>(ptr);

      if constexpr (system_internal::merory_internal::enable_checks)
      {
        auto* head = reinterpret_cast<typename base_date::header*>(p - sizeof(typename base_date::header));
        if (system_internal::merory_internal::FRONT_MAGIC != head->magic)
        {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
          system::System_Monitor::log_error(system_internal::merory_internal::Memory_Error_Code::MAGIC_CORRUPTED, "Byte Memory Pool Dealloc Corrupted");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
          return;
        }

        auto* footer = reinterpret_cast<typename base_date::footer*>(p + head->user_size);
        if (system_internal::merory_internal::REAR_MAGIC != footer->magic)
        {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
          system::System_Monitor::log_error(system_internal::merory_internal::Memory_Error_Code::MAGIC_CORRUPTED, "Byte Memory Pool Dealloc Corrupted");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
          return;
        }

        p = p - sizeof(typename base_date::header);
      }

#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      if (TX_SUCCESS != tx_byte_release(p))
      {
        system::System_Monitor::log_error(system_internal::merory_internal::Memory_Error_Code::DEALLOC_FAILED, "Byte Memory Pool Dealloc Failed");
      }
#else
      tx_byte_release(p);
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
    }
  }

  /**
   * @brief  内存池 分配块型内存
   *
   * @return void* 内存指针
   */
  void* O3 allocate_block(void) noexcept
  {
    void*      ptr    = nullptr;
    const UINT status = tx_block_allocate(&m_pool_struct, &ptr, TX_NO_WAIT);

    if (TX_SUCCESS != status)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      system::System_Monitor::log_error(system_internal::merory_internal::Memory_Error_Code::ALLOC_FAILED, "Block Memory Pool Alloc Failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
      ptr = nullptr;
    }
    else
    {
      if constexpr (system_internal::merory_internal::enable_checks)
      {
        auto* head  = reinterpret_cast<typename base_date::header*>(ptr);
        head->magic = system_internal::merory_internal::FRONT_MAGIC;

        auto* foot  = reinterpret_cast<typename base_date::footer*>(reinterpret_cast<uint8_t*>(ptr) + base_date::block_size - sizeof(typename base_date::footer));
        foot->magic = system_internal::merory_internal::REAR_MAGIC;

        ptr         = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(ptr) + sizeof(typename base_date::header));
      }
    }

    return ptr;
  }

  /**
   * @brief 内存池 释放块型内存
   *
   * @param ptr   内存指针
   */
  void O3 deallocate_block(void* ptr) noexcept
  {
    if (nullptr != ptr)
    {
      uint8_t* p = reinterpret_cast<uint8_t*>(ptr);

      if constexpr (system_internal::merory_internal::enable_checks)
      {
        auto* head = reinterpret_cast<typename base_date::header*>(p - sizeof(typename base_date::header));
        if (system_internal::merory_internal::FRONT_MAGIC != head->magic)
        {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
          system::System_Monitor::log_error(system_internal::merory_internal::Memory_Error_Code::MAGIC_CORRUPTED, "Block Memory Pool Dealloc Corrupted");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
          return;
        }

        auto* foot = reinterpret_cast<typename base_date::footer*>(p - sizeof(typename base_date::header) + base_date::block_size - sizeof(typename base_date::footer));
        if (system_internal::merory_internal::REAR_MAGIC != foot->magic)
        {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
          system::System_Monitor::log_error(system_internal::merory_internal::Memory_Error_Code::MAGIC_CORRUPTED, "Block Memory Pool Dealloc Corrupted");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
          return;
        }

        p = p - sizeof(typename base_date::header);
      }

#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      if (TX_SUCCESS != tx_block_release(p))
      {
        system::System_Monitor::log_error(system_internal::merory_internal::Memory_Error_Code::DEALLOC_FAILED, "Block Memory Pool Dealloc Failed");
      }
#else
      tx_block_release(p);
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
    }
  }

  /**
   * @brief  内存池 分配结构型内存
   *
   * @return T*    内存指针
   */
  T* O3 allocate_struct(void) noexcept
  {
    void*      ptr    = nullptr;
    const UINT status = tx_block_allocate(&m_pool_struct, &ptr, TX_NO_WAIT);

    if (TX_SUCCESS != status)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      system::System_Monitor::log_error(system_internal::merory_internal::Memory_Error_Code::ALLOC_FAILED, "Block Memory Pool Alloc Failed");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
      ptr = nullptr;
    }
    else
    {
      if constexpr (system_internal::merory_internal::enable_checks)
      {
        auto* head  = reinterpret_cast<typename base_date::header*>(ptr);
        head->magic = system_internal::merory_internal::FRONT_MAGIC;

        auto* foot  = reinterpret_cast<typename base_date::footer*>(reinterpret_cast<uint8_t*>(ptr) + base_date::block_size - sizeof(typename base_date::footer));
        foot->magic = system_internal::merory_internal::REAR_MAGIC;

        ptr         = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(ptr) + sizeof(typename base_date::header));
      }
    }

    return (nullptr == ptr) ? nullptr : new (ptr) T;
  }

  /**
   * @brief  内存池 释放结构型内存
   *
   * @param  block 内存指针
   */
  void O3 deallocate_struct(T* block) noexcept
  {
    if (nullptr != block)
    {
      uint8_t* ptr = reinterpret_cast<uint8_t*>(block);

      if constexpr (system_internal::merory_internal::enable_checks)
      {
        auto* head = reinterpret_cast<typename base_date::header*>(ptr - sizeof(typename base_date::header));
        if (system_internal::merory_internal::FRONT_MAGIC != head->magic)
        {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
          system::System_Monitor::log_error(system_internal::merory_internal::Memory_Error_Code::MAGIC_CORRUPTED, "Block Memory Pool Dealloc Corrupted");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
          return;
        }

        auto* foot = reinterpret_cast<typename base_date::footer*>(ptr - sizeof(typename base_date::header) + base_date::block_size - sizeof(typename base_date::footer));
        if (system_internal::merory_internal::REAR_MAGIC != foot->magic)
        {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
          system::System_Monitor::log_error(system_internal::merory_internal::Memory_Error_Code::MAGIC_CORRUPTED, "Block Memory Pool Dealloc Corrupted");
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
          return;
        }

        ptr = ptr - sizeof(typename base_date::header);
      }

      block->~T();

#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      if (TX_SUCCESS != tx_block_release(ptr))
      {
        system::System_Monitor::log_error(system_internal::merory_internal::Memory_Error_Code::DEALLOC_FAILED, "Block Memory Pool Dealloc Failed");
      }
#else
      tx_block_release(ptr);
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
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
    if constexpr (!std::is_same_v<T, void>)
    {
      static_assert(Block_Size == sizeof(T), "Block size mismatch");
    }

    if constexpr (base_date::is_block)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      UINT status = tx_block_pool_create(&m_pool_struct, const_cast<CHAR*>(name), base_date::block_size, m_storage.data, base_date::storage_size);
      system::System_Monitor::check_status(status, "Memory Pool Creat Failed");
#else
      tx_block_pool_create(&m_pool_struct, const_cast<CHAR*>(name), base_date::block_size, m_storage.data, base_date::storage_size);
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
    }
    else
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      UINT status = tx_byte_pool_create(&m_pool_struct, const_cast<CHAR*>(name), m_storage.data, base_date::storage_size);
      system::System_Monitor::check_status(status, "Memory Pool Creat Failed");
#else
      tx_byte_pool_create(&m_pool_struct, const_cast<CHAR*>(name), m_storage.data, base_date::storage_size);
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
    }
  }

  /**
   * @brief  内存池 申请内存 - 字节型内存池
   *
   * @param  size  内存大小
   * @return void* 内存指针
   */
  template <bool Enable = !base_date::is_block>
  std::enable_if_t<Enable, void*> O3 malloc(size_t size) noexcept
  {
    return allocate_bytes(size);
  }

  /**
   * @brief  内存池 申请内存 - 块型内存池
   *
   * @return void* 内存指针
   */
  template <bool Enable = (base_date::is_block && std::is_same_v<T, void>)>
  std::enable_if_t<Enable, void*> O3 malloc() noexcept
  {
    return allocate_block();
  }

  /**
   * @brief  内存池 申请内存 - 结构型内存池
   *
   * @return T*    内存指针
   */
  template <typename U = T, bool Enable = (base_date::is_block && !std::is_same_v<T, void>)>
  std::enable_if_t<Enable, U*> O3 malloc() noexcept
  {
    static_assert(std::is_same_v<U, T>, "Block type mismatch");
    return allocate_struct();
  }

  /**
   * @brief  内存池 释放内存 - 字节型内存池
   *
   * @param  ptr   内存指针
   */
  template <bool Enable = !base_date::is_block>
  std::enable_if_t<Enable, void> O3 free(void* ptr) noexcept
  {
    deallocate_bytes(ptr);
  }

  /**
   * @brief  内存池 释放内存 - 块型内存池
   *
   * @param  ptr   内存指针
   */
  template <bool Enable = base_date::is_block>
  std::enable_if_t<Enable && std::is_same_v<T, void>, void> O3 free(void* ptr) noexcept
  {
    deallocate_block(ptr);
  }

  /**
   * @brief  内存池 释放内存 - 结构型内存池
   *
   * @param  ptr   内存指针
   */
  template <typename U = T, bool Enable = base_date::is_block>
  std::enable_if_t<Enable && !std::is_same_v<T, void>, void> O3 free(U* ptr) noexcept
  {
    static_assert(std::is_same_v<U, T>, "Block type mismatch");
    deallocate_struct(ptr);
  }

  /**
   * @brief  内存池 内存池大小
   *
   * @return uint32_t   内存池大小
   */
  const uint32_t get_total_size(void) const noexcept
  {
    return base_date::size;
  }

  /**
   * @brief  内存池 内存池剩余大小
   *
   * @return uint32_t   剩余大小
   */
  const uint32_t get_available(void) const noexcept
  {
    ULONG available = 0;

    if constexpr (base_date::is_block)
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      const UINT status = tx_block_pool_info_get(const_cast<pool_struct*>(&m_pool_struct), reinterpret_cast<CHAR**>(TX_NULL), &available, reinterpret_cast<ULONG*>(TX_NULL), reinterpret_cast<TX_THREAD**>(TX_NULL), reinterpret_cast<ULONG*>(TX_NULL), reinterpret_cast<TX_BLOCK_POOL**>(TX_NULL));

      if (TX_SUCCESS != status)
      {
        system::System_Monitor::check_status(status, "Memory Pool Info Get Failed");
        available = 0;
      }
#else
      tx_block_pool_info_get(const_cast<pool_struct*>(&m_pool_struct), reinterpret_cast<CHAR**>(TX_NULL), &available, reinterpret_cast<ULONG*>(TX_NULL), reinterpret_cast<TX_THREAD**>(TX_NULL), reinterpret_cast<ULONG*>(TX_NULL), reinterpret_cast<TX_BLOCK_POOL**>(TX_NULL));
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
    }
    else
    {
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
      const UINT status = tx_byte_pool_info_get(const_cast<pool_struct*>(&m_pool_struct), reinterpret_cast<CHAR**>(TX_NULL), &available, reinterpret_cast<ULONG*>(TX_NULL), reinterpret_cast<TX_THREAD**>(TX_NULL), reinterpret_cast<ULONG*>(TX_NULL), reinterpret_cast<TX_BYTE_POOL**>(TX_NULL));

      if (TX_SUCCESS != status)
      {
        system::System_Monitor::check_status(status, "Memory Pool Info Get Failed");
        available = 0;
      }
#else
      tx_byte_pool_info_get(const_cast<pool_struct*>(&m_pool_struct), reinterpret_cast<CHAR**>(TX_NULL), &available, reinterpret_cast<ULONG*>(TX_NULL), reinterpret_cast<TX_THREAD**>(TX_NULL), reinterpret_cast<ULONG*>(TX_NULL), reinterpret_cast<TX_BYTE_POOL**>(TX_NULL));
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
    }

    return available;
  }

  /**
   * @brief   内存池 内存池碎片数量
   *
   * @warning 块型内存池不支持此功能
   * @return  uint32_t   碎片数量
   */
  template <bool Enable = !base_date::is_block, typename = std::enable_if_t<Enable>>
  const uint32_t get_fragments() const noexcept
  {
    ULONG fragments = 0;
#if (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE)
    const UINT status = tx_byte_pool_info_get(const_cast<pool_struct*>(&m_pool_struct), reinterpret_cast<CHAR**>(TX_NULL), reinterpret_cast<ULONG*>(TX_NULL), &fragments, reinterpret_cast<TX_THREAD**>(TX_NULL), reinterpret_cast<ULONG*>(TX_NULL), reinterpret_cast<TX_BYTE_POOL**>(TX_NULL));

    if (TX_SUCCESS != status)
    {
      system::System_Monitor::log_error(status, "Memory Pool Info Get Failed");
      fragments = 0;
    }
#else
    tx_byte_pool_info_get(const_cast<pool_struct*>(&m_pool_struct), reinterpret_cast<CHAR**>(TX_NULL), reinterpret_cast<ULONG*>(TX_NULL), &fragments, reinterpret_cast<TX_THREAD**>(TX_NULL), reinterpret_cast<ULONG*>(TX_NULL), reinterpret_cast<TX_BYTE_POOL**>(TX_NULL));
#endif /* (SYSTEM_ERROR_LOG_ENABLE && MEMORY_POOL_ERROR_LOG_ENABLE) */
    return fragments;
  }

  /**
   * @brief  内存池 内存池已用大小
   *
   * @return uint32_t   已用大小
   */
  const uint32_t get_used(void) const noexcept
  {
    return get_total_size() - get_available();
  }

  /**
   * @brief  内存池 内存池空闲率(百分比)
   *
   * @return float      空闲率(百分比)
   */
  const float get_available_percent(void) const noexcept
  {
    return static_cast<float>(get_available()) / static_cast<float>(get_total_size()) * 100.0f;
  }

  /**
   * @brief   内存池 内存池碎片率(百分比)
   *
   * @warning 块型内存池不支持此功能
   * @return  float      碎片率(百分比)
   */
  template <bool Enable = !base_date::is_block, typename = std::enable_if_t<Enable>>
  const float get_fragments_percent(void) const noexcept
  {
    return static_cast<float>(get_fragments()) / static_cast<float>(get_total_size()) * 100.0f;
  }

  /**
   * @brief  内存池 内存池使用率(百分比)
   *
   * @return float      使用率(百分比)
   */
  const float get_used_percent(void) const noexcept
  {
    return static_cast<float>(get_used()) / static_cast<float>(get_total_size()) * 100.0f;
  }

  /**
   * @brief  内存池 析构函数
   *
   */
  ~Memory_Pool() noexcept
  {
    if constexpr (base_date::is_block)
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

template <uint32_t Size>
using Byte_Memory_Pool = Memory_Pool<Size, 1, void>;

template <uint32_t Size, uint32_t Block_Size>
using Block_Memory_Pool = Memory_Pool<Size, Block_Size, void>;

template <uint32_t Size, typename T>
using Struct_Memory_Pool = Memory_Pool<Size, sizeof(T), T>;

} /* namespace memory */
} /* namespace system */
} /* namespace QAQ */

#endif /* __MEMORY_POOL_HPP__ */
