#ifndef __QSTRING_BASE_HPP__
#define __QSTRING_BASE_HPP__

#include "qstring_memory.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 容器
namespace container
{
/// @brief 名称空间 内部
namespace container_internal
{
/// @brief 名称空间 动态字符串内部
namespace qstring_internal
{
/**
 * @brief 动态字符串 基类
 *
 */
class QString_Base
{
private:
  /// @brief SSO存储阈值
  static constexpr uint32_t SSO_THRESHOLD = 16;

  union
  {
    /// @brief SSO存储区
    struct
    {
      /// @brief SSO存储区
      char    data[SSO_THRESHOLD + 1];
      /// @brief 长度
      uint8_t size   : 7;
      /// @brief SSO存储标志
      uint8_t is_sso : 1;
    } sso;

    /// @brief 动态存储区
    struct
    {
      /// @brief 动态存储区指针
      char*                  data;
      /// @brief 长度
      uint32_t               size;
      /// @brief 容量
      uint32_t               capacity;
      /// @brief 引用计数
      std::atomic<uint32_t>* ref_count;
    } heap;
  } storage;

protected:
  /**
   * @brief  动态字符串基类 获取当前存储类型
   *
   * @return bool  是否为SSO存储
   */
  bool is_sso(void) const noexcept
  {
    return storage.sso.is_sso;
  }

  /**
   * @brief  动态字符串基类 初始化引用计数
   */
  void init_ref_count(void) noexcept
  {
    if (!storage.sso.is_sso)
    {
      storage.heap.ref_count = QString_Memory_Pool::instance().counter_malloc();
      storage.heap.ref_count->store(0);
    }
  }

  /**
   * @brief  动态字符串基类 增加引用计数
   */
  void add_ref(void) noexcept
  {
    if (!storage.sso.is_sso && storage.heap.ref_count)
    {
      storage.heap.ref_count->fetch_add(1);
    }
  }

  /**
   * @brief  动态字符串基类 减少引用计数 (如果为0则释放内存)
   */
  void release(void) noexcept
  {
    if (!storage.sso.is_sso && storage.heap.ref_count)
    {
      if (0 == storage.heap.ref_count->load())
      {
        QString_Memory_Pool::instance().free(storage.heap.data, storage.heap.capacity);
        QString_Memory_Pool::instance().counter_free(storage.heap.ref_count);
      }
      else
      {
        storage.heap.ref_count->fetch_sub(1);
      }
    }
  }

  /**
   * @brief  动态字符串基类 分离共享数据 (写时复制)
   */
  void detach(void)
  {
    if (!storage.sso.is_sso && storage.heap.ref_count)
    {
      if (storage.heap.ref_count->load())
      {
        QString_Base new_string;
        new_string.storage.heap.capacity = QString_Memory_Pool::instance().get_capacity(storage.heap.capacity);
        new_string.storage.heap.data     = QString_Memory_Pool::instance().malloc(new_string.storage.heap.capacity);
        new_string.storage.heap.size     = storage.heap.size;
        new_string.storage.sso.is_sso    = 0;

        memcpy(new_string.storage.heap.data, storage.heap.data, storage.heap.size);
        new_string.storage.heap.data[storage.heap.size] = '\0';

        new_string.init_ref_count();
        swap_impl(new_string);
      }
    }
  }

  /**
   * @brief  动态字符串基类 确保有足够的容量
   *
   * @param  new_capacity  新容量
   * @param  keep_data     是否保留原有数据
   */
  void reserve_impl(uint32_t new_capacity, bool keep_data = true)
  {
    if (storage.sso.is_sso)
    {
      if (SSO_THRESHOLD < new_capacity)
      {
        QString_Base new_string;
        new_string.storage.heap.capacity = QString_Memory_Pool::instance().get_capacity(new_capacity + 1);
        new_string.storage.heap.data     = QString_Memory_Pool::instance().malloc(new_string.storage.heap.capacity);
        new_string.storage.heap.size     = storage.sso.size;
        new_string.storage.sso.is_sso    = 0;

        if (keep_data)
        {
          memcpy(new_string.storage.heap.data, storage.sso.is_sso ? storage.sso.data : storage.heap.data, size_impl());
          new_string.storage.heap.data[storage.sso.size] = '\0';
        }
        else
        {
          new_string.storage.heap.size    = 0;
          new_string.storage.heap.data[0] = '\0';
        }

        new_string.init_ref_count();
        swap_impl(new_string);
      }
    }
    else
    {
      if (new_capacity + 1 >= storage.heap.capacity)
      {
        if (nullptr != storage.heap.ref_count)
        {
          QString_Base new_string;
          new_string.storage.heap.capacity = QString_Memory_Pool::instance().get_capacity(new_capacity + 1);
          new_string.storage.heap.data     = QString_Memory_Pool::instance().malloc(new_string.storage.heap.capacity);
          new_string.storage.heap.size     = keep_data ? storage.heap.size : 0;
          new_string.storage.sso.is_sso    = 0;

          if (keep_data)
          {
            memcpy(new_string.storage.heap.data, storage.heap.data, storage.heap.size);
            new_string.storage.heap.data[storage.heap.size] = '\0';
          }
          else
          {
            new_string.storage.heap.data[0] = '\0';
          }

          new_string.init_ref_count();
          swap_impl(new_string);
        }
      }
    }
  }

  /**
   * @brief  动态字符串基类 获取数据指针 (const版本)
   *
   * @return const char*  数据指针
   */
  const char* data_impl(void) const noexcept
  {
    return storage.sso.is_sso ? storage.sso.data : storage.heap.data;
  }

  /**
   * @brief  动态字符串基类 获取数据指针 (非const版本，会触发COW)
   *
   * @return char*  数据指针
   */
  char* data_impl(void)
  {
    if (!storage.sso.is_sso)
    {
      detach();
      return storage.heap.data;
    }
    else
    {
      return storage.sso.data;
    }
  }

  /**
   * @brief  动态字符串基类 获取大小
   *
   * @return uint32_t  字符串大小
   */
  uint32_t size_impl(void) const noexcept
  {
    return storage.sso.is_sso ? storage.sso.size : storage.heap.size;
  }

  /**
   * @brief  动态字符串基类 获取容量
   *
   * @return uint32_t  字符串容量
   */
  uint32_t capacity_impl(void) const noexcept
  {
    return storage.sso.is_sso ? SSO_THRESHOLD : storage.heap.capacity - 1;
  }

  /**
   * @brief  动态字符串基类 设置大小
   *
   * @param  new_size  新大小
   */
  void set_size(uint32_t new_size) noexcept
  {
    if (storage.sso.is_sso)
    {
      storage.sso.size           = static_cast<unsigned char>(new_size);
      storage.sso.data[new_size] = '\0';
    }
    else
    {
      storage.heap.size           = new_size;
      storage.heap.data[new_size] = '\0';
    }
  }

  /**
   * @brief  动态字符串基类 清空数据
   */
  void clear_impl(void) noexcept
  {
    if (storage.sso.is_sso)
    {
      storage.sso.data[0] = '\0';
      storage.sso.size    = 0;
    }
    else
    {
      detach();
      storage.heap.data[0] = '\0';
      storage.heap.size    = 0;
    }
  }

  /**
   * @brief  动态字符串基类 从C字符串初始化
   *
   * @param  str  C字符串
   * @param  len  字符串长度
   */
  void init_from_cstr(const char* str, uint32_t len)
  {
    if (SSO_THRESHOLD >= len)
    {
      if (str && 0 < len)
      {
        memcpy(storage.sso.data, str, len);
        storage.sso.size = static_cast<uint8_t>(len);
      }
      else
      {
        storage.sso.size = 0;
      }

      storage.sso.data[len] = '\0';
      storage.sso.is_sso    = 1;
    }
    else
    {
      storage.heap.capacity = QString_Memory_Pool::instance().get_capacity(len + 1);
      storage.heap.data     = QString_Memory_Pool::instance().malloc(storage.heap.capacity);

      if (str && 0 < len)
      {
        memcpy(storage.heap.data, str, len);
      }

      storage.heap.data[len] = '\0';
      storage.heap.size      = len;
      storage.sso.is_sso     = 0;
      init_ref_count();
    }
  }

  /**
   * @brief  动态字符串基类 从填充字符初始化
   *
   * @param  count  字符数量
   * @param  ch     填充字符
   */
  void init_from_fill(uint32_t count, char ch)
  {
    if (SSO_THRESHOLD >= count)
    {
      memset(storage.sso.data, ch, count);
      storage.sso.data[count] = '\0';
      storage.sso.size        = static_cast<uint8_t>(count);
      storage.sso.is_sso      = 1;
    }
    else
    {
      storage.heap.capacity = QString_Memory_Pool::instance().get_capacity(count + 1);
      storage.heap.data     = QString_Memory_Pool::instance().malloc(storage.heap.capacity);
      memset(storage.heap.data, ch, count);
      storage.heap.data[count] = '\0';
      storage.heap.size        = count;
      storage.sso.is_sso       = 0;
      init_ref_count();
    }
  }

  /**
   * @brief  动态字符串基类 拷贝初始化 (COW)
   *
   * @param  other  源对象
   */
  void init_from_copy(const QString_Base& other) noexcept
  {
    if (other.storage.sso.is_sso)
    {
      memcpy(&storage.sso.data[0], &other.storage.sso.data[0], other.storage.sso.size + 1);
      storage.sso.size   = other.storage.sso.size;
      storage.sso.is_sso = 1;
    }
    else
    {
      storage.heap.data      = other.storage.heap.data;
      storage.heap.size      = other.storage.heap.size;
      storage.heap.capacity  = other.storage.heap.capacity;
      storage.heap.ref_count = other.storage.heap.ref_count;
      storage.sso.is_sso     = 0;
      add_ref();
    }
  }

  /**
   * @brief  动态字符串基类 移动初始化
   *
   * @param  other  源对象
   */
  void init_from_move(QString_Base&& other) noexcept
  {
    if (other.storage.sso.is_sso)
    {
      memcpy(storage.sso.data, other.storage.sso.data, other.storage.sso.size + 1);
      storage.sso.size          = other.storage.sso.size;
      storage.sso.is_sso        = 1;

      other.storage.sso.data[0] = '\0';
      other.storage.sso.size    = 0;
    }
    else
    {
      storage.heap.data         = other.storage.heap.data;
      storage.heap.size         = other.storage.heap.size;
      storage.heap.capacity     = other.storage.heap.capacity;
      storage.heap.ref_count    = other.storage.heap.ref_count;
      storage.sso.is_sso        = 0;

      other.storage.sso.data[0] = '\0';
      other.storage.sso.size    = 0;
      other.storage.sso.is_sso  = 1;
    }
  }

  /**
   * @brief  动态字符串基类 交换实现
   *
   * @param  other  另一个对象
   */
  void swap_impl(QString_Base& other) noexcept
  {
    if (storage.sso.is_sso && other.storage.sso.is_sso)
    {
      std::swap(storage.sso, other.storage.sso);
    }
    else if (!storage.sso.is_sso && !other.storage.sso.is_sso)
    {
      std::swap(storage.heap.data, other.storage.heap.data);
      std::swap(storage.heap.size, other.storage.heap.size);
      std::swap(storage.heap.capacity, other.storage.heap.capacity);
      std::swap(storage.heap.ref_count, other.storage.heap.ref_count);
    }
    else
    {
      QString_Base temp = std::move(*this);
      *this             = std::move(other);
      other             = std::move(temp);
    }
  }

  /**
   * @brief  动态字符串基类 调整大小实现
   *
   * @param  new_size  新大小
   * @param  ch        填充字符
   */
  void resize_impl(uint32_t new_size, char ch = '\0')
  {
    if (size_impl() > new_size)
    {
      set_size(new_size);
    }
    else if (size_impl() < new_size)
    {
      reserve_impl(new_size);
      detach();

      uint32_t old_size = size_impl();
      char*    ptr      = storage.sso.is_sso ? storage.sso.data : storage.heap.data;

      memset(ptr + old_size, ch, new_size - old_size);
      set_size(new_size);
    }
  }

  /**
   * @brief  动态字符串基类 计算容量
   *
   * @param  target_size  目标容量
   * @return uint32_t     容量
   */
  uint32_t calculate_growth_capacity(uint32_t target_size)
  {
    uint32_t current_capacity = capacity_impl();
    return std::max(target_size, current_capacity * 2);
  }

  /**
   * @brief  动态字符串基类 追加数据实现
   *
   * @param  str    要追加的字符串
   * @param  count  字符数量
   */
  void append_impl(const char* str, uint32_t count)
  {
    if (0 < count)
    {
      uint32_t old_size = size_impl();
      uint32_t new_size = old_size + count;

      if (capacity_impl() < new_size)
      {
        uint32_t new_capacity = calculate_growth_capacity(new_size);
        reserve_impl(new_capacity);
      }
      else
      {
        detach();
      }

      char* ptr = (storage.sso.is_sso ? storage.sso.data : storage.heap.data) + old_size;

      if (str)
      {
        memcpy(ptr, str, count);
      }
      else
      {
        memset(ptr, '\0', count);
      }

      set_size(new_size);
    }
  }

  /**
   * @brief 动态字符串基类 追加字符实现
   *
   * @param ch    要追加的字符
   * @param count 字符数量
   */
  void append_impl(char ch, uint32_t count)
  {
    if (1 == count)
    {
      if (size_impl() < capacity_impl())
      {
        detach();
        (storage.sso.is_sso ? storage.sso.data : storage.heap.data)[size_impl()] = ch;
      }
      else
      {
        reserve_impl(calculate_growth_capacity(size_impl() + 1));
        (storage.sso.is_sso ? storage.sso.data : storage.heap.data)[size_impl()] = ch;
      }

      set_size(size_impl() + 1);
    }
    else if (1 < count)
    {
      uint32_t old_size = size_impl();
      uint32_t new_size = old_size + count;

      if (capacity_impl() < new_size)
      {
        reserve_impl(calculate_growth_capacity(new_size));
      }
      else
      {
        detach();
      }

      char* ptr = (storage.sso.is_sso ? storage.sso.data : storage.heap.data) + old_size;
      memset(ptr, ch, count);
      set_size(new_size);
    }
  }

  /**
   * @brief  动态字符串基类 插入数据实现
   *
   * @param  index  插入位置
   * @param  str    要插入的字符串
   * @param  count  字符数量
   */
  void insert_impl(uint32_t index, const char* str, uint32_t count)
  {
    if (size_impl() < index || 0 == count)
    {
      return;
    }

    uint32_t old_size = size_impl();
    uint32_t new_size = old_size + count;

    if (capacity_impl() < new_size)
    {
      uint32_t new_capacity = calculate_growth_capacity(new_size);
      reserve_impl(new_capacity);
    }
    else
    {
      detach();
    }

    char* ptr = storage.sso.is_sso ? storage.sso.data : storage.heap.data;

    if (index < old_size)
    {
      memmove(ptr + index + count, ptr + index, old_size - index);
    }

    memcpy(ptr + index, str, count);
    set_size(new_size);
  }

  /**
   * @brief 动态字符串基类 插入字符实现
   *
   * @param index 插入位置
   * @param ch    要插入的字符
   * @param count 字符数量
   */
  void insert_impl(uint32_t index, char ch, uint32_t count)
  {
    if (size_impl() < index || 0 == count)
    {
      return;
    }

    uint32_t old_size = size_impl();
    uint32_t new_size = old_size + count;

    if (capacity_impl() < new_size)
    {
      uint32_t new_capacity = calculate_growth_capacity(new_size);
      reserve_impl(new_capacity);
    }
    else
    {
      detach();
    }

    char* ptr = storage.sso.is_sso ? storage.sso.data : storage.heap.data;

    if (index < old_size)
    {
      memmove(ptr + index + count, ptr + index, old_size - index);
    }

    if (1 == count)
    {
      *(ptr + index) = ch;
    }
    else
    {
      memset(ptr + index, ch, count);
    }

    set_size(new_size);
  }

  /**
   * @brief  动态字符串基类 删除数据实现
   *
   * @param  index  删除起始位置
   * @param  count  删除字符数量
   */
  void erase_impl(uint32_t index, uint32_t count)
  {
    if (size_impl() <= index || 0 == count)
    {
      return;
    }

    count = std::min(count, size_impl() - index);
    detach();

    char*    ptr      = storage.sso.is_sso ? storage.sso.data : storage.heap.data;
    uint32_t new_size = size_impl() - count;

    if (size_impl() > (index + count))
    {
      memmove(ptr + index, ptr + index + count, size_impl() - index - count);
    }

    set_size(new_size);
  }

  /**
   * @brief  动态字符串基类 替换数据实现
   *
   * @param  pos        替换起始位置
   * @param  count      替换字符数量
   * @param  str        新字符串
   * @param  str_count  新字符串字符数量
   */
  void replace_impl(uint32_t pos, uint32_t count, const char* str, uint32_t str_count)
  {
    if (size_impl() < pos)
    {
      return;
    }

    count             = std::min(count, size_impl() - pos);
    uint32_t new_size = size_impl() - count + str_count;

    if (capacity_impl() < new_size)
    {
      QString_Base new_string;
      new_string.reserve_impl(new_size, false);

      if (pos)
      {
        new_string.append_impl(storage.sso.is_sso ? storage.sso.data : storage.heap.data, pos);
      }

      new_string.append_impl(str, str_count);

      if (size_impl() > pos + count)
      {
        new_string.append_impl((storage.sso.is_sso ? storage.sso.data : storage.heap.data) + pos + count, size_impl() - pos - count);
      }

      swap_impl(new_string);
    }
    else
    {
      detach();
      char* dest = storage.sso.is_sso ? storage.sso.data : storage.heap.data;

      if (str_count != count)
      {
        memmove(dest + pos + str_count, dest + pos + count, size_impl() - pos - count);
      }

      memcpy(dest + pos, str, str_count);
      set_size(new_size);
    }
  }

public:
  /**
   * @brief  动态字符串基类 默认构造函数
   */
  QString_Base() noexcept
  {
    storage.sso.data[0] = '\0';
    storage.sso.size    = 0;
    storage.sso.is_sso  = 1;
  }

  /**
   * @brief  动态字符串基类 从C字符串构造
   *
   * @param  str  C字符串
   */
  QString_Base(const char* str)
  {
    if (str)
    {
      init_from_cstr(str, strlen(str));
    }
    else
    {
      storage.sso.data[0] = '\0';
      storage.sso.size    = 0;
      storage.sso.is_sso  = 1;
    }
  }

  /**
   * @brief  动态字符串基类 从字符数组构造
   *
   * @param  data  字符数据
   * @param  len   数据长度
   */
  QString_Base(const char* data, uint32_t len)
  {
    init_from_cstr(data, len);
  }

  /**
   * @brief  动态字符串基类 拷贝构造函数 (COW)
   *
   * @param  other  源对象
   */
  QString_Base(const QString_Base& other) noexcept
  {
    init_from_copy(other);
  }

  /**
   * @brief  动态字符串基类 移动构造函数
   *
   * @param  other  源对象
   */
  QString_Base(QString_Base&& other) noexcept
  {
    init_from_move(std::move(other));
  }

  /**
   * @brief  动态字符串基类 填充构造函数
   *
   * @param  count  字符数量
   * @param  ch     填充字符
   */
  QString_Base(uint32_t count, char ch)
  {
    init_from_fill(count, ch);
  }

  /**
   * @brief  动态字符串基类 析构函数
   */
  virtual ~QString_Base()
  {
    if (!storage.sso.is_sso)
    {
      release();
    }
  }

  /**
   * @brief  动态字符串基类 拷贝赋值 (COW)
   *
   * @param  other  源对象
   * @return QString_Base&  当前对象引用
   */
  QString_Base& operator=(const QString_Base& other) noexcept
  {
    if (&other != this)
    {
      if (!storage.sso.is_sso)
      {
        release();
      }

      init_from_copy(other);
    }

    return *this;
  }

  /**
   * @brief  动态字符串基类 移动赋值
   *
   * @param  other  源对象
   * @return QString_Base&  当前对象引用
   */
  QString_Base& operator=(QString_Base&& other) noexcept
  {
    if (&other != this)
    {
      if (!storage.sso.is_sso)
      {
        release();
      }

      init_from_move(std::move(other));
    }

    return *this;
  }

  /**
   * @brief  动态字符串基类 从C字符串赋值
   *
   * @param  str  C字符串
   * @return QString_Base&  当前对象引用
   */
  QString_Base& operator=(const char* str)
  {
    if (str)
    {
      uint32_t len = strlen(str);

      if (SSO_THRESHOLD > len)
      {
        if (!storage.sso.is_sso)
        {
          release();
        }

        memcpy(storage.sso.data, str, len);
        storage.sso.data[len] = '\0';
        storage.sso.size      = len;
        storage.sso.is_sso    = 1;
      }
      else
      {
        reserve_impl(len, false);

        memcpy(storage.heap.data, str, len);
        storage.heap.data[len] = '\0';
        storage.heap.size      = len;
        storage.sso.is_sso     = 0;
      }
    }
    else
    {
      clear_impl();
    }

    return *this;
  }

  /**
   * @brief  动态字符串基类 从单个字符赋值
   *
   * @param  ch  字符
   * @return QString_Base&  当前对象引用
   */
  QString_Base& operator=(char ch)
  {
    if (storage.sso.is_sso)
    {
      storage.sso.data[0] = ch;
      storage.sso.data[1] = '\0';
      storage.sso.size    = 1;
      storage.sso.is_sso  = 1;
    }
    else
    {
      release();

      storage.sso.data[0] = ch;
      storage.sso.data[1] = '\0';
      storage.sso.size    = 1;
      storage.sso.is_sso  = 1;
    }

    return *this;
  }

  /**
   * @brief  动态字符串基类 交换
   *
   * @param  first   第一个对象
   * @param  second  第二个对象
   */
  friend void swap(QString_Base& first, QString_Base& second) noexcept
  {
    first.swap_impl(second);
  }
};
} /* namespace qstring_internal */
} /* namespace container_internal */
} /* namespace container */
} /* namespace QAQ */

#endif /* __QSTRING_BASE_HPP__ */
