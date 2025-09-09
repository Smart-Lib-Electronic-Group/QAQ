#ifndef __RING_BUFFER_HPP__
#define __RING_BUFFER_HPP__

#include "fast_memory.hpp"
#include "mutex.hpp"
#include "device_base.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 系统
namespace system
{
/// @brief 名称空间 内存
namespace memory
{
/// @brief 环形缓冲区 工作模式
enum class Ring_Buffer_Mode
{
  NORMAL,              /* 正常模式 */
  INPUT_BYTES,         /* 字节输入模式 */
  INPUT_SINGLE_BUFFER, /* 单缓冲区输入模式 */
  INPUT_DOUBLE_BUFFER, /* 双缓冲区输入模式 */
  OUTPUT,              /* 输出模式 */
};
} /* namespace memory */

/// @brief 名称空间 设备
namespace device
{
template <Stream_Type Type, uint32_t In_Buf_Size, uint32_t Out_Buf_Size, memory::Ring_Buffer_Mode In_Buf_Mode>
class Stream_Device;
} /* namespace device */

/// @brief 名称空间 内部
namespace system_internal
{
/// @brief 名称空间 环形缓冲区 内部
namespace ring_buffer_internal
{
/**
 * @brief  环形缓冲区模板基类
 *
 * @tparam T    元素类型
 * @tparam N    缓冲区大小，必须是2的幂次方，且小于等于4096
 */
template <typename T, uint32_t N>
class Ring_Buffer_Base
{
  // 检查缓冲区大小大于等于2
  static_assert(N >= 2, "Ring buffer size must be greater than or equal to 2");
  // 检查缓冲区大小是2的幂次方
  static_assert((N & (N - 1)) == 0, "Ring buffer size must be a power of 2");
  // 检查缓冲区大小小于等于4096
  static_assert(N <= 16384, "Ring buffer size must be less than or equal to 16384");
  // 检查缓冲区元素类型必须是平凡类型
  static_assert(std::is_trivially_copyable_v<T>, "Ring buffer element type must be trivially copyable");

  // 禁止拷贝和移动
  NO_COPY_MOVE(Ring_Buffer_Base)

protected:
  /// @brief 环形缓冲区 头指针
  volatile uint32_t m_head ALIGN(32)             = 0;
  /// @brief 环形缓冲区 尾指针
  volatile uint32_t m_tail ALIGN(32)             = 0;
  /// @brief 环形缓冲区 数据
  T                        m_buffer[N] ALIGN(32) = { 0 };
  /// @brief 环形缓冲区 回滚指针
  uint32_t                 m_roll_back_save      = 0;

public:
  /// @brief 环形缓冲区 状态
  enum class Status
  {
    SUCCESS = 0, /* 成功 */
    FULL,        /* 已满 */
    EMPTY,       /* 已空 */
    ROLL_OVER,   /* 回滚溢出 */
    ERROR,       /* 其他错误 */
  };

private:
  /**
   * @brief  环形缓冲区 写入数据
   *
   * @param  data     要写入的数据(引用)
   * @return true     写入成功
   * @return false    写入失败
   */
  bool O3 try_push(const T& data) noexcept
  {
    const uint32_t current_tail = m_tail;
    const uint32_t next_tail    = (current_tail + 1) & (N - 1);

    if (next_tail == (m_head & (N - 1)))
    {
      return false;
    }

    memory::fast_memcpy(&m_buffer[current_tail], &data, sizeof(T));
    m_tail = next_tail;
    return true;
  }

  /**
   * @brief  环形缓冲区 读取数据
   *
   * @param  data     要读取的数据(引用)
   * @return true     读取成功
   * @return false    读取失败
   */
  bool O3 try_pop(T& data) noexcept
  {
    const uint32_t current_head = m_head;
    if (current_head == m_tail)
    {
      return false;
    }

    m_roll_back_save = current_head;
    memory::fast_memcpy(&data, &m_buffer[current_head], sizeof(T));
    m_head = (current_head + 1) & (N - 1);
    return true;
  }

  /**
   * @brief  环形缓冲区 批量读取数据
   *
   * @param  data     要读取的数据(指针)
   * @param  request  要读取的数据数量
   * @return uint32_t 实际读取的数据数量
   */
  uint32_t O3 try_read(T* data, uint32_t request) noexcept
  {
    const uint32_t used            = available();
    const uint32_t copy_size       = (request > used) ? used : request;
    const uint32_t current_head    = m_head;
    const uint32_t first_copy      = N - (current_head & (N - 1));
    const uint32_t first_copy_size = (first_copy < copy_size) ? first_copy : copy_size;

    memory::fast_memcpy(data, &m_buffer[current_head], first_copy_size * sizeof(T));
    if (first_copy_size < copy_size)
    {
      memory::fast_memcpy(data + first_copy_size, m_buffer, (copy_size - first_copy_size) * sizeof(T));
    }

    m_roll_back_save = (current_head + copy_size) & (N - 1);
    m_head           = (current_head + copy_size) & (N - 1);
    return copy_size;
  }

  /**
   * @brief  环形缓冲区 批量写入数据
   *
   * @param  data     要写入的数据(指针)
   * @param  request  要写入的数据数量
   * @return uint32_t 实际写入的数据数量
   */
  uint32_t O3 try_write(const T* data, uint32_t request) noexcept
  {
    const uint32_t space           = this->space();
    const uint32_t copy_size       = (request > space) ? space : request;
    const uint32_t current_tail    = m_tail;
    const uint32_t first_copy      = N - (current_tail & (N - 1));
    const uint32_t first_copy_size = (first_copy < copy_size) ? first_copy : copy_size;

    memory::fast_memcpy(&m_buffer[current_tail], data, first_copy_size * sizeof(T));
    if (first_copy_size < copy_size)
    {
      memory::fast_memcpy(m_buffer, data + first_copy_size, (copy_size - first_copy_size) * sizeof(T));
    }

    m_tail = (current_tail + copy_size) & (N - 1);
    return copy_size;
  }

  /**
   * @brief  环形缓冲区 回滚
   *
   * @return true     回滚成功
   * @return false    回滚失败
   */
  bool O3 try_roll_back(void) noexcept
  {
    if (m_head == m_roll_back_save)
    {
      return false;
    }

    m_head = m_roll_back_save;
    return true;
  }

  /**
   * @brief  环形缓冲区 探视数据
   *
   * @param  data      要探视的数据(指针)
   * @param  request   要探视的数据数量
   * @return uint32_t  实际探视的数据数量
   */
  uint32_t O3 try_peek(T* data, uint32_t request) const noexcept
  {
    const uint32_t used = available();

    if (request == 0 || data == nullptr || used == 0)
    {
      return 0;
    }

    const uint32_t copy_size       = (request > used) ? used : request;
    const uint32_t current_head    = m_head;
    const uint32_t first_copy      = N - (current_head & (N - 1));
    const uint32_t first_copy_size = (first_copy < copy_size) ? first_copy : copy_size;

    memory::fast_memcpy(data, &m_buffer[current_head], first_copy_size * sizeof(T));
    if (first_copy_size < copy_size)
    {
      memory::fast_memcpy(data + first_copy_size, m_buffer, (copy_size - first_copy_size) * sizeof(T));
    }

    return copy_size;
  }

protected:
  /**
   * @brief 环形缓冲区 构造函数
   *
   */
  explicit Ring_Buffer_Base(const char* name = "Ring Buffer") {}

  /**
   * @brief 环形缓冲区 写入数据
   *
   * @param  data    要写入的数据(引用)
   * @return Status  写入结果
   */
  Status push(const T& data) noexcept
  {
    system::kernel::Interrupt_Guard lock;
    const bool                      success = try_push(data);
    return success ? Status::SUCCESS : Status::FULL;
  }

  /**
   * @brief  环形缓冲区 读取数据
   *
   * @param  data     要读取的数据(引用)
   * @return Status   读取结果
   */
  Status pop(T& data) noexcept
  {
    system::kernel::Interrupt_Guard lock;
    const bool                      success = try_pop(data);
    return success ? Status::SUCCESS : Status::EMPTY;
  }

  /**
   * @brief  环形缓冲区 批量读取数据
   *
   * @param  data     要读取的数据(指针)
   * @param  request  要读取的数据数量
   * @return uint32_t 实际读取的数据数量
   */
  uint32_t read(T* data, uint32_t request) noexcept
  {
    system::kernel::Interrupt_Guard lock;
    const uint32_t                  result = try_read(data, request);
    return result;
  }

  /**
   * @brief  环形缓冲区 批量写入数据
   *
   * @param  data     要写入的数据(指针)
   * @param  request  要写入的数据数量
   * @return uint32_t 实际写入的数据数量
   */
  uint32_t write(const T* data, uint32_t request) noexcept
  {
    system::kernel::Interrupt_Guard lock;
    const uint32_t                  result = try_write(data, request);
    return result;
  }

  /**
   * @brief  环形缓冲区 回滚
   *
   * @return Status   回滚结果
   */
  Status roll_back(void) noexcept
  {
    system::kernel::Interrupt_Guard lock;
    const bool                      success = try_roll_back();
    return success ? Status::SUCCESS : Status::ROLL_OVER;
  }

  /**
   * @brief  环形缓冲区 探视数据
   *
   * @param  data     要探视的数据(指针)
   * @param  request  要探视的数据数量
   * @return uint32_t 实际探视的数据数量
   */
  uint32_t peek(T* data, uint32_t request) noexcept
  {
    system::kernel::Interrupt_Guard lock;
    const uint32_t                  result = try_peek(data, request);
    return result;
  }

  /**
   * @brief  环形缓冲区 析构函数
   *
   */
  virtual ~Ring_Buffer_Base() {}

public:
  uint32_t clear(void) noexcept
  {
    system::kernel::Interrupt_Guard lock;
    uint32_t                        ret = available();
    m_head                              = 0;
    m_tail                              = 0;
    return ret;
  }

  /**
   * @brief  环形缓冲区 可用数据数量
   *
   * @return uint32_t 可用数据数量
   */
  uint32_t available(void) const noexcept
  {
    return ((m_tail - m_head) & (N - 1));
  }

  /**
   * @brief  环形缓冲区 空闲空间大小
   *
   * @return uint32_t 空闲空间大小
   */
  uint32_t space(void) const noexcept
  {
    return ((N - 1) - ((m_tail - m_head) & (N - 1)));
  }

  /**
   * @brief  环形缓冲区 容量
   *
   * @return uint32_t 容量
   */
  uint32_t capacity(void) const noexcept
  {
    return N;
  }

  /**
   * @brief  环形缓冲区 是否为空
   *
   * @return true   为空
   * @return false  不为空
   */
  bool empty(void) const noexcept
  {
    return m_head == m_tail;
  }

  /**
   * @brief  环形缓冲区 是否已满
   *
   * @return true   已满
   * @return false  不满
   */
  bool full(void) const noexcept
  {
    return ((m_tail + 1) & (N - 1)) == m_head;
  }
};
} /* namespace ring_buffer_internal */
} /* namespace system_internal */

/// @brief 名称空间 内存
namespace memory
{
/**
 * @brief  环形缓冲区模板类
 *
 * @tparam T    元素类型
 * @tparam N    缓冲区大小，必须是2的幂次方，且小于等于4096
 * @tparam Mode 环形缓冲区模式
 */
template <typename T, uint32_t N, Ring_Buffer_Mode Mode = Ring_Buffer_Mode::NORMAL>
class Ring_Buffer;

/**
 * @brief  环形缓冲区模板类 - 普通模式特化
 *
 * @tparam T    元素类型
 * @tparam N    缓冲区大小，必须是2的幂次方，且小于等于4096
 */
template <typename T, uint32_t N>
class Ring_Buffer<T, N, Ring_Buffer_Mode::NORMAL> : public system_internal::ring_buffer_internal::Ring_Buffer_Base<T, N>
{
  // 禁止拷贝和移动
  NO_COPY_MOVE(Ring_Buffer)

private:
  // 基类类型
  using Base   = system_internal::ring_buffer_internal::Ring_Buffer_Base<T, N>;
  // 返回值类型
  using Status = typename Base::Status;

public:
  /**
   * @brief  环形缓冲区 构造函数
   *
   */
  explicit Ring_Buffer() : Base() {}

  /**
   * @brief 环形缓冲区 写入数据
   *
   * @param  data    要写入的数据(引用)
   * @return Status  写入结果
   */
  Status push(const T& data) noexcept
  {
    return Base::push(data);
  }

  /**
   * @brief  环形缓冲区 读取数据
   *
   * @param  data     要读取的数据(引用)
   * @return Status   读取结果
   */
  Status pop(T& data) noexcept
  {
    return Base::pop(data);
  }

  /**
   * @brief  环形缓冲区 批量读取数据
   *
   * @param  data     要读取的数据(指针)
   * @param  request  要读取的数据数量
   * @return uint32_t 实际读取的数据数量
   */
  uint32_t read(T* data, uint32_t request) noexcept
  {
    return Base::read(data, request);
  }

  /**
   * @brief  环形缓冲区 批量写入数据
   *
   * @param  data     要写入的数据(指针)
   * @param  request  要写入的数据数量
   * @return uint32_t 实际写入的数据数量
   */
  uint32_t write(const T* data, uint32_t request) noexcept
  {
    return Base::write(data, request);
  }

  /**
   * @brief  环形缓冲区 回滚
   *
   * @return Status   回滚结果
   */
  Status roll_back(void) noexcept
  {
    return Base::roll_back();
  }

  /**
   * @brief  环形缓冲区 探视数据
   *
   * @param  data     要探视的数据(指针)
   * @param  request  要探视的数据数量
   * @return uint32_t 实际探视的数据数量
   */
  uint32_t peek(T* data, uint32_t request) noexcept
  {
    return Base::peek(data, request);
  }

  /**
   * @brief  环形缓冲区 析构函数
   *
   */
  virtual ~Ring_Buffer() {}
};

/**
 * @brief  环形缓冲区模板类 - 字节输入模式特化
 *
 * @tparam T    元素类型
 * @tparam N    缓冲区大小，必须是2的幂次方，且小于等于4096
 */
template <typename T, uint32_t N>
class Ring_Buffer<T, N, Ring_Buffer_Mode::INPUT_BYTES> : public system_internal::ring_buffer_internal::Ring_Buffer_Base<T, N>
{
  // 禁止拷贝和移动
  NO_COPY_MOVE(Ring_Buffer)

private:
  template <device::Stream_Type Type, uint32_t In_Buf_Size, uint32_t Out_Buf_Size, memory::Ring_Buffer_Mode In_Buf_Mode>
  friend class device::Stream_Device;

  // 基类类型
  using Base   = system_internal::ring_buffer_internal::Ring_Buffer_Base<T, N>;
  // 返回值类型
  using Status = typename Base::Status;

private:
  /**
   * @brief 环形缓冲区 写入数据
   *
   * @param  data    要写入的数据(引用)
   * @return Status  写入结果
   */
  Status push(const T& data) noexcept
  {
    return Base::push(data);
  }

  /**
   * @brief  环形缓冲区 批量写入数据
   *
   * @param  data     要写入的数据(指针)
   * @param  request  要写入的数据数量
   * @return uint32_t 实际写入的数据数量
   */
  uint32_t write(const T* data, uint32_t request) noexcept
  {
    return Base::write(data, request);
  }

public:
  /**
   * @brief  环形缓冲区 构造函数
   *
   */
  explicit Ring_Buffer(const char* name = "Ring Buffer") : Base(name) {}

  /**
   * @brief  环形缓冲区 读取数据
   *
   * @param  data     要读取的数据(引用)
   * @return Status   读取结果
   */
  Status pop(T& data) noexcept
  {
    return Base::pop(data);
  }

  /**
   * @brief  环形缓冲区 批量读取数据
   *
   * @param  data     要读取的数据(指针)
   * @param  request  要读取的数据数量
   * @return uint32_t 实际读取的数据数量
   */
  uint32_t read(T* data, uint32_t request) noexcept
  {
    return Base::read(data, request);
  }

  /**
   * @brief  环形缓冲区 回滚
   *
   * @return Status   回滚结果
   */
  Status roll_back(void) noexcept
  {
    return Base::roll_back();
  }

  /**
   * @brief  环形缓冲区 探视数据
   *
   * @param  data     要探视的数据(指针)
   * @param  request  要探视的数据数量
   * @return uint32_t 实际探视的数据数量
   */
  uint32_t peek(T* data, uint32_t request) noexcept
  {
    return Base::peek(data, request);
  }

  /**
   * @brief  环形缓冲区 析构函数
   *
   */
  virtual ~Ring_Buffer() {}
};

/**
 * @brief  环形缓冲区模板类 - 单缓冲区输入模式特化
 *
 * @tparam T    元素类型
 * @tparam N    缓冲区大小，必须是2的幂次方，且小于等于4096
 */
template <typename T, uint32_t N>
class Ring_Buffer<T, N, Ring_Buffer_Mode::INPUT_SINGLE_BUFFER> : public system_internal::ring_buffer_internal::Ring_Buffer_Base<T, N>
{
  // 禁止拷贝和移动
  NO_COPY_MOVE(Ring_Buffer)

private:
  template <device::Stream_Type Type, uint32_t In_Buf_Size, uint32_t Out_Buf_Size, memory::Ring_Buffer_Mode In_Buf_Mode>
  friend class device::Stream_Device;

  // 基类类型
  using Base   = system_internal::ring_buffer_internal::Ring_Buffer_Base<T, N>;
  // 返回值类型
  using Status = typename Base::Status;

  /// @brief 环形缓冲区 额外缓冲区
  T    m_ex_buffer[N] ALIGN(32);
  /// @brief 是否使用额外缓冲区
  bool m_is_used_ex_buffer = false;

private:
  /**
   * @brief  环形缓冲区 单缓冲区输入模式 开始输入
   *
   * @param  size     数据大小 (需小于等于缓冲区大小减1)
   * @return T*       输入起始地址指针
   */
  T* input_start(uint32_t size = N - 1)
  {
    if (size > N - 1)
    {
      return nullptr;
    }

    const uint32_t current_head = this->m_head;
    const uint32_t current_tail = this->m_tail;
    const uint32_t ptr_to_tail  = N - current_tail;
    const uint32_t new_tail     = (current_tail + size) & (N - 1);

    if ((size > ptr_to_tail) || size > Base::space())
    {
      m_is_used_ex_buffer = true;
      return m_ex_buffer;
    }
    else
    {
      kernel::Interrupt_Guard lock;

      if (new_tail > current_head && current_head > current_tail)
      {
        this->m_head = new_tail;
      }

      return &this->m_buffer[current_tail];
    }
  }

  /**
   * @brief 环形缓冲区 单缓冲区输入模式 完成输入
   *
   * @param size     DMA写入的数据大小
   */
  void input_complete(uint32_t size)
  {
    const uint32_t current_tail = this->m_tail;
    const uint32_t new_tail     = (current_tail + size) & (N - 1);

    if (m_is_used_ex_buffer)
    {
      Base::write(m_ex_buffer, size);
      m_is_used_ex_buffer = false;
    }

    kernel::Interrupt_Guard lock;
    this->m_tail = new_tail;
  }

public:
  /**
   * @brief  环形缓冲区 构造函数
   *
   */
  explicit Ring_Buffer(const char* name = "Ring Buffer") : Base(name) {}

  /**
   * @brief  环形缓冲区 读取数据
   *
   * @param  data     要读取的数据(引用)
   * @return Status   读取结果
   */
  Status pop(T& data) noexcept
  {
    return Base::pop(data);
  }

  /**
   * @brief  环形缓冲区 批量读取数据
   *
   * @param  data     要读取的数据(指针)
   * @param  request  要读取的数据数量
   * @return uint32_t 实际读取的数据数量
   */
  uint32_t read(T* data, uint32_t request) noexcept
  {
    return Base::read(data, request);
  }

  /**
   * @brief  环形缓冲区 回滚
   *
   * @return Status   回滚结果
   */
  Status roll_back(void) noexcept
  {
    return Base::roll_back();
  }

  /**
   * @brief  环形缓冲区 探视数据
   *
   * @param  data     要探视的数据(指针)
   * @param  request  要探视的数据数量
   * @return uint32_t 实际探视的数据数量
   */
  uint32_t peek(T* data, uint32_t request) noexcept
  {
    return Base::peek(data, request);
  }

  /**
   * @brief  环形缓冲区 析构函数
   *
   */
  virtual ~Ring_Buffer() {}
};

/**
 * @brief  环形缓冲区模板类 - 双缓冲区输入模式特化
 *
 * @tparam T    元素类型
 * @tparam N    缓冲区大小，必须是2的幂次方，且小于等于4096
 */
template <typename T, uint32_t N>
class Ring_Buffer<T, N, Ring_Buffer_Mode::INPUT_DOUBLE_BUFFER> : public system_internal::ring_buffer_internal::Ring_Buffer_Base<T, N>
{
  // 禁止拷贝和移动
  NO_COPY_MOVE(Ring_Buffer)

private:
  template <device::Stream_Type Type, uint32_t In_Buf_Size, uint32_t Out_Buf_Size, memory::Ring_Buffer_Mode In_Buf_Mode>
  friend class device::Stream_Device;

  // 基类类型
  using Base   = system_internal::ring_buffer_internal::Ring_Buffer_Base<T, N>;
  // 返回值类型
  using Status = typename Base::Status;

  /// @brief 环形缓冲区 第一个缓冲区
  T    m_first_buffer[N / 2] ALIGN(32);
  /// @brief 环形缓冲区 第二个缓冲区
  T    m_second_buffer[N / 2] ALIGN(32);
  /// @brief 当前缓冲区
  bool m_current_buffer = 0;

private:
  /**
   * @brief  环形缓冲区 双缓冲区输入模式 开始输入
   *
   * @param  ptr1      数据域1指针引用
   * @param  ptr2      数据域2指针引用
   * @return uint32_t  允许写入的最大数据大小
   */
  uint32_t input_start(T*& ptr1, T*& ptr2)
  {
    ptr1             = m_first_buffer;
    ptr2             = m_second_buffer;
    m_current_buffer = 0;
    return N / 2;
  }

  /**
   * @brief  环形缓冲区 双缓冲区输入模式 切换缓冲区
   * @return true      成功
   * @return false     失败
   */
  bool switch_buffer(void)
  {
    if (Base::write(m_current_buffer ? m_second_buffer : m_first_buffer, N / 2) == N / 2)
    {
      m_current_buffer = !m_current_buffer;
      return true;
    }
    else
    {
      return false;
    }
  }

  /**
   * @brief  环形缓冲区 双缓冲区输入模式 完成输入
   *
   * @param  size     当前缓冲区写入的数据大小
   * @return true     成功
   * @return false    失败
   */
  bool input_complete(uint32_t size)
  {
    return (Base::write(m_current_buffer ? m_second_buffer : m_first_buffer, size) == size);
  }

public:
  /**
   * @brief  环形缓冲区 构造函数
   *
   */
  explicit Ring_Buffer(const char* name = "Ring Buffer") : Base(name) {}

  /**
   * @brief  环形缓冲区 读取数据
   *
   * @param  data     要读取的数据(引用)
   * @return Status   读取结果
   */
  Status pop(T& data) noexcept
  {
    return Base::pop(data);
  }

  /**
   * @brief  环形缓冲区 批量读取数据
   *
   * @param  data     要读取的数据(指针)
   * @param  request  要读取的数据数量
   * @return uint32_t 实际读取的数据数量
   */
  uint32_t read(T* data, uint32_t request) noexcept
  {
    return Base::read(data, request);
  }

  /**
   * @brief  环形缓冲区 回滚
   *
   * @return Status   回滚结果
   */
  Status roll_back(void) noexcept
  {
    return Base::roll_back();
  }

  /**
   * @brief  环形缓冲区 探视数据
   *
   * @param  data     要探视的数据(指针)
   * @param  request  要探视的数据数量
   * @return uint32_t 实际探视的数据数量
   */
  uint32_t peek(T* data, uint32_t request) noexcept
  {
    return Base::peek(data, request);
  }

  /**
   * @brief  环形缓冲区 析构函数
   *
   */
  virtual ~Ring_Buffer() {}
};

/**
 * @brief  环形缓冲区模板类 - 输出模式特化
 *
 * @tparam T    元素类型
 * @tparam N    缓冲区大小，必须是2的幂次方，且小于等于4096
 */
template <typename T, uint32_t N>
class Ring_Buffer<T, N, Ring_Buffer_Mode::OUTPUT> : public system_internal::ring_buffer_internal::Ring_Buffer_Base<T, N>
{
  // 禁止拷贝和移动
  NO_COPY_MOVE(Ring_Buffer)

private:
  template <device::Stream_Type Type, uint32_t In_Buf_Size, uint32_t Out_Buf_Size, memory::Ring_Buffer_Mode In_Buf_Mode>
  friend class device::Stream_Device;

  // 基类类型
  using Base   = system_internal::ring_buffer_internal::Ring_Buffer_Base<T, N>;
  // 返回值类型
  using Status = typename Base::Status;

  /// @brief 环形缓冲区 额外缓冲区
  T        m_ex_buffer[N] ALIGN(32);
  /// @brief 输出的数据长度
  uint32_t m_output_size = 0;

private:
  /**
   * @brief   环形缓冲区 输出模式 开始输出
   *
   * @param   size     数据大小(通过引用返回)
   * @return  T*       输出起始地址指针
   */
  T* output_start(uint32_t& size)
  {
    const uint32_t used         = this->available();
    const uint32_t current_head = this->m_head;
    const uint32_t current_tail = this->m_tail;
    const uint32_t new_head     = (current_head + size) & (N - 1);

    if (current_tail >= current_head)
    {
      kernel::Interrupt_Guard lock;
      if (new_head > current_tail && current_tail > current_head)
      {
        this->m_tail = new_head;
      }

      memory::fast_memcpy(m_ex_buffer, &this->m_buffer[current_head], used * sizeof(T));

      m_output_size = size = used;
      return m_ex_buffer;
    }
    else
    {
      const uint32_t first_copy_size  = N - (current_head & (N - 1));
      const uint32_t second_copy_size = (current_tail) & (N - 1);

      memory::fast_memcpy(m_ex_buffer, &this->m_buffer[current_head], first_copy_size * sizeof(T));
      memory::fast_memcpy(m_ex_buffer + first_copy_size, this->m_buffer, second_copy_size * sizeof(T));

      m_output_size = size = first_copy_size + second_copy_size;
      return m_ex_buffer;
    }
  }

  /**
   * @brief 环形缓冲区 输出模式 完成输出
   *
   */
  void output_complete(void)
  {
    const uint32_t current_head = this->m_head;
    const uint32_t new_head     = (current_head + m_output_size) & (N - 1);

    this->m_head                = new_head;
    m_output_size               = 0;
  }

public:
  /**
   * @brief  环形缓冲区 构造函数
   *
   */
  explicit Ring_Buffer(const char* name = "Ring Buffer") : Base(name) {}

  /**
   * @brief 环形缓冲区 写入数据
   *
   * @param  data    要写入的数据(引用)
   * @return Status  写入结果
   */
  Status push(const T& data) noexcept
  {
    return Base::push(data);
  }

  /**
   * @brief  环形缓冲区 批量写入数据
   *
   * @param  data     要写入的数据(指针)
   * @param  request  要写入的数据数量
   * @return uint32_t 实际写入的数据数量
   */
  uint32_t write(const T* data, uint32_t request) noexcept
  {
    return Base::write(data, request);
  }

  /**
   * @brief  环形缓冲区 析构函数
   *
   */
  virtual ~Ring_Buffer() {}
};
} /* namespace memory */
} /* namespace system */
} /* namespace QAQ */

#endif /* __RING_BUFFER_HPP__ */
