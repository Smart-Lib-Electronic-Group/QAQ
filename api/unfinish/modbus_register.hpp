#ifndef __MODBUS_REGISTER_HPP__
#define __MODBUS_REGISTER_HPP__

#include "system_include.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 协议
namespace protocol
{
/// @brief 名称空间 Modbus
namespace modbus
{
enum class Register_Endianness
{
  Big_Endian,          // ABCD: MSW first, BE inside (标准大端)
  Big_Endian_Swap,     // CDAB: LSW first, BE inside (大端字交换)
  Little_Endian,       // DCBA: LSW first, LE inside (标准小端)
  Little_Endian_Swap   // BADC: MSW first, LE inside (小端字交换)
};

template <uint16_t Start, uint16_t Length>
struct Register
{
  static constexpr uint16_t start  = Start;
  static constexpr uint16_t length = Length;
};
} /* namespace modbus */

/// @brief 命名空间 内部
namespace protocol_internal
{
/// @brief 名称空间 Modbus 内部
namespace modbus_internal
{
template <typename... Registers>
struct Storage;

template <typename First, typename... Rest>
struct Storage<First, Rest...>
{
  uint16_t         data[First::length] = { 0 };
  Storage<Rest...> rest;
};

template <>
struct Storage<>
{
};

constexpr bool is_little_endian_host()
{
  return __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
}
} /* namespace modbus_internal */
} /* namespace protocol_internal */

/// @brief 名称空间 Modbus
namespace modbus
{
template <typename... Registers>
class Register_Group
{
private:
  protocol_internal::modbus_internal::Storage<Registers...> storage;   // 静态分配的存储空间

  // 编译期段验证（零成本抽象）
  template <uint16_t Addr>
  static constexpr bool is_in_segment()
  {
    return ((Addr >= Registers::start && Addr < Registers::start + Registers::length) || ...);
  }

  // 运行时单寄存器查找（高效实现）
  template <typename First, typename... Rest>
  static uint16_t* find_register(uint16_t addr, Storage<First, Rest...>& s)
  {
    if (addr >= First::start && addr < First::start + First::length)
    {
      return &s.data[addr - First::start];
    }
    return find_register(addr, s.rest);
  }

  static uint16_t* find_register(uint16_t, Storage<>&)
  {
    return nullptr;
  }

  // 批量操作核心：编译期展开的段处理
  template <bool is_write, typename First, typename... Rest>
  static uint16_t process_segment(uint16_t start, uint16_t count, uint16_t* buffer, Storage<First, Rest...>& s)
  {
    constexpr auto bounds    = SegmentBounds<First> {};
    uint16_t       processed = 0;

    // 计算地址交集（编译期可优化常量）
    if (start < bounds.end && start + count > bounds.start)
    {
      const uint16_t overlap_start = (start > bounds.start) ? start : bounds.start;
      const uint16_t overlap_end   = (start + count < bounds.end) ? start + count : bounds.end;
      const uint16_t overlap_count = overlap_end - overlap_start;

      if (overlap_count > 0)
      {
        const uint16_t seg_offset = overlap_start - bounds.start;
        const uint16_t buf_offset = overlap_start - start;

        if constexpr (is_write)
        {
          // 写操作：硬件友好循环（避免memcpy调用开销）
          for (uint16_t i = 0; i < overlap_count; ++i)
          {
            s.data[seg_offset + i] = buffer[buf_offset + i];
          }
        }
        else
        {
          // 读操作：硬件友好循环
          for (uint16_t i = 0; i < overlap_count; ++i)
          {
            buffer[buf_offset + i] = s.data[seg_offset + i];
          }
        }
        processed = overlap_count;
      }
    }

    // 递归处理剩余段（编译期展开为线性代码）
    if constexpr (sizeof...(Rest) > 0)
    {
      processed += process_segment<is_write, Rest...>(start, count, buffer, s.rest);
    }
    return processed;
  }

  // 递归终止（编译期消除）
  template <bool is_write>
  static uint16_t process_segment(uint16_t, uint16_t, uint16_t*, Storage<>&)
  {
    return 0;
  }

  // 辅助函数：将任意类型转换为大端字节数组
  template <typename T>
  static void to_big_endian_bytes(const T& value, uint8_t* big_endian_bytes)
  {
    if constexpr (internal::modbus_internal::is_little_endian_host())
    {
      const uint8_t* src = reinterpret_cast<const uint8_t*>(&value);
      for (size_t i = 0; i < sizeof(T); ++i)
      {
        big_endian_bytes[i] = src[sizeof(T) - 1 - i];
      }
    }
    else
    {
      std::memcpy(big_endian_bytes, &value, sizeof(T));
    }
  }

  // 辅助函数：从大端字节数组转换为任意类型
  template <typename T>
  static void from_big_endian_bytes(const uint8_t* big_endian_bytes, T& value)
  {
    if constexpr (internal::modbus_internal::is_little_endian_host())
    {
      uint8_t* dst = reinterpret_cast<uint8_t*>(&value);
      for (size_t i = 0; i < sizeof(T); ++i)
      {
        dst[i] = big_endian_bytes[sizeof(T) - 1 - i];
      }
    }
    else
    {
      std::memcpy(&value, big_endian_bytes, sizeof(T));
    }
  }

  // 将T类型转换为寄存器序列（根据字节序）
  template <typename T, Register_Endianness E>
  static void convert_from_type(const T& value, uint16_t* reg_buffer)
  {
    static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
    static_assert(sizeof(T) % 2 == 0, "T size must be even for multi-register types");

    uint8_t big_endian_bytes[sizeof(T)];
    to_big_endian_bytes(value, big_endian_bytes);
    const size_t n = sizeof(T) / 2;   // 寄存器数量

    if constexpr (E == Register_Endianness::Big_Endian)
    {
      // ABCD: reg0 = [0,1], reg1 = [2,3], ...
      for (size_t i = 0; i < n; ++i)
      {
        reg_buffer[i] = (big_endian_bytes[2 * i] << 8) | big_endian_bytes[2 * i + 1];
      }
    }
    else if constexpr (E == Register_Endianness::Big_Endian_Swap)
    {
      // CDAB: reg0 = [n-1, n], reg1 = [n-3, n-2], ...
      for (size_t i = 0; i < n; ++i)
      {
        size_t idx    = (n - 1 - i) * 2;
        reg_buffer[i] = (big_endian_bytes[idx] << 8) | big_endian_bytes[idx + 1];
      }
    }
    else if constexpr (E == Register_Endianness::Little_Endian)
    {
      // DCBA: reg0 = [n-1, n] swapped, reg1 = [n-3, n-2] swapped, ...
      for (size_t i = 0; i < n; ++i)
      {
        size_t idx    = (n - 1 - i) * 2;
        reg_buffer[i] = (big_endian_bytes[idx + 1] << 8) | big_endian_bytes[idx];
      }
    }
    else if constexpr (E == Register_Endianness::Little_Endian_Swap)
    {
      // BADC: reg0 = [0,1] swapped, reg1 = [2,3] swapped, ...
      for (size_t i = 0; i < n; ++i)
      {
        reg_buffer[i] = (big_endian_bytes[2 * i + 1] << 8) | big_endian_bytes[2 * i];
      }
    }
  }

  // 将寄存器序列转换为T类型（根据字节序）
  template <typename T, Register_Endianness E>
  static T convert_to_type(const uint16_t* reg_buffer)
  {
    static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
    static_assert(sizeof(T) % 2 == 0, "T size must be even for multi-register types");

    uint8_t      big_endian_bytes[sizeof(T)] = { 0 };
    const size_t n                           = sizeof(T) / 2;

    if constexpr (E == Register_Endianness::Big_Endian)
    {
      for (size_t i = 0; i < n; ++i)
      {
        big_endian_bytes[2 * i]     = reg_buffer[i] >> 8;
        big_endian_bytes[2 * i + 1] = reg_buffer[i] & 0xFF;
      }
    }
    else if constexpr (E == Register_Endianness::Big_Endian_Swap)
    {
      for (size_t i = 0; i < n; ++i)
      {
        size_t idx                = (n - 1 - i) * 2;
        big_endian_bytes[idx]     = reg_buffer[i] >> 8;
        big_endian_bytes[idx + 1] = reg_buffer[i] & 0xFF;
      }
    }
    else if constexpr (E == Register_Endianness::Little_Endian)
    {
      for (size_t i = 0; i < n; ++i)
      {
        size_t idx                = (n - 1 - i) * 2;
        big_endian_bytes[idx]     = reg_buffer[i] & 0xFF;
        big_endian_bytes[idx + 1] = reg_buffer[i] >> 8;
      }
    }
    else if constexpr (E == Register_Endianness::Little_Endian_Swap)
    {
      for (size_t i = 0; i < n; ++i)
      {
        big_endian_bytes[2 * i]     = reg_buffer[i] & 0xFF;
        big_endian_bytes[2 * i + 1] = reg_buffer[i] >> 8;
      }
    }

    T value;
    from_big_endian_bytes(big_endian_bytes, value);
    return value;
  }

public:
  // 单寄存器读（保留原始接口）
  uint16_t read(uint16_t addr)
  {
    uint16_t* reg = find_register(addr, storage);
    return reg ? *reg : 0;
  }

  // 单寄存器写（返回操作结果）
  bool write(uint16_t addr, uint16_t value)
  {
    uint16_t* reg = find_register(addr, storage);
    if (reg)
    {
      *reg = value;
      return true;
    }
    return false;
  }

  // 优化的多读操作（工业级实现）
  uint16_t read(uint16_t start, uint16_t count, uint16_t* buffer)
  {
    // 初始化为0（非法地址处理）
    for (uint16_t i = 0; i < count; ++i)
    {
      buffer[i] = 0;
    }

    // 批量处理（返回实际读取数量）
    return process_segment<false, Registers...>(start, count, buffer, storage);
  }

  // 优化的多写操作（工业级实现）
  uint16_t write(uint16_t start, uint16_t count, const uint16_t* buffer)
  {
    // 直接处理有效段（返回实际写入数量）
    return process_segment<true, Registers...>(start, count, const_cast<uint16_t*>(buffer), storage);
  }

  // 编译期地址验证（开发时使用）
  template <uint16_t Addr>
  static constexpr bool is_valid_address()
  {
    return is_in_segment<Addr>();
  }

  // ============== 新增：多类型读写接口 ==============

  // 通用类型读取（支持float/double等）
  template <typename T, Register_Endianness E = Register_Endianness::Big_Endian>
  T read(uint16_t start)
  {
    static_assert(sizeof(T) <= 8, "T size must be <= 64 bits");
    constexpr size_t reg_count = sizeof(T) / 2;
    uint16_t         reg_buffer[reg_count];
    uint16_t         actual_read = read(start, reg_count, reg_buffer);

    if (actual_read != reg_count)
    {
      T zero = {};
      return zero;
    }
    return convert_to_type<T, E>(reg_buffer);
  }

  // 通用类型写入（支持float/double等）
  template <typename T, Register_Endianness E = Register_Endianness::Big_Endian>
  bool write(uint16_t start, const T& value)
  {
    static_assert(sizeof(T) <= 8, "T size must be <= 64 bits");
    constexpr size_t reg_count = sizeof(T) / 2;
    uint16_t         reg_buffer[reg_count];
    convert_from_type<T, E>(value, reg_buffer);
    return write(start, reg_count, reg_buffer) == reg_count;
  }

  // 字符串读取（支持字节序选择）
  template <Register_Endianness E = Register_Endianness::Big_Endian>
  uint16_t read(uint16_t start, uint16_t max_bytes, char* buffer)
  {
    const uint16_t reg_count = (max_bytes + 1) / 2;
    uint16_t       reg_buffer[reg_count];
    const uint16_t actual_regs  = read(start, reg_count, reg_buffer);
    uint16_t       actual_bytes = 0;

    for (uint16_t i = 0; i < actual_regs && actual_bytes < max_bytes; ++i)
    {
      if (E == Register_Endianness::Big_Endian || E == Register_Endianness::Big_Endian_Swap)
      {
        // 大端字节序：高字节在前
        buffer[actual_bytes++] = (reg_buffer[i] >> 8) & 0xFF;
        if (actual_bytes < max_bytes)
        {
          buffer[actual_bytes++] = reg_buffer[i] & 0xFF;
        }
      }
      else
      {
        // 小端字节序：低字节在前
        buffer[actual_bytes++] = reg_buffer[i] & 0xFF;
        if (actual_bytes < max_bytes)
        {
          buffer[actual_bytes++] = (reg_buffer[i] >> 8) & 0xFF;
        }
      }
    }
    return actual_bytes;
  }

  // 字符串写入（支持字节序选择）
  template <Register_Endianness E = Register_Endianness::Big_Endian>
  uint16_t write(uint16_t start, uint16_t byte_count, const char* buffer)
  {
    const uint16_t reg_count             = (byte_count + 1) / 2;
    uint16_t       reg_buffer[reg_count] = { 0 };
    uint16_t       bytes_written         = 0;

    for (uint16_t i = 0; i < reg_count && bytes_written < byte_count; ++i)
    {
      if (E == Register_Endianness::Big_Endian || E == Register_Endianness::Big_Endian_Swap)
      {
        // 大端：高字节在前
        reg_buffer[i] = (buffer[bytes_written++] << 8);
        if (bytes_written < byte_count)
        {
          reg_buffer[i] |= static_cast<uint8_t>(buffer[bytes_written++]);
        }
      }
      else
      {
        // 小端：低字节在前
        reg_buffer[i] = static_cast<uint8_t>(buffer[bytes_written++]);
        if (bytes_written < byte_count)
        {
          reg_buffer[i] |= (static_cast<uint8_t>(buffer[bytes_written++]) << 8);
        }
      }
    }

    const uint16_t actual_regs = write(start, reg_count, reg_buffer);
    return std::min(static_cast<uint16_t>(actual_regs * 2), byte_count);
  }

  // 8位整数数组读取（支持字节序选择）
  template <Register_Endianness E = Register_Endianness::Big_Endian>
  uint16_t read(uint16_t start, uint16_t count, uint8_t* buffer)
  {
    return read<E>(start, count, reinterpret_cast<char*>(buffer));
  }

  // 8位整数数组写入（支持字节序选择）
  template <Register_Endianness E = Register_Endianness::Big_Endian>
  uint16_t write(uint16_t start, uint16_t count, const uint8_t* buffer)
  {
    return write<E>(start, count, reinterpret_cast<const char*>(buffer));
  }
};
} /* namespace modbus */
} /* namespace protocol */
} /* namespace QAQ */

#endif /* __MODBUS_REGISTER_HPP__ */
