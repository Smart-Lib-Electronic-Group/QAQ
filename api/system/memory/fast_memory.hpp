#ifndef __FAST_MEMORY_HPP__
#define __FAST_MEMORY_HPP__

#include "system_include.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 系统
namespace system
{
/// @brief 名称空间 内存
namespace memory
{
/// @brief 快速内存拷贝函数 - 声明
extern inline void* fast_memcpy(void* dest, const void* src, size_t n) noexcept;
} /* namespace memory */
/// @brief 命名空间 内部
namespace system_internal
{
/// @brief 命名空间 内存 内部
namespace memory_internal
{
/// @brief DMA区域起始地址
constexpr uint32_t DMA_REGION_START = DMA_REGION_START_ADDRESSES;
/// @brief DMA区域结束地址
constexpr uint32_t DMA_REGION_END   = DMA_REGION_END_ADDRESSES;
/// @brief SIMD类型
using simd128_t                     = uint8_t __attribute__((vector_size(16)));

/**
 * @brief  判断指针是否对齐
 *
 * @tparam Align  对齐字节数
 * @param  ptr    指针
 * @return true   对齐
 * @return false  不对齐
 */
template <size_t Align>
constexpr bool is_aligned(const void* ptr) noexcept
{
  return (reinterpret_cast<uintptr_t>(ptr) % Align) == 0;
}

/**
 * @brief  判断指针是否在DMA区域
 *
 * @param  ptr    指针
 * @return true   在DMA区域
 * @return false  不在DMA区域
 */
constexpr bool is_dma_region(const void* ptr) noexcept
{
  const auto addr = reinterpret_cast<uintptr_t>(ptr);
  return (addr >= DMA_REGION_START) && (addr <= DMA_REGION_END);
}

/**
 * @brief  缓存维护操作
 *
 * @tparam Clean      是否需要清除缓存
 * @tparam Invalidate 是否需要失效缓存
 * @param  ptr        缓存起始地址
 * @param  size       缓存大小
 */
template <bool Clean, bool Invalidate>
QAQ_INLINE void QAQ_O3 cache_ops(const void* ptr, size_t size) noexcept
{
  if constexpr (Clean || Invalidate)
  {
    const uint32_t loc   = reinterpret_cast<uint32_t>(ptr) & ~0x1F;   // 32字节对齐
    const uint32_t end   = reinterpret_cast<uint32_t>(ptr) + size;
    const uint32_t lines = ((end - loc) + 31) / 32;

    if constexpr (Clean)
    {
      SCB_CleanDCache_by_Addr(reinterpret_cast<uint32_t*>(loc), lines * 32);
    }
    if constexpr (Invalidate)
    {
      SCB_InvalidateDCache_by_Addr(reinterpret_cast<uint32_t*>(loc), lines * 32);
    }
  }
}

/**
 * @brief  拷贝核心函数
 *
 * @tparam DMA_SRC  源地址是否在DMA区域
 * @tparam DMA_DEST 目标地址是否在DMA区域
 * @param  dest     目标地址
 * @param  src      源地址
 * @param  n        拷贝大小
 * @return void*    目标地址
 */
template <bool DMA_SRC, bool DMA_DEST>
QAQ_INLINE void* QAQ_O3 copy_core(void* dest, const void* src, size_t n) noexcept
{
  // 内存屏障保证操作顺序
  __DMB();

  // 缓存维护操作
  if constexpr (DMA_SRC)
    cache_ops<true, false>(src, n);
  if constexpr (DMA_DEST)
    cache_ops<false, true>(dest, n);

  // 主拷贝逻辑
  const size_t simd_size = sizeof(simd128_t);
  auto*        d         = static_cast<uint8_t*>(dest);
  auto*        s         = static_cast<const uint8_t*>(src);

  // 处理对齐部分
  if constexpr (DMA_SRC || DMA_DEST)
  {
    const size_t aligned_n = n & ~(simd_size - 1);
    for (size_t i = 0; i < aligned_n; i += simd_size * 4)
    {
      const auto v0                                        = *reinterpret_cast<const simd128_t*>(s + i);
      const auto v1                                        = *reinterpret_cast<const simd128_t*>(s + i + simd_size);
      const auto v2                                        = *reinterpret_cast<const simd128_t*>(s + i + simd_size * 2);
      const auto v3                                        = *reinterpret_cast<const simd128_t*>(s + i + simd_size * 3);

      *reinterpret_cast<simd128_t*>(d + i)                 = v0;
      *reinterpret_cast<simd128_t*>(d + i + simd_size)     = v1;
      *reinterpret_cast<simd128_t*>(d + i + simd_size * 2) = v2;
      *reinterpret_cast<simd128_t*>(d + i + simd_size * 3) = v3;
    }
    // 处理尾部
    if (n > aligned_n)
    {
      __builtin_memcpy(d + aligned_n, s + aligned_n, n - aligned_n);
    }
  }
  else
  {
    // 非DMA区域优化路径
    switch (n)
    {
      case 0 :
        break;
      case 1 :
        *d = *s;
        break;
      case 2 :
        *reinterpret_cast<uint16_t*>(d) = *reinterpret_cast<const uint16_t*>(s);
        break;
      case 4 :
        *reinterpret_cast<uint32_t*>(d) = *reinterpret_cast<const uint32_t*>(s);
        break;
      case 8 :
        *reinterpret_cast<uint64_t*>(d) = *reinterpret_cast<const uint64_t*>(s);
        break;
      case 16 :
        *reinterpret_cast<simd128_t*>(d) = *reinterpret_cast<const simd128_t*>(s);
        break;
      default :
        {
          const size_t blocks = n / simd_size;
          for (size_t i = 0; i < blocks; ++i)
          {
            reinterpret_cast<simd128_t*>(d)[i] = reinterpret_cast<const simd128_t*>(s)[i];
          }
          __builtin_memcpy(d + blocks * simd_size, s + blocks * simd_size, n % simd_size);
        }
    }
  }

  __DSB();   // 确保所有内存操作完成
  return dest;
}

/**
 * @brief  填充核心函数
 *
 * @tparam DMA_DEST 目标地址是否在DMA区域
 * @param  dest     目标地址
 * @param  ch       填充值
 * @param  n        填充大小
 * @return void*    目标地址
 */
template <bool DMA_DEST>
QAQ_INLINE void* QAQ_O3 set_core(void* dest, int ch, size_t n) noexcept
{
  __DMB();
  if constexpr (DMA_DEST)
    cache_ops<false, true>(dest, n);

  constexpr size_t simd128_size = sizeof(simd128_t);
  auto*            d            = static_cast<uint8_t*>(dest);
  const uint8_t    c            = static_cast<uint8_t>(ch);

  // 生成SIMD模式
  const simd128_t pattern       = simd128_t { c } - simd128_t { 0 };

  // 对齐处理
  const size_t prefix           = reinterpret_cast<uintptr_t>(d) % simd128_size;
  if (prefix)
  {
    const size_t adjust = simd128_size - prefix;
    __builtin_memset(d, c, std::min(adjust, n));
    d += adjust;
    n -= adjust;
  }

  // SIMD块填充
  auto* vd = reinterpret_cast<simd128_t*>(d);
  for (; n >= simd128_size * 4; n -= simd128_size * 4)
  {
    vd[0]  = pattern;
    vd[1]  = pattern;
    vd[2]  = pattern;
    vd[3]  = pattern;
    vd    += 4;
  }
  while (n >= simd128_size)
  {
    *vd++  = pattern;
    n     -= simd128_size;
  }

  // 尾部处理
  if (n > 0)
    __builtin_memset(vd, c, n);

  __DSB();
  return dest;
}

/**
 * @brief  移动核心函数
 *
 * @tparam DMA_SRC  源地址是否在DMA区域
 * @tparam DMA_DEST 目标地址是否在DMA区域
 * @param  dest     目标地址
 * @param  src      源地址
 * @param  n        移动大小
 * @return void*    目标地址
 */
template <bool DMA_SRC, bool DMA_DEST>
QAQ_INLINE void* QAQ_O3 move_core(void* dest, const void* src, size_t n) noexcept
{
  const auto d = static_cast<uint8_t*>(dest);
  const auto s = static_cast<const uint8_t*>(src);

  // 处理重叠区域
  if (d < s + n && s < d + n)
  {
    if (d > s)
    {   // 需要反向拷贝
      for (size_t i = n; i > 0; --i)
      {
        d[i - 1] = s[i - 1];
      }
    }
    else
    {   // 正向拷贝
      memory::fast_memcpy(d, s, n);
    }
  }
  else
  {
    memory::fast_memcpy(d, s, n);
  }
  return dest;
}

/**
 * @brief  比较核心函数
 *
 * @tparam DMA_SRC  源地址是否在DMA区域
 * @tparam DMA_DEST 目标地址是否在DMA区域
 * @param  s1       第一个地址
 * @param  s2       第二个地址
 * @param  n        比较大小
 * @return int      相等返回0，不等返回1或-1
 */
template <bool DMA_SRC, bool DMA_DEST>
QAQ_INLINE int QAQ_O3 compare_core(const void* s1, const void* s2, size_t n) noexcept
{
  __DMB();
  if constexpr (DMA_SRC)
    cache_ops<true, false>(s1, n);
  if constexpr (DMA_DEST)
    cache_ops<true, false>(s2, n);

  const auto* p1                = static_cast<const uint8_t*>(s1);
  const auto* p2                = static_cast<const uint8_t*>(s2);

  // SIMD并行比较
  constexpr size_t simd128_size = sizeof(simd128_t);
  for (; n >= simd128_size; n -= simd128_size)
  {
    const auto v1 = *reinterpret_cast<const simd128_t*>(p1);
    const auto v2 = *reinterpret_cast<const simd128_t*>(p2);
    if (__builtin_memcmp(&v1, &v2, simd128_size) != 0)
      break;
    p1 += simd128_size;
    p2 += simd128_size;
  }

  // 处理尾部
  for (size_t i = 0; i < n; ++i)
  {
    if (p1[i] != p2[i])
      return (p1[i] > p2[i]) ? 1 : -1;
  }
  return 0;
}
} /* namespace memory_internal */
} /* namespace system_internal */

/// @brief 名称空间 内存
namespace memory
{
/**
 * @brief  快速拷贝函数
 *
 * @param  dest   目标地址
 * @param  src    源
 * @param  n      拷贝大小
 * @return void*  目标地址
 */
/* QAQ_INLINE void* QAQ_O3 fast_memcpy(void* dest, const void* src, size_t n) noexcept
{
  return __builtin_memcpy(dest, src, n);
} */

QAQ_INLINE void* QAQ_O3 fast_memcpy(void* dest, const void* src, size_t n) noexcept
{
  if (n == 0 || dest == src)
    return dest;

  constexpr size_t min_dma_size = 64;
  const bool       src_dma      = system_internal::memory_internal::is_dma_region(src);
  const bool       dest_dma     = system_internal::memory_internal::is_dma_region(dest);

  // 小数据直接使用编译器内置实现
  if (n < min_dma_size && !(src_dma || dest_dma))
  {
    return __builtin_memcpy(dest, src, n);
  }

  // 选择优化路径
  if (src_dma && dest_dma)
  {
    return system_internal::memory_internal::copy_core<true, true>(dest, src, n);
  }
  else if (src_dma)
  {
    return system_internal::memory_internal::copy_core<true, false>(dest, src, n);
  }
  else if (dest_dma)
  {
    return system_internal::memory_internal::copy_core<false, true>(dest, src, n);
  }
  else
  {
    // 普通内存优化路径
    const bool aligned64 = system_internal::memory_internal::is_aligned<8>(dest) && system_internal::memory_internal::is_aligned<8>(src);
    if (aligned64)
    {
      return system_internal::memory_internal::copy_core<false, false>(dest, src, n);
    }
    else
    {
      return __builtin_memcpy(dest, src, n);
    }
  }
}

/* QAQ_INLINE void* fast_memcpy(void* dest, const void* src, size_t n)
{
  // 处理零长度拷贝
  if (n == 0)
  {
    return dest;
  }

  uintptr_t      d_addr = reinterpret_cast<uintptr_t>(dest);
  uintptr_t      s_addr = reinterpret_cast<uintptr_t>(src);
  uint8_t*       d8     = static_cast<uint8_t*>(dest);
  const uint8_t* s8     = static_cast<const uint8_t*>(src);

  // ========== 区域检测：AXI SRAM (D1域) ==========
  // AXI SRAM地址范围: 0x24000000 - 0x2407FFFF (512KB)
  const bool in_axi     = (d_addr >= 0x24000000 && d_addr < 0x24080000) && (s_addr >= 0x24000000 && s_addr < 0x24080000);

  if (in_axi)
  {
    // ========== AXI SRAM优化：64位宽访问 ==========
    // 处理小数据（<8字节）直接字节拷贝
    if (n < 8)
    {
      for (size_t i = 0; i < n; ++i)
      {
        d8[i] = s8[i];
      }
      return dest;
    }

    // 步骤1: 对齐目的地址到8字节边界
    size_t d_align = d_addr & 7;   // 当前偏移
    if (d_align != 0)
    {
      d_align                 = 8 - d_align;   // 需补齐的字节数
      const size_t copy_bytes = (d_align < n) ? d_align : n;
      for (size_t i = 0; i < copy_bytes; ++i)
      {
        *d8++ = *s8++;
      }
      n -= copy_bytes;
      // 检查剩余数据是否足够64位访问
      if (n < 8)
      {
        for (size_t i = 0; i < n; ++i)
        {
          *d8++ = *s8++;
        }
        return dest;
      }
    }

    // 步骤2: 对齐源地址到8字节边界（必须！避免LDRD总线错误）
    size_t s_align = reinterpret_cast<uintptr_t>(s8) & 7;
    if (s_align != 0)
    {
      s_align                 = 8 - s_align;
      const size_t copy_bytes = (s_align < n) ? s_align : n;
      for (size_t i = 0; i < copy_bytes; ++i)
      {
        *d8++ = *s8++;
      }
      n -= copy_bytes;
      if (n < 8)
      {
        for (size_t i = 0; i < n; ++i)
        {
          *d8++ = *s8++;
        }
        return dest;
      }
    }

    // 此时源/目的均8字节对齐，启用64位拷贝
    uint64_t*       d64    = reinterpret_cast<uint64_t*>(d8);
    const uint64_t* s64    = reinterpret_cast<const uint64_t*>(s8);
    size_t          words  = n / 8;   // 64位字数量
    n                     %= 8;       // 剩余字节数

    // 循环展开：每次迭代处理8个64位字（64字节）
    while (words >= 8)
    {
      d64[0]  = s64[0];
      d64[1]  = s64[1];
      d64[2]  = s64[2];
      d64[3]  = s64[3];
      d64[4]  = s64[4];
      d64[5]  = s64[5];
      d64[6]  = s64[6];
      d64[7]  = s64[7];
      d64    += 8;
      s64    += 8;
      words  -= 8;
    }

    // 处理剩余4-7个64位字
    while (words >= 4)
    {
      d64[0]  = s64[0];
      d64[1]  = s64[1];
      d64[2]  = s64[2];
      d64[3]  = s64[3];
      d64    += 4;
      s64    += 4;
      words  -= 4;
    }

    // 处理剩余1-3个64位字
    while (words > 0)
    {
      *d64++ = *s64++;
      words--;
    }

    // 更新指针到剩余字节
    d8 = reinterpret_cast<uint8_t*>(d64);
    s8 = reinterpret_cast<const uint8_t*>(s64);
  }
  // ========== 通用区域优化（TCM或其他RAM） ==========
  else
  {
    // 处理小数据（<4字节）直接字节拷贝
    if (n < 4)
    {
      for (size_t i = 0; i < n; ++i)
      {
        d8[i] = s8[i];
      }
      return dest;
    }

    // 步骤1: 对齐目的地址到4字节边界
    size_t d_align = d_addr & 3;
    if (d_align != 0)
    {
      d_align                 = 4 - d_align;
      const size_t copy_bytes = (d_align < n) ? d_align : n;
      for (size_t i = 0; i < copy_bytes; ++i)
      {
        *d8++ = *s8++;
      }
      n -= copy_bytes;
      if (n < 4)
      {
        for (size_t i = 0; i < n; ++i)
        {
          *d8++ = *s8++;
        }
        return dest;
      }
    }

    // 步骤2: 对齐源地址到4字节边界
    size_t s_align = reinterpret_cast<uintptr_t>(s8) & 3;
    if (s_align != 0)
    {
      s_align                 = 4 - s_align;
      const size_t copy_bytes = (s_align < n) ? s_align : n;
      for (size_t i = 0; i < copy_bytes; ++i)
      {
        *d8++ = *s8++;
      }
      n -= copy_bytes;
      if (n < 4)
      {
        for (size_t i = 0; i < n; ++i)
        {
          *d8++ = *s8++;
        }
        return dest;
      }
    }

    // 此时源/目的均4字节对齐，启用32位拷贝
    uint32_t*       d32    = reinterpret_cast<uint32_t*>(d8);
    const uint32_t* s32    = reinterpret_cast<const uint32_t*>(s8);
    size_t          words  = n / 4;   // 32位字数量
    n                     %= 4;       // 剩余字节数

    // 循环展开：每次迭代处理8个32位字（32字节）
    while (words >= 8)
    {
      d32[0]  = s32[0];
      d32[1]  = s32[1];
      d32[2]  = s32[2];
      d32[3]  = s32[3];
      d32[4]  = s32[4];
      d32[5]  = s32[5];
      d32[6]  = s32[6];
      d32[7]  = s32[7];
      d32    += 8;
      s32    += 8;
      words  -= 8;
    }

    // 处理剩余4-7个32位字
    while (words >= 4)
    {
      d32[0]  = s32[0];
      d32[1]  = s32[1];
      d32[2]  = s32[2];
      d32[3]  = s32[3];
      d32    += 4;
      s32    += 4;
      words  -= 4;
    }

    // 处理剩余1-3个32位字
    while (words > 0)
    {
      *d32++ = *s32++;
      words--;
    }

    // 更新指针到剩余字节
    d8 = reinterpret_cast<uint8_t*>(d32);
    s8 = reinterpret_cast<const uint8_t*>(s32);
  }

  // 拷贝剩余字节（0-7字节）
  for (size_t i = 0; i < n; ++i)
  {
    *d8++ = *s8++;
  }

  return dest;
}*/

/* QAQ_INLINE void* fast_memcpy(void* dst, const void* src, size_t n) noexcept
{
  uint8_t*       d = (uint8_t*)dst;
  const uint8_t* s = (const uint8_t*)src;

  // 1. 处理长度为0与源地址与目标地址相同的情况
  if (n == 0 || dst == src)
    return dst;

  // 2. 检查源/目标地址偏移是否同余（关键优化前提）
  if (((uint32_t)src ^ (uint32_t)dst) & 7)
  {
    // 偏移不同余时回退到标准memcpy（避免复杂非对齐处理）
    __builtin_memcpy(dst, src, n);
    return dst;
  }

  // 3. 处理非对齐头部（0~7字节）
  size_t head = (8 - ((size_t)src & 7)) & 7;
  if (head > n)
    head = n;
  while (head--)
    *d++ = *s++;

  n -= head;
  if (n == 0)
    return dst;

  // 4. 主循环：8字节对齐拷贝（核心优化）
  size_t n8  = n / 8;   // 双字数量
  n         %= 8;       // 剩余字节数

  // 使用内联汇编实现高效双字拷贝
  __asm__ volatile("1: \n"
                   "LDRD r0, r1, [%0], #8 \n"   // 从src加载8字节到r0,r1（自动递增地址）
                   "STRD r0, r1, [%1], #8 \n"   // 存储到dst（自动递增地址）
                   "SUBS %2, %2, #1 \n"         // 循环计数-1
                   "BNE 1b \n"                  // 循环直到完成
                   : "+r"(s), "+r"(d), "+r"(n8)
                   :
                   : "r0", "r1", "cc", "memory"   // 告知编译器寄存器/内存被修改
  );

  // 5. 处理尾部（0~7字节）
  while (n--)
    *d++ = *s++;

  return dst;
} */

/* QAQ_INLINE void* fast_memcpy(void* dst, const void* src, size_t len)
{
  if (len == 0)
    return dst;

  uint8_t*       d = static_cast<uint8_t*>(dst);
  const uint8_t* s = static_cast<const uint8_t*>(src);

  // 检查8字节对齐条件（源/目标偏移相同）
  if (reinterpret_cast<uintptr_t>(s) % 8 == reinterpret_cast<uintptr_t>(d) % 8)
  {
    // 处理头部（0-7字节）使地址8字节对齐
    size_t head = (8 - (reinterpret_cast<uintptr_t>(s) & 7)) & 7;
    if (head > 0 && head <= len)
    {
      for (size_t i = 0; i < head; ++i)
      {
        *d++ = *s++;
      }
      len -= head;
    }

    // 主循环：64位宽拷贝 + 4路循环展开 (32字节/次)
    size_t blocks  = len / 8;
    len           %= 8;
    while (blocks >= 4)
    {
      // 一次性加载4个64位数据
      uint64_t a                            = *reinterpret_cast<const uint64_t*>(s);
      uint64_t b                            = *reinterpret_cast<const uint64_t*>(s + 8);
      uint64_t c                            = *reinterpret_cast<const uint64_t*>(s + 16);
      uint64_t d_val                        = *reinterpret_cast<const uint64_t*>(s + 24);

      // 一次性存储4个64位数据
      *reinterpret_cast<uint64_t*>(d)       = a;
      *reinterpret_cast<uint64_t*>(d + 8)   = b;
      *reinterpret_cast<uint64_t*>(d + 16)  = c;
      *reinterpret_cast<uint64_t*>(d + 24)  = d_val;

      s                                    += 32;
      d                                    += 32;
      blocks                               -= 4;
    }

    // 处理剩余64位块
    while (blocks > 0)
    {
      *reinterpret_cast<uint64_t*>(d)  = *reinterpret_cast<const uint64_t*>(s);
      s                               += 8;
      d                               += 8;
      --blocks;
    }
  }

  // 尾部处理（0-7字节）或未对齐时的退化处理
  while (len >= 4)
  {
    *reinterpret_cast<uint32_t*>(d)  = *reinterpret_cast<const uint32_t*>(s);
    s                               += 4;
    d                               += 4;
    len                             -= 4;
  }
  while (len > 0)
  {
    *d++ = *s++;
    --len;
  }

  return dst;
} */

/* QAQ_INLINE void* fast_memcpy(void* dest, const void* src, size_t n)
{
  // 保存原始指针用于返回
  void* ret = dest;

  // 使用内联汇编实现高性能拷贝
  __asm__ volatile(
    // 检查长度是否为0
    "    cbz     %2, .Ldone          \n"   // if (n == 0) return dest

    // 设置寄存器
    "    mov     r4, %1              \n"   // r4 = src (源地址)
    "    mov     r5, %0              \n"   // r5 = dest (目标地址)
    "    mov     r6, %2              \n"   // r6 = n (剩余字节数)

    // ===== 步骤1: 处理目标地址未对齐到8字节的情况 =====
    "    mov     r7, r5              \n"   // r7 = 当前目标地址
    "    and     r7, r7, #7          \n"   // r7 = dest & 0x7 (计算偏移量)
    "    cbz     r7, .Laligned8      \n"   // 如果已8字节对齐，跳过

    // 计算需要对齐的字节数 (8 - offset)
    "    sub     r7, #8              \n"   // r7 = -8
    "    neg     r7, r7              \n"   // r7 = 8
    "    sub     r7, r7, r7, lsr #31 \n"   // 确保正值 (ARMv7-M技巧)
    "    cmp     r6, r7              \n"   // 检查剩余字节是否足够
    "    itt     lt                  \n"
    "    blt     .Lcopy_bytes        \n"   // 不够则直接字节拷贝
    "    sub     r6, r6, r7          \n"   // n -= 对齐所需字节数

    // 逐字节拷贝直到8字节对齐
    ".Lalign_loop:                   \n"
    "    ldrb    r8, [r4], #1        \n"   // 读取1字节
    "    strb    r8, [r5], #1        \n"   // 写入1字节
    "    subs    r7, r7, #1          \n"   // 递减对齐计数
    "    bne     .Lalign_loop        \n"

    // ===== 步骤2: 8字节对齐后的主拷贝循环 =====
    ".Laligned8:                     \n"
    "    cmp     r6, #64             \n"   // 检查是否够1个cache行
    "    blt     .Lcopy_dwords       \n"   // 不够则用双字拷贝

    // 计算64字节块的数量 (n / 64)
    "    lsrs    r7, r6, #6          \n"   // r7 = n >> 6 (块数量)
    "    beq     .Lcopy_dwords       \n"   // 无完整块则跳过

    // ===== 主循环: 每次拷贝64字节 (8个双字) =====
    ".Lblock_loop:                   \n"
    // 读取8个双字 (64字节)
    "    ldrd    r8, r9, [r4], #8    \n"   // 读取第1-2双字
    "    ldrd    r10, r11, [r4], #8  \n"   // 读取第3-4双字
    "    ldrd    r12, lr, [r4], #8   \n"   // 读取第5-6双字
    "    ldrd    r0, r1, [r4], #8    \n"   // 读取第7-8双字

    // 写入8个双字 (64字节)
    "    strd    r8, r9, [r5], #8    \n"
    "    strd    r10, r11, [r5], #8  \n"
    "    strd    r12, lr, [r5], #8   \n"
    "    strd    r0, r1, [r5], #8    \n"

    "    subs    r7, r7, #1          \n"   // 块计数-1
    "    bne     .Lblock_loop        \n"   // 循环

    // 计算剩余字节数 (n % 64)
    "    and     r6, r6, #63         \n"
    "    cbz     r6, .Ldone          \n"   // 无剩余字节则完成

    // ===== 步骤3: 拷贝剩余双字 (8字节块) =====
    ".Lcopy_dwords:                  \n"
    "    cmp     r6, #8              \n"
    "    blt     .Lcopy_bytes        \n"   // 不足8字节则跳转

    // 拷贝所有完整双字
    "    lsrs    r7, r6, #3          \n"   // r7 = n / 8
    ".Ldword_loop:                   \n"
    "    ldrd    r8, r9, [r4], #8    \n"
    "    strd    r8, r9, [r5], #8    \n"
    "    subs    r7, r7, #1          \n"
    "    bne     .Ldword_loop        \n"

    // 计算剩余字节数 (n % 8)
    "    and     r6, r6, #7          \n"
    "    cbz     r6, .Ldone          \n"   // 无剩余字节则完成

    // ===== 步骤4: 拷贝尾部字节 =====
    ".Lcopy_bytes:                   \n"
    "    ldrb    r8, [r4], #1        \n"
    "    strb    r8, [r5], #1        \n"
    "    subs    r6, r6, #1          \n"
    "    bne     .Lcopy_bytes        \n"

    ".Ldone:                         \n"
    : "+r"(dest), "+r"(src), "+r"(n)                                                              // 输出操作数
    :                                                                                             // 无输入操作数
    : "r0", "r1", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "lr", "cc", "memory"   // 被破坏的寄存器
  );

  return ret;
} */

/**
 * @brief  快速填充函数
 *
 * @param  dest   目标地址
 * @param  ch     填充值
 * @param  n      填充大小
 * @return void*  目标地址
 */
QAQ_INLINE void* QAQ_O3 fast_memset(void* dest, int ch, size_t n) noexcept
{
  if (n == 0)
    return dest;

  const bool dest_dma = system_internal::memory_internal::is_dma_region(dest);
  if (dest_dma || n >= 64)
  {
    if (dest_dma)
      return system_internal::memory_internal::set_core<true>(dest, ch, n);
    else
      return system_internal::memory_internal::set_core<false>(dest, ch, n);
  }
  else
  {
    // 小数据优化
    uint64_t pattern64 = 0x0101010101010101ULL * static_cast<uint8_t>(ch);
    switch (n)
    {
      case 1 :
        *static_cast<uint8_t*>(dest) = ch;
        break;
      case 2 :
        *static_cast<uint16_t*>(dest) = pattern64;
        break;
      case 4 :
        *static_cast<uint32_t*>(dest) = pattern64;
        break;
      case 8 :
        *static_cast<uint64_t*>(dest) = pattern64;
        break;
      default :
        __builtin_memset(dest, ch, n);
    }
    return dest;
  }
}

/**
 * @brief  快速移动函数
 *
 * @param  dest   目标地址
 * @param  src    源地址
 * @param  n      移动大小
 * @return void*  目标地址
 */
QAQ_INLINE void* QAQ_O3 fast_memmove(void* dest, const void* src, size_t n) noexcept
{
  if (n == 0 || dest == src)
    return dest;

  const bool src_dma  = system_internal::memory_internal::is_dma_region(src);
  const bool dest_dma = system_internal::memory_internal::is_dma_region(dest);

  if (src_dma && dest_dma)
  {
    return system_internal::memory_internal::move_core<true, true>(dest, src, n);
  }
  else if (src_dma)
  {
    return system_internal::memory_internal::move_core<true, false>(dest, src, n);
  }
  else if (dest_dma)
  {
    return system_internal::memory_internal::move_core<false, true>(dest, src, n);
  }
  else
  {
    // 非DMA区域使用优化拷贝
    const bool aligned64 = system_internal::memory_internal::is_aligned<8>(dest) && system_internal::memory_internal::is_aligned<8>(src);
    if (aligned64)
    {
      return fast_memcpy(dest, src, n);
    }
    else
    {
      return __builtin_memmove(dest, src, n);
    }
  }
}

/**
 * @brief  快速比较函数
 *
 * @param  s1  第一个地址
 * @param  s2  第二个地址
 * @param  n   比较大小
 * @return int 相等返回0，不等返回1或-1
 */
QAQ_INLINE int QAQ_O3 fast_memcmp(const void* s1, const void* s2, size_t n) noexcept
{
  if (n == 0)
    return 0;

  const bool s1_dma = system_internal::memory_internal::is_dma_region(s1);
  const bool s2_dma = system_internal::memory_internal::is_dma_region(s2);

  if (s1_dma && s2_dma)
  {
    return system_internal::memory_internal::compare_core<true, true>(s1, s2, n);
  }
  else if (s1_dma)
  {
    return system_internal::memory_internal::compare_core<true, false>(s1, s2, n);
  }
  else if (s2_dma)
  {
    return system_internal::memory_internal::compare_core<false, true>(s1, s2, n);
  }
  else
  {
    // 非DMA优化路径
    const bool aligned64 = system_internal::memory_internal::is_aligned<8>(s1) && system_internal::memory_internal::is_aligned<8>(s2);
    if (aligned64)
    {
      const uint64_t* p1 = static_cast<const uint64_t*>(s1);
      const uint64_t* p2 = static_cast<const uint64_t*>(s2);
      for (; n >= 8; n -= 8)
      {
        if (*p1 != *p2)
          break;
        ++p1;
        ++p2;
      }
    }
    return __builtin_memcmp(s1, s2, n);
  }
}
} /* namespace memory */
} /* namespace system */
} /* namespace QAQ */

#endif /* __FAST_MEMORY_HPP__ */
