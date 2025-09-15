#ifndef __STL_MEMORY_POOL_HPP__
#define __STL_MEMORY_POOL_HPP__

#include "memory_pool.hpp"
#include <memory>
#include <type_traits>

namespace QAQ
{
namespace system
{
namespace memory
{
/**
 * @brief STL兼容连续内存池分配器 - 适用于vector等需要连续内存的容器
 *
 * @tparam T 数据类型
 * @tparam Pool_Size 内存池大小（元素个数）
 */
template <typename T, size_t Pool_Size>
class STL_Continuous_Allocator
{
public:
  // 类型定义
  using value_type      = T;
  using pointer         = T*;
  using const_pointer   = const T*;
  using reference       = T&;
  using const_reference = const T&;
  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

  // 重新绑定模板
  template <typename U>
  struct rebind
  {
    using other = STL_Continuous_Allocator<U, Pool_Size>;
  };

private:
  // 使用结构型内存池
  using Pool_Type = Struct_Memory_Pool<Pool_Size, T>;
  static Pool_Type s_pool;

public:
  /**
   * @brief 构造函数
   */
  constexpr STL_Continuous_Allocator() noexcept = default;

  /**
   * @brief 拷贝构造函数
   */
  template <typename U>
  constexpr STL_Continuous_Allocator(const STL_Continuous_Allocator<U, Pool_Size>&) noexcept
  {
  }

  /**
   * @brief 分配内存
   *
   * @param n 需要分配的元素个数
   * @return pointer 指向分配内存的指针
   */
  pointer allocate(size_type n)
  {
    if (n > Pool_Size)
    {
      return nullptr;   // 请求超过池容量
    }

    // 计算实际需要的字节数
    size_t bytes_needed = n * sizeof(T);

    // 使用字节型分配以支持非固定大小请求
    void* ptr           = s_pool.template allocate(bytes_needed, TX_WAIT_FOREVER);
    if (!ptr)
    {
      return nullptr;
    }

    return static_cast<pointer>(ptr);
  }

  /**
   * @brief 释放内存
   *
   * @param p 指向要释放内存的指针
   * @param n 要释放的元素个数
   */
  void deallocate(pointer p, size_type n) noexcept
  {
    if (p)
    {
      s_pool.deallocate(p);
    }
  }

  /**
   * @brief 构造对象
   *
   * @tparam U 对象类型
   * @tparam Args 构造参数类型
   * @param p 指向构造位置的指针
   * @param args 构造参数
   */
  template <typename U, typename... Args>
  void construct(U* p, Args&&... args)
  {
    ::new ((void*)p) U(std::forward<Args>(args)...);
  }

  /**
   * @brief 析构对象
   *
   * @tparam U 对象类型
   * @param p 指向要析构对象的指针
   */
  template <typename U>
  void destroy(U* p)
  {
    p->~U();
  }

  /**
   * @brief 获取最大可分配大小
   *
   * @return size_type 最大可分配元素个数
   */
  size_type max_size() const noexcept
  {
    return Pool_Size;
  }

  /**
   * @brief 判断两个分配器是否相等
   *
   * @param other 另一个分配器
   * @return true 总是相等（无状态分配器）
   */
  template <typename U>
  bool operator==(const STL_Continuous_Allocator<U, Pool_Size>& other) const noexcept
  {
    return true;   // 无状态分配器总是相等
  }

  /**
   * @brief 判断两个分配器是否不等
   *
   * @param other 另一个分配器
   * @return false 总是相等（无状态分配器）
   */
  template <typename U>
  bool operator!=(const STL_Continuous_Allocator<U, Pool_Size>& other) const noexcept
  {
    return false;   // 无状态分配器总是相等
  }
};

// 静态成员定义
template <typename T, size_t Pool_Size>
typename STL_Continuous_Allocator<T, Pool_Size>::Pool_Type STL_Continuous_Allocator<T, Pool_Size>::s_pool;

/**
 * @brief STL兼容不连续内存池分配器 - 适用于list等节点式容器
 *
 * @tparam T 数据类型
 * @tparam Pool_Size 内存池大小（元素个数）
 */
template <typename T, size_t Pool_Size>
class STL_Discontinuous_Allocator
{
public:
  // 类型定义
  using value_type      = T;
  using pointer         = T*;
  using const_pointer   = const T*;
  using reference       = T&;
  using const_reference = const T&;
  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

  // 重新绑定模板
  template <typename U>
  struct rebind
  {
    using other = STL_Discontinuous_Allocator<U, Pool_Size>;
  };

private:
  // 使用结构型内存池
  using Pool_Type = Struct_Memory_Pool<Pool_Size, T>;
  static Pool_Type s_pool;

public:
  /**
   * @brief 构造函数
   */
  constexpr STL_Discontinuous_Allocator() noexcept = default;

  /**
   * @brief 拷贝构造函数
   */
  template <typename U>
  constexpr STL_Discontinuous_Allocator(const STL_Discontinuous_Allocator<U, Pool_Size>&) noexcept
  {
  }

  /**
   * @brief 分配内存（单个对象）
   *
   * @param n 需要分配的元素个数（通常为1）
   * @return pointer 指向分配内存的指针
   */
  [[nodiscard]] pointer allocate(size_type n)
  {
    if (n != 1)
    {
      // 不连续分配器通常只分配单个对象
      return nullptr;
    }

    // 分配单个T类型的对象
    T* ptr = s_pool.template allocate(TX_WAIT_FOREVER);
    if (!ptr)
    {
      return nullptr;
    }

    return ptr;
  }

  /**
   * @brief 释放内存
   *
   * @param p 指向要释放内存的指针
   * @param n 要释放的元素个数（通常为1）
   */
  void deallocate(pointer p, size_type n) noexcept
  {
    if (p && n == 1)
    {
      s_pool.deallocate(p);
    }
  }

  /**
   * @brief 构造对象
   *
   * @tparam U 对象类型
   * @tparam Args 构造参数类型
   * @param p 指向构造位置的指针
   * @param args 构造参数
   */
  template <typename U, typename... Args>
  void construct(U* p, Args&&... args)
  {
    ::new ((void*)p) U(std::forward<Args>(args)...);
  }

  /**
   * @brief 析构对象
   *
   * @tparam U 对象类型
   * @param p 指向要析构对象的指针
   */
  template <typename U>
  void destroy(U* p)
  {
    p->~U();
  }

  /**
   * @brief 获取最大可分配大小
   *
   * @return size_type 最大可分配元素个数
   */
  [[nodiscard]] size_type max_size() const noexcept
  {
    return Pool_Size;
  }

  /**
   * @brief 判断两个分配器是否相等
   *
   * @param other 另一个分配器
   * @return true 总是相等（无状态分配器）
   */
  template <typename U>
  bool operator==(const STL_Discontinuous_Allocator<U, Pool_Size>& other) const noexcept
  {
    return true;   // 无状态分配器总是相等
  }

  /**
   * @brief 判断两个分配器是否不等
   *
   * @param other 另一个分配器
   * @return false 总是相等（无状态分配器）
   */
  template <typename U>
  bool operator!=(const STL_Discontinuous_Allocator<U, Pool_Size>& other) const noexcept
  {
    return false;   // 无状态分配器总是相等
  }
};

// 静态成员定义
template <typename T, size_t Pool_Size>
typename STL_Discontinuous_Allocator<T, Pool_Size>::Pool_Type STL_Discontinuous_Allocator<T, Pool_Size>::s_pool;

/**
 * @brief 连续内存池分配器别名模板
 *
 * @tparam T 数据类型
 * @tparam Pool_Size 内存池大小（元素个数）
 */
template <typename T, size_t Pool_Size>
using continuous_allocator = STL_Continuous_Allocator<T, Pool_Size>;

/**
 * @brief 不连续内存池分配器别名模板
 *
 * @tparam T 数据类型
 * @tparam Pool_Size 内存池大小（元素个数）
 */
template <typename T, size_t Pool_Size>
using discontinuous_allocator = STL_Discontinuous_Allocator<T, Pool_Size>;

}   // namespace memory
}   // namespace system
}   // namespace QAQ

#endif   // __STL_MEMORY_POOL_HPP__