#ifndef __QSTRING_HPP__
#define __QSTRING_HPP__

#include "qstring_base.hpp"
#include "format.hpp"
#include "parse.hpp"
#include <algorithm>

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 容器
namespace container
{
/**
 * @brief 动态字符串
 *
 */
class QString final : private container_internal::qstring_internal::QString_Base
{
private:
  /// @brief BMH 匹配表大小
  static constexpr uint32_t BMH_SKIPER_SIZE = 256;
  /// @brief 动态字符串 基类类型
  using Base                                = container_internal::qstring_internal::QString_Base;

public:
  /// @brief 动态字符串 迭代器类型
  using iterator       = char*;
  /// @brief 动态字符串 常量迭代器类型
  using const_iterator = const char*;

private:
  /**
   * @brief  动态字符串 字符解析 判断字符是否为空白字符
   *
   * @param  c      待判断的字符
   * @return true   字符为空白字符
   * @return false  字符不为空白字符
   */
  static const bool O3 is_space(char c) noexcept
  {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
  }

  /**
   * @brief  动态字符串 字符解析 判断字符是否为数字字符
   *
   * @param  c      待判断的字符
   * @return true   字符为数字字符
   * @return false  字符不为数字字符
   */
  static const bool O3 is_digit(char c) noexcept
  {
    return c >= '0' && c <= '9';
  }

  /**
   * @brief  动态字符串 字符解析 判断字符是否为字母字符
   *
   * @param  c      待判断的字符
   * @return true   字符为字母字符
   * @return false  字符不为字母字符
   */
  static const bool O3 is_alpha(char c) noexcept
  {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
  }

  /**
   * @brief  动态字符串 字符解析 判断字符是否为大写字母
   *
   * @param  c      待判断的字符
   * @return true   字符为大写字母
   * @return false  字符不为大写字母
   */
  static const bool O3 is_upper(char c) noexcept
  {
    return c >= 'A' && c <= 'Z';
  }

  /**
   * @brief  动态字符串 字符解析 判断字符是否为小写字母
   *
   * @param  c      待判断的字符
   * @return true   字符为小写字母
   * @return false  字符不为小写字母
   */
  static const bool O3 is_lower(char c) noexcept
  {
    return c >= 'a' && c <= 'z';
  }

private:
  /**
   * @brief  动态字符串 使用Boyer-Moore-Horspool算法正向查找
   */
  static const char* O3 bmh_find(const char* text, uint32_t text_len, const char* pattern, uint32_t pattern_len) noexcept
  {
    if (0 == pattern_len)
      return text;
    if (text_len < pattern_len)
      return nullptr;

    // 构建坏字符表
    static uint8_t bad_char_skip[BMH_SKIPER_SIZE];
    for (uint32_t i = 0; i < BMH_SKIPER_SIZE; i++)
    {
      bad_char_skip[i] = static_cast<uint8_t>(pattern_len);
    }

    for (uint32_t i = 0; i < pattern_len - 1; i++)
    {
      bad_char_skip[static_cast<uint8_t>(pattern[i])] = static_cast<uint8_t>(pattern_len - 1 - i);
    }

    // BMH搜索
    uint32_t pos = 0;
    while (pos <= text_len - pattern_len)
    {
      int j = pattern_len - 1;
      while (j >= 0 && text[pos + j] == pattern[j])
      {
        j--;
      }

      if (j < 0)
      {
        return text + pos;   // 找到匹配
      }

      pos += bad_char_skip[static_cast<uint8_t>(text[pos + pattern_len - 1])];
    }

    return nullptr;
  }

  /**
   * @brief  动态字符串 使用Boyer-Moore-Horspool算法反向查找
   */
  static const char* O3 bmh_rfind(const char* text, uint32_t text_len, const char* pattern, uint32_t pattern_len) noexcept
  {
    if (0 == pattern_len)
      return text + text_len;
    if (text_len < pattern_len)
      return nullptr;

    // 构建坏字符表
    static uint8_t bad_char_skip[BMH_SKIPER_SIZE];
    for (uint32_t i = 0; i < BMH_SKIPER_SIZE; i++)
    {
      bad_char_skip[i] = static_cast<uint8_t>(pattern_len);
    }

    for (uint32_t i = pattern_len - 1; i > 0; i--)
    {
      bad_char_skip[static_cast<uint8_t>(pattern[i])] = static_cast<uint8_t>(i);
    }

    // 反向BMH搜索
    uint32_t pos = text_len - pattern_len;
    while (pos < text_len)
    {
      int j = 0;
      while (j < static_cast<int>(pattern_len) && text[pos + j] == pattern[j])
      {
        j++;
      }

      if (j >= static_cast<int>(pattern_len))
      {
        return text + pos;   // 找到匹配
      }

      if (pos >= bad_char_skip[static_cast<uint8_t>(text[pos])])
      {
        pos -= bad_char_skip[static_cast<uint8_t>(text[pos])];
      }
      else
      {
        break;
      }
    }

    return nullptr;
  }

public:
  /**
   * @brief  动态字符串 构造函数 默认构造函数
   */
  QString() noexcept {}

  /**
   * @brief  动态字符串 构造函数 从C字符串构造
   *
   * @param  str  C字符串指针
   */
  QString(const char* str) : Base(str) {}

  /**
   * @brief  动态字符串 构造函数 从指定长度的数据构造
   *
   * @param  data    数据指针
   * @param  length  数据长度
   */
  QString(const char* data, uint32_t length) : Base(data, length) {}

  /**
   * @brief  动态字符串 构造函数 拷贝构造函数
   *
   * @param  other  源QString对象
   */
  QString(const QString& other) noexcept : Base(other) {}

  /**
   * @brief  动态字符串 构造函数 移动构造函数
   *
   * @param  other  源QString对象
   */
  QString(QString&& other) noexcept : Base(std::move(other)) {}

  /**
   * @brief  动态字符串 构造函数 从字符构造
   *
   * @param  ch     字符
   * @param  count  字符重复次数，默认为1
   */
  QString(char ch, uint32_t count = 1) : Base(count, ch) {}

  /**
   * @brief  动态字符串 构造函数 从整数构造
   *
   * @param  value        整数值
   * @param  base         进制基数，默认为10
   * @param  uppercase    是否大写，默认为false
   * @param  add_prefix   是否添加前缀，默认为false
   */
  template <typename T, typename = std::enable_if_t<std::is_integral<T>::value && !std::is_same<T, char>::value>>
  QString(T value, int base = 10, bool uppercase = false, bool add_prefix = false)
  {
    char buffer[64];
    int  result = system::algorithm::Format::format(buffer, sizeof(buffer), value, base, uppercase, add_prefix);
    if (result > 0)
    {
      Base::init_from_cstr(buffer, result);
    }
    else
    {
      Base::init_from_cstr("", 0);
    }
  }

  /**
   * @brief  动态字符串 构造函数 从浮点数构造
   *
   * @param  value            浮点数值
   * @param  precision        精度，默认为6
   * @param  use_scientific   是否使用科学计数法，默认不使用
   */
  template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::value && !std::is_same<T, char>::value>>
  QString(T value, int precision = 6, bool use_scientific = false)
  {
    char buffer[64];
    int  result = system::algorithm::Format::format(buffer, sizeof(buffer), value, precision, use_scientific);
    if (result > 0)
    {
      Base::init_from_cstr(buffer, result);
    }
    else
    {
      Base::init_from_cstr("", 0);
    }
  }

  /**
   * @brief  动态字符串 析构函数
   */
  ~QString() {}

  /**
   * @brief  动态字符串 赋值运算符 拷贝赋值
   *
   * @param  other      源QString对象
   * @return QString&   当前对象引用
   */
  QString& operator=(const QString& other) noexcept
  {
    Base::operator=(other);
    return *this;
  }

  /**
   * @brief  动态字符串 赋值运算符 移动赋值
   *
   * @param  other      源QString对象
   * @return QString&   当前对象引用
   */
  QString& operator=(QString&& other) noexcept
  {
    Base::operator=(std::move(other));
    return *this;
  }

  /**
   * @brief  动态字符串 赋值运算符 从C字符串赋值
   *
   * @param  str        C字符串指针
   * @return QString&   当前对象引用
   */
  QString& operator=(const char* str)
  {
    Base::operator=(str);
    return *this;
  }

  /**
   * @brief  动态字符串 赋值运算符 从字符赋值
   *
   * @param  ch         字符
   * @return QString&   当前对象引用
   */
  QString& operator=(char ch)
  {
    Base::operator=(ch);
    return *this;
  }

  /**
   * @brief  动态字符串 赋值运算符 从整数赋值(十进制)或浮点数赋值
   *
   * @param  value      整数值
   * @return QString&   当前对象引用
   */
  template <typename T, typename = std::enable_if_t<!std::is_same<T, char>::value>>
  QString& operator=(T value)
  {
    char buffer[64];
    int  result = system::algorithm::Format::format(buffer, sizeof(buffer), value);
    if (result > 0)
    {
      Base::operator=(QString(buffer, result));
    }
    else
    {
      Base::operator=("");
    }
    return *this;
  }

  /**
   * @brief  动态字符串 迭代器 获取起始迭代器
   *
   * @return iterator  起始迭代器
   */
  iterator begin(void)
  {
    return data_impl();
  }

  /**
   * @brief  动态字符串 迭代器 获取结束迭代器
   *
   * @return iterator  结束迭代器
   */
  iterator end(void)
  {
    return data_impl() + size_impl();
  }

  /**
   * @brief  动态字符串 迭代器 获取起始常量迭代器
   *
   * @return const_iterator  起始常量迭代器
   */
  const_iterator begin(void) const
  {
    return data_impl();
  }

  /**
   * @brief  动态字符串 迭代器 获取结束常量迭代器
   *
   * @return const_iterator  结束常量迭代器
   */
  const_iterator end(void) const
  {
    return data_impl() + size_impl();
  }

  /**
   * @brief  动态字符串 迭代器 获取起始常量迭代器
   *
   * @return const_iterator  起始常量迭代器
   */
  const_iterator cbegin(void) const
  {
    return data_impl();
  }

  /**
   * @brief  动态字符串 迭代器 获取结束常量迭代器
   *
   * @return const_iterator  结束常量迭代器
   */
  const_iterator cend(void) const
  {
    return data_impl() + size_impl();
  }

  /**
   * @brief  动态字符串 元素访问 通过索引访问字符
   *
   * @param  index  索引位置
   * @return char&  索引位置的字符引用
   */
  char& operator[](uint32_t index)
  {
    if (index < size_impl())
      return data_impl()[index];
    else
      return data_impl()[size_impl() - 1];
  }

  /**
   * @brief  动态字符串 元素访问 通过索引访问常量字符
   *
   * @param  index        索引位置
   * @return const char&  索引位置的常量字符引用
   */
  const char& operator[](uint32_t index) const
  {
    if (index < size_impl())
      return data_impl()[index];
    else
      return data_impl()[size_impl() - 1];
  }

  /**
   * @brief  动态字符串 元素访问 通过索引访问字符（安全版本）
   *
   * @param  index  索引位置
   * @return char&  索引位置的字符引用
   */
  char& at(uint32_t index)
  {
    if (index < size_impl())
      return data_impl()[index];
    else
      return data_impl()[size_impl() - 1];
  }

  /**
   * @brief  动态字符串 元素访问 通过索引访问常量字符（安全版本）
   *
   * @param  index        索引位置
   * @return const char&  索引位置的常量字符引用
   */
  const char& at(uint32_t index) const
  {
    if (index < size_impl())
      return data_impl()[index];
    else
      return data_impl()[size_impl() - 1];
  }

  /**
   * @brief  动态字符串 元素访问 获取第一个字符
   *
   * @return char&  第一个字符的引用
   */
  char& front(void)
  {
    return data_impl()[0];
  }

  /**
   * @brief  动态字符串 元素访问 获取第一个常量字符
   *
   * @return const char&  第一个常量字符的引用
   */
  const char& front(void) const
  {
    return data_impl()[0];
  }

  /**
   * @brief  动态字符串 元素访问 获取最后一个字符
   *
   * @return char&  最后一个字符的引用
   */
  char& back(void)
  {
    return data_impl()[size_impl() - 1];
  }

  /**
   * @brief  动态字符串 元素访问 获取最后一个常量字符
   *
   * @return const char&  最后一个常量字符的引用
   */
  const char& back(void) const
  {
    return data_impl()[size_impl() - 1];
  }

  /**
   * @brief  动态字符串 容量 获取字符串大小
   *
   * @return uint32_t  字符串大小
   */
  uint32_t size(void) const
  {
    return size_impl();
  }

  /**
   * @brief  动态字符串 容量 获取字符串长度
   *
   * @return uint32_t  字符串长度
   */
  uint32_t length(void) const
  {
    return size_impl();
  }

  /**
   * @brief  动态字符串 容量 获取字符串容量
   *
   * @return uint32_t  字符串容量
   */
  uint32_t capacity(void) const
  {
    return capacity_impl();
  }

  /**
   * @brief  动态字符串 容量 判断字符串是否为空
   *
   * @return true   字符串为空
   * @return false  字符串不为空
   */
  bool empty(void) const
  {
    return size_impl() == 0;
  }

  /**
   * @brief  动态字符串 修改器 清空字符串
   */
  void clear(void)
  {
    clear_impl();
  }

  /**
   * @brief  动态字符串 修改器 调整字符串大小
   *
   * @param  new_size  新的大小
   */
  void resize(uint32_t new_size)
  {
    resize_impl(new_size);
  }

  /**
   * @brief  动态字符串 修改器 调整字符串大小并用指定字符填充
   *
   * @param  new_size  新的大小
   * @param  ch        填充字符
   */
  void resize(uint32_t new_size, char ch)
  {
    resize_impl(new_size, ch);
  }

  /**
   * @brief  动态字符串 修改器 预留字符串容量
   *
   * @param  new_capacity  新的容量
   */
  void reserve(uint32_t new_capacity)
  {
    reserve_impl(new_capacity);
  }

  /**
   * @brief  动态字符串 修改器 在末尾添加字符
   *
   * @param  ch  要添加的字符
   */
  void push_back(char ch)
  {
    append_impl(ch, 1);
  }

  /**
   * @brief  动态字符串 修改器 删除末尾字符
   */
  void pop_back(void)
  {
    resize_impl(size_impl() - 1);
  }

  /**
   * @brief  动态字符串 修改器 追加QString对象
   *
   * @param  other      要追加的QString对象
   * @return QString&   当前对象引用
   */
  QString& append(const QString& other)
  {
    append_impl(other.data_impl(), other.size_impl());
    return *this;
  }

  /**
   * @brief  动态字符串 修改器 追加C字符串
   *
   * @param  str        要追加的C字符串
   * @return QString&   当前对象引用
   */
  QString& append(const char* str)
  {
    append_impl(str, strlen(str));
    return *this;
  }

  /**
   * @brief  动态字符串 修改器 追加指定长度的字符串
   *
   * @param  str        要追加的字符串
   * @param  len        字符串长度
   * @return QString&   当前对象引用
   */
  QString& append(const char* str, uint32_t len)
  {
    append_impl(str, len);
    return *this;
  }

  /**
   * @brief  动态字符串 修改器 追加字符
   *
   * @param  ch         要追加的字符
   * @param  count      字符重复次数，默认为1
   * @return QString&   当前对象引用
   */
  QString& append(char ch, uint32_t count = 1)
  {
    append_impl(ch, count);
    return *this;
  }

  /**
   * @brief  动态字符串 修改器 追加整数
   *
   * @param  value      要追加的整数值
   * @param  base       进制基数，默认为10
   * @param  uppercase  是否使用大写字母，默认为false
   * @param  add_prefix 是否添加前缀，默认为false
   * @return QString&   当前对象引用
   */
  template <typename T, typename = std::enable_if_t<std::is_integral<T>::value && !std::is_same<T, char>::value>>
  QString& append(T value, int base = 10, bool uppercase = false, bool add_prefix = false)
  {
    char buffer[64];
    int  result = system::algorithm::Format::format(buffer, sizeof(buffer), value, base, uppercase, add_prefix);
    if (result > 0)
    {
      append_impl(buffer, result);
    }
    return *this;
  }

  /**
   * @brief  动态字符串 修改器 追加浮点数
   *
   * @param  value      要追加的浮点数值
   * @param  precision  精度，默认为6
   * @return QString&   当前对象引用
   */
  template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::value && !std::is_same<T, char>::value>>
  QString& append(T value, int precision = 6)
  {
    char buffer[64];
    int  result = system::algorithm::Format::format(buffer, sizeof(buffer), value, precision);
    if (result > 0)
    {
      append_impl(buffer, result);
    }
    return *this;
  }

  /**
   * @brief  动态字符串 修改器 追加QString对象（+=运算符）
   *
   * @param  other      要追加的QString对象
   * @return QString&   当前对象引用
   */
  QString& operator+=(const QString& other)
  {
    return append(other);
  }

  /**
   * @brief  动态字符串 修改器 追加C字符串（+=运算符）
   *
   * @param  str        要追加的C字符串
   * @return QString&   当前对象引用
   */
  QString& operator+=(const char* str)
  {
    return append(str);
  }

  /**
   * @brief  动态字符串 修改器 追加字符（+=运算符）
   *
   * @param  ch         要追加的字符
   * @return QString&   当前对象引用
   */
  QString& operator+=(char ch)
  {
    return append(ch);
  }

  /**
   * @brief  动态字符串 修改器 追加整数或浮点数（+=运算符）
   *
   * @param  value      要追加的整数值
   * @return QString&   当前对象引用
   */
  template <typename T, typename = std::enable_if_t<!std::is_same<T, char>::value>>
  QString& operator+=(T value)
  {
    return append(value);
  }

  /**
   * @brief  动态字符串 修改器 在指定位置插入QString对象
   *
   * @param  index      插入位置
   * @param  other      要插入的QString对象
   * @return QString&   当前对象引用
   */
  QString& insert(uint32_t index, const QString& other)
  {
    insert_impl(index, other.data_impl(), other.size_impl());
    return *this;
  }

  /**
   * @brief  动态字符串 修改器 在指定位置插入C字符串
   *
   * @param  index      插入位置
   * @param  str        要插入的C字符串
   * @return QString&   当前对象引用
   */
  QString& insert(uint32_t index, const char* str)
  {
    insert_impl(index, str, strlen(str));
    return *this;
  }

  /**
   * @brief  动态字符串 修改器 在指定位置插入指定长度的字符串
   *
   * @param  index      插入位置
   * @param  str        要插入的字符串
   * @param  len        字符串长度
   * @return QString&   当前对象引用
   */
  QString& insert(uint32_t index, const char* str, uint32_t len)
  {
    insert_impl(index, str, len);
    return *this;
  }

  /**
   * @brief  动态字符串 修改器 在指定位置插入字符
   *
   * @param  index      插入位置
   * @param  ch         要插入的字符
   * @param  count      字符重复次数，默认为1
   * @return QString&   当前对象引用
   */
  QString& insert(uint32_t index, char ch, uint32_t count = 1)
  {
    insert_impl(index, ch, count);
    return *this;
  }

  /**
   * @brief  动态字符串 修改器 在指定位置插入整数
   *
   * @param  index      插入位置
   * @param  value      要插入的整数值
   * @param  base       进制基数，默认为10
   * @param  uppercase  是否大写，默认为false
   * @param  add_prefix 是否添加前缀，默认为false
   * @return QString&   当前对象引用
   */
  template <typename T, typename = std::enable_if_t<std::is_integral<T>::value && !std::is_same<T, char>::value>>
  QString& insert(uint32_t index, T value, int base = 10, bool uppercase = false, bool add_prefix = false)
  {
    char buffer[64];
    int  result = system::algorithm::Format::format(buffer, sizeof(buffer), value, base, uppercase, add_prefix);
    if (result > 0)
    {
      insert_impl(index, buffer, result);
    }
    return *this;
  }

  /**
   * @brief  动态字符串 修改器 在指定位置插入浮点数
   *
   * @param  index      插入位置
   * @param  value      要插入的浮点数值
   * @param  precision  精度，默认为6
   * @return QString&   当前对象引用
   */
  template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::valu && !std::is_same<T, char>::value>>
  QString& insert(uint32_t index, T value, int precision = 6)
  {
    char buffer[64];
    int  result = system::algorithm::Format::format(buffer, sizeof(buffer), value, precision);
    if (result > 0)
    {
      insert_impl(index, buffer, result);
    }
    return *this;
  }

  /**
   * @brief  动态字符串 修改器 删除指定位置的字符
   *
   * @param  index      删除起始位置，默认为0
   * @param  count      删除字符数量，默认为UINT32_MAX
   * @return QString&   当前对象引用
   */
  QString& erase(uint32_t index = 0, uint32_t count = UINT32_MAX)
  {
    erase_impl(index, count);
    return *this;
  }

  /**
   * @brief  动态字符串 修改器 替换指定位置的子串为QString对象
   *
   * @param  index      替换起始位置
   * @param  count      被替换字符数量
   * @param  other      替换的QString对象
   * @return QString&   当前对象引用
   */
  QString& replace(uint32_t index, uint32_t count, const QString& other)
  {
    replace_impl(index, count, other.data_impl(), other.size());
    return *this;
  }

  /**
   * @brief  动态字符串 修改器 替换指定位置的子串为C字符串
   *
   * @param  index      替换起始位置
   * @param  count      被替换字符数量
   * @param  str        替换的C字符串
   * @return QString&   当前对象引用
   */
  QString& replace(uint32_t index, uint32_t count, const char* str)
  {
    replace_impl(index, count, str, strlen(str));
    return *this;
  }

  /**
   * @brief  动态字符串 修改器 替换指定位置的子串为指定长度的字符串
   *
   * @param  index      替换起始位置
   * @param  count      被替换字符数量
   * @param  str        替换的字符串
   * @param  length     字符串长度
   * @return QString&   当前对象引用
   */
  QString& replace(uint32_t index, uint32_t count, const char* str, uint32_t length)
  {
    replace_impl(index, count, str, length);
    return *this;
  }

  /**
   * @brief  动态字符串 字符串操作 获取C风格字符串
   *
   * @return const char*  C风格字符串指针
   */
  const char* c_str() const noexcept
  {
    return data_impl();
  }

  /**
   * @brief  动态字符串 字符串操作 获取数据指针（常量版本）
   *
   * @return const char*  数据指针
   */
  const char* data() const noexcept
  {
    return data_impl();
  }

  /**
   * @brief  动态字符串 字符串操作 获取数据指针
   *
   * @return char*  数据指针
   */
  char* data()
  {
    return data_impl();
  }

  /**
   * @brief  动态字符串 字符串操作 获取子字符串
   *
   * @param  pos      起始位置，默认为0
   * @param  count    子字符串长度，默认为UINT32_MAX
   * @return QString  子字符串
   */
  QString substr(uint32_t pos = 0, uint32_t count = UINT32_MAX) const
  {
    if (size_impl() <= pos)
    {
      return QString();
    }

    count = (count == UINT32_MAX) ? (size_impl() - pos) : std::min(count, size_impl() - pos);
    return QString(data_impl() + pos, count);
  }

  /**
   * @brief  动态字符串 查找操作 正向查找QString对象
   *
   * @param  other      要查找的QString对象
   * @param  pos        查找起始位置，默认为0
   * @return uint32_t   找到的位置，未找到返回UINT32_MAX
   */
  uint32_t find(const QString& other, uint32_t pos = 0) const
  {
    return find(other.data_impl(), pos, other.size_impl());
  }

  /**
   * @brief  动态字符串 查找操作 正向查找指定长度的字符串
   *
   * @param  str        要查找的字符串
   * @param  pos        查找起始位置
   * @param  count      字符串长度
   * @return uint32_t   找到的位置，未找到返回UINT32_MAX
   */
  uint32_t find(const char* str, uint32_t pos, uint32_t count) const
  {
    if (nullptr == str || 0 == count || size_impl() <= pos)
    {
      return UINT32_MAX;
    }

    const char* data_ptr = data_impl();
    const char* found    = bmh_find(data_ptr + pos, size_impl() - pos, str, count);

    return found ? static_cast<uint32_t>(found - data_ptr) : UINT32_MAX;
  }

  /**
   * @brief  动态字符串 查找操作 正向查找C字符串
   *
   * @param  str        要查找的C字符串
   * @param  pos        查找起始位置，默认为0
   * @return uint32_t   找到的位置，未找到返回UINT32_MAX
   */
  uint32_t find(const char* str, uint32_t pos = 0) const
  {
    if (nullptr != str && pos < size_impl())
    {
      uint32_t len = 0;

      while (str[len] != '\0')
      {
        ++len;
      }

      if (0 < len)
      {
        const char* data_ptr = data_impl();
        const char* found    = bmh_find(data_ptr + pos, size_impl() - pos, str, len);

        return found ? static_cast<uint32_t>(found - data_ptr) : UINT32_MAX;
      }
    }

    return UINT32_MAX;
  }

  /**
   * @brief  动态字符串 查找操作 正向查找字符
   *
   * @param  ch         要查找的字符
   * @param  pos        查找起始位置，默认为0
   * @return uint32_t   找到的位置，未找到返回UINT32_MAX
   */
  uint32_t find(char ch, uint32_t pos = 0) const
  {
    if (size_impl() <= pos)
    {
      return UINT32_MAX;
    }

    const char* data_ptr = data_impl();
    const char* found    = static_cast<const char*>(memchr(data_ptr + pos, ch, size_impl() - pos));

    return found ? static_cast<uint32_t>(found - data_ptr) : UINT32_MAX;
  }

  /**
   * @brief  动态字符串 查找操作 反向查找QString对象
   *
   * @param  other      要查找的QString对象
   * @param  pos        查找起始位置，默认为UINT32_MAX
   * @return uint32_t   找到的位置，未找到返回UINT32_MAX
   */
  uint32_t rfind(const QString& other, uint32_t pos = UINT32_MAX) const
  {
    return rfind(other.data_impl(), pos, other.size_impl());
  }

  /**
   * @brief  动态字符串 查找操作 反向查找指定长度的字符串
   *
   * @param  str        要查找的字符串
   * @param  pos        查找起始位置
   * @param  count      字符串长度
   * @return uint32_t   找到的位置，未找到返回UINT32_MAX
   */
  uint32_t rfind(const char* str, uint32_t pos, uint32_t count) const
  {
    if (0 == count)
    {
      return std::min(pos, size_impl());
    }

    pos = std::min(pos, size_impl());

    if (nullptr == str || pos < count)
    {
      return UINT32_MAX;
    }

    const char* data_ptr = data_impl();
    const char* found    = bmh_rfind(data_ptr, pos, str, count);

    return found ? static_cast<uint32_t>(found - data_ptr) : UINT32_MAX;
  }

  /**
   * @brief  动态字符串 查找操作 反向查找C字符串
   *
   * @param  str        要查找的C字符串
   * @param  pos        查找起始位置，默认为UINT32_MAX
   * @return uint32_t   找到的位置，未找到返回UINT32_MAX
   */
  uint32_t rfind(const char* str, uint32_t pos = UINT32_MAX) const
  {
    if (str != nullptr && (pos < size_impl() || pos == UINT32_MAX))
    {
      uint32_t len = 0;

      while (str[len] != '\0')
      {
        ++len;
      }

      if (0 < len)
      {
        pos                  = std::min(pos, size_impl());

        const char* data_ptr = data_impl();
        const char* found    = bmh_rfind(data_ptr, pos, str, len);

        return found ? static_cast<uint32_t>(found - data_ptr) : UINT32_MAX;
      }
    }

    return UINT32_MAX;
  }

  /**
   * @brief  动态字符串 查找操作 反向查找字符
   *
   * @param  ch         要查找的字符
   * @param  pos        查找起始位置，默认为UINT32_MAX
   * @return uint32_t   找到的位置，未找到返回UINT32_MAX
   */
  uint32_t rfind(char ch, uint32_t pos = UINT32_MAX) const
  {
    pos = std::min(pos, size_impl());

    if (pos == 0)
    {
      return UINT32_MAX;
    }

    const char* data_ptr   = data_impl();
    const char* start_ptr  = data_ptr;
    const char* search_ptr = data_ptr + pos;

    while (search_ptr > start_ptr)
    {
      --search_ptr;
      if (*search_ptr == ch)
      {
        return static_cast<uint32_t>(search_ptr - data_ptr);
      }
    }

    if (pos > 0 && data_ptr[0] == ch)
    {
      return 0;
    }

    return UINT32_MAX;
  }

  /**
   * @brief  动态字符串 比较操作 与QString对象比较
   *
   * @param  other  要比较的QString对象
   * @return int    比较结果：小于0表示小于，等于0表示相等，大于0表示大于
   */
  int compare(const QString& other) const noexcept
  {
    return compare(0, size_impl(), other.data_impl(), other.size_impl());
  }

  /**
   * @brief  动态字符串 比较操作 与C字符串比较
   *
   * @param  str  要比较的C字符串
   * @return int  比较结果：小于0表示小于，等于0表示相等，大于0表示大于
   */
  int compare(const char* str) const noexcept
  {
    if (str != nullptr)
    {
      uint32_t len = 0;

      while (str[len] != '\0')
      {
        ++len;
      }

      return compare(0, size_impl(), str, len);
    }
    else
    {
      return -1;
    }
  }

  /**
   * @brief  动态字符串 比较操作 与QString对象比较指定范围
   *
   * @param  pos    比较起始位置
   * @param  count  比较字符数量
   * @param  other  要比较的QString对象
   * @return int    比较结果：小于0表示小于，等于0表示相等，大于0表示大于
   */
  int compare(uint32_t pos, uint32_t count, const QString& other) const
  {
    return compare(pos, count, other.data_impl(), other.size_impl());
  }

  /**
   * @brief  动态字符串 比较操作 与C字符串比较指定范围
   *
   * @param  pos    比较起始位置
   * @param  count  比较字符数量
   * @param  str    要比较的C字符串
   * @return int    比较结果：小于0表示小于，等于0表示相等，大于0表示大于
   */
  int compare(uint32_t pos, uint32_t count, const char* str) const
  {
    if (str != nullptr)
    {
      uint32_t len = 0;

      while (str[len] != '\0')
      {
        ++len;
      }

      return compare(pos, count, str, len);
    }
    else
    {
      return -1;
    }
  }

  /**
   * @brief  动态字符串 比较操作 与指定长度字符串比较指定范围
   *
   * @param  pos     比较起始位置
   * @param  count1  当前字符串比较字符数量
   * @param  str     要比较的字符串
   * @param  count2  比较字符串字符数量
   * @return int     比较结果：小于0表示小于，等于0表示相等，大于0表示大于
   */
  int compare(uint32_t pos, uint32_t count1, const char* str, uint32_t count2) const
  {
    if (str != nullptr)
    {
      uint32_t len = std::min(count1, count2);

      int result   = memcmp(data_impl() + pos, str, len);

      if (0 != result)
      {
        return result;
      }
      else if (count1 < count2)
      {
        return -1;
      }
      else if (count1 > count2)
      {
        return 1;
      }
      else
      {
        return 0;
      }
    }
    else
    {
      return -1;
    }
  }

  /**
   * @brief  动态字符串 比较操作 等于比较（QString对象）
   *
   * @param  other  要比较的QString对象
   * @return bool   比较结果
   */
  bool operator==(const QString& other) const noexcept
  {
    return compare(other) == 0;
  }

  /**
   * @brief  动态字符串 比较操作 不等于比较（QString对象）
   *
   * @param  other  要比较的QString对象
   * @return bool   比较结果
   */
  bool operator!=(const QString& other) const noexcept
  {
    return compare(other) != 0;
  }

  /**
   * @brief  动态字符串 比较操作 小于比较（QString对象）
   *
   * @param  other  要比较的QString对象
   * @return bool   比较结果
   */
  bool operator<(const QString& other) const noexcept
  {
    return compare(other) < 0;
  }

  /**
   * @brief  动态字符串 比较操作 大于比较（QString对象）
   *
   * @param  other  要比较的QString对象
   * @return bool   比较结果
   */
  bool operator>(const QString& other) const noexcept
  {
    return compare(other) > 0;
  }

  /**
   * @brief  动态字符串 比较操作 小于等于比较（QString对象）
   *
   * @param  other  要比较的QString对象
   * @return bool   比较结果
   */
  bool operator<=(const QString& other) const noexcept
  {
    return compare(other) <= 0;
  }

  /**
   * @brief  动态字符串 比较操作 大于等于比较（QString对象）
   *
   * @param  other  要比较的QString对象
   * @return bool   比较结果
   */
  bool operator>=(const QString& other) const noexcept
  {
    return compare(other) >= 0;
  }

  /**
   * @brief  动态字符串 比较操作 等于比较（C字符串）
   *
   * @param  str   要比较的C字符串
   * @return bool  比较结果
   */
  bool operator==(const char* str) const noexcept
  {
    return compare(str) == 0;
  }

  /**
   * @brief  动态字符串 比较操作 不等于比较（C字符串）
   *
   * @param  str   要比较的C字符串
   * @return bool  比较结果
   */
  bool operator!=(const char* str) const noexcept
  {
    return compare(str) != 0;
  }

  /**
   * @brief  动态字符串 比较操作 小于比较（C字符串）
   *
   * @param  str   要比较的C字符串
   * @return bool  比较结果
   */
  bool operator<(const char* str) const noexcept
  {
    return compare(str) < 0;
  }

  /**
   * @brief  动态字符串 比较操作 大于比较（C字符串）
   *
   * @param  str   要比较的C字符串
   * @return bool  比较结果
   */
  bool operator>(const char* str) const noexcept
  {
    return compare(str) > 0;
  }

  /**
   * @brief  动态字符串 比较操作 小于等于比较（C字符串）
   *
   * @param  str   要比较的C字符串
   * @return bool  比较结果
   */
  bool operator<=(const char* str) const noexcept
  {
    return compare(str) <= 0;
  }

  /**
   * @brief  动态字符串 比较操作 大于等于比较（C字符串）
   *
   * @param  str   要比较的C字符串
   * @return bool  比较结果
   */
  bool operator>=(const char* str) const noexcept
  {
    return compare(str) >= 0;
  }

  /**
   * @brief  动态字符串 大小写转换 转换为小写
   *
   * @return QString&  当前对象引用
   */
  QString& to_lower(void) noexcept
  {
    char* ptr = data_impl();

    for (uint32_t i = 0; i < size_impl(); ++i)
    {
      ptr[i] = tolower(ptr[i]);
    }

    return *this;
  }

  /**
   * @brief  动态字符串 大小写转换 转换为大写
   *
   * @return QString&  当前对象引用
   */
  QString& to_upper(void) noexcept
  {
    char* ptr = data_impl();

    for (uint32_t i = 0; i < size_impl(); ++i)
    {
      ptr[i] = toupper(ptr[i]);
    }

    return *this;
  }

  /**
   * @brief  动态字符串 大小写转换 获取小写副本
   *
   * @return QString  小写字符串副本
   */
  QString to_lower_copy(void) const
  {
    QString str(*this);
    str.to_lower();
    return str;
  }

  /**
   * @brief  动态字符串 大小写转换 获取大写副本
   *
   * @return QString  大写字符串副本
   */
  QString to_upper_copy(void) const
  {
    QString str(*this);
    str.to_upper();
    return str;
  }

  /**
   * @brief  动态字符串 前缀后缀检查 检查是否以QString对象开头
   *
   * @param  other  要检查的QString对象
   * @return bool   检查结果
   */
  bool starts_with(const QString& other) const noexcept
  {
    if (other.size_impl() > size_impl())
    {
      return false;
    }

    return memcmp(data_impl(), other.data_impl(), other.size_impl()) == 0;
  }

  /**
   * @brief  动态字符串 前缀后缀检查 检查是否以C字符串开头
   *
   * @param  str  要检查的C字符串
   * @return bool 检查结果
   */
  bool starts_with(const char* str) const noexcept
  {
    if (nullptr != str)
    {
      uint32_t len = 0;

      while (str[len] != '\0')
      {
        ++len;
      }

      if (size_impl() < len)
      {
        return false;
      }

      return memcmp(data_impl(), str, len) == 0;
    }
    else
    {
      return false;
    }
  }

  /**
   * @brief  动态字符串 前缀后缀检查 检查是否以指定长度字符串开头
   *
   * @param  str  要检查的字符串
   * @param  len  字符串长度
   * @return bool 检查结果
   */
  bool starts_with(const char* str, uint32_t len) const noexcept
  {
    if (nullptr == str || 0 == len || size_impl() < len)
    {
      return false;
    }

    return memcmp(data_impl(), str, len) == 0;
  }

  /**
   * @brief  动态字符串 前缀后缀检查 检查是否以字符开头
   *
   * @param  ch   要检查的字符
   * @return bool 检查结果
   */
  bool starts_with(char ch) const noexcept
  {
    return ((0 != size_impl()) && (data_impl()[0] == ch));
  }

  /**
   * @brief  动态字符串 前缀后缀检查 检查是否以QString对象结尾
   *
   * @param  other  要检查的QString对象
   * @return bool   检查结果
   */
  bool ends_with(const QString& other) const noexcept
  {
    if (other.size_impl() > size_impl())
    {
      return false;
    }

    return memcmp(data_impl() + size_impl() - other.size_impl(), other.data_impl(), other.size_impl()) == 0;
  }

  /**
   * @brief  动态字符串 前缀后缀检查 检查是否以C字符串结尾
   *
   * @param  str  要检查的C字符串
   * @return bool 检查结果
   */
  bool ends_with(const char* str) const noexcept
  {
    if (nullptr != str)
    {
      uint32_t len = 0;

      while (str[len] != '\0')
      {
        ++len;
      }

      if (size_impl() < len)
      {
        return false;
      }

      return memcmp(data_impl() + size_impl() - len, str, len) == 0;
    }
    else
    {
      return false;
    }
  }

  /**
   * @brief  动态字符串 前缀后缀检查 检查是否以指定长度字符串结尾
   *
   * @param  str  要检查的字符串
   * @param  len  字符串长度
   * @return bool 检查结果
   */
  bool ends_with(const char* str, uint32_t len) const noexcept
  {
    if (nullptr == str || 0 == len || size_impl() < len)
    {
      return false;
    }

    return memcmp(data_impl() + size_impl() - len, str, len) == 0;
  }

  /**
   * @brief  动态字符串 前缀后缀检查 检查是否以字符结尾
   *
   * @param  ch   要检查的字符
   * @return bool 检查结果
   */
  bool ends_with(char ch) const noexcept
  {
    return ((0 != size_impl()) && (data_impl()[size_impl() - 1] == ch));
  }

  /**
   * @brief  动态字符串 统计操作 统计字符出现次数
   *
   * @param  ch         要统计的字符
   * @return uint32_t   字符出现次数
   */
  uint32_t count(char ch) const noexcept
  {
    uint32_t    count = 0;
    const char* ptr   = data_impl();

    for (uint32_t i = 0; i < size_impl(); ++i)
    {
      if (ptr[i] == ch)
      {
        ++count;
      }
    }

    return count;
  }

  /**
   * @brief  动态字符串 统计操作 统计QString对象出现次数
   *
   * @param  other      要统计的QString对象
   * @return uint32_t   QString对象出现次数
   */
  uint32_t count(const QString& other) const
  {
    if (other.empty() || (other.size_impl() > size_impl()))
    {
      return 0;
    }

    uint32_t    count     = 0;
    const char* ptr       = data_impl();
    const char* other_ptr = other.data_impl();
    uint32_t    length    = other.size_impl();

    for (uint32_t i = 0; i < size_impl(); ++i)
    {
      if (0 == memcmp(ptr + i, other_ptr, length))
      {
        ++count;
        i += length - 1;
      }
    }

    return count;
  }

  /**
   * @brief  动态字符串 格式化操作 左侧填充
   *
   * @param  total_width  总宽度
   * @param  pad_char     填充字符，默认为空格
   * @return QString&     当前对象引用
   */
  QString& pad_left(uint32_t total_width, char pad_char = ' ')
  {
    uint32_t pad_count = 0;

    if (size_impl() < total_width)
    {
      pad_count = total_width - size_impl();
      resize_impl(total_width);
    }
    else
    {
      return *this;
    }

    QString padding(pad_char, pad_count);
    insert(0, padding);
    return *this;
  }

  /**
   * @brief  动态字符串 格式化操作 右侧填充
   *
   * @param  total_width  总宽度
   * @param  pad_char     填充字符，默认为空格
   * @return QString&     当前对象引用
   */
  QString& pad_right(uint32_t total_width, char pad_char = ' ')
  {
    if (size_impl() >= total_width)
    {
      return *this;
    }

    uint32_t pad_count = total_width - size_impl();
    append(pad_char, pad_count);
    return *this;
  }

  /**
   * @brief  动态字符串 格式化操作 去除首尾空白
   *
   * @return QString&  当前对象引用
   */
  QString& trim()
  {
    return trim_left().trim_right();
  }

  /**
   * @brief  动态字符串 格式化操作 去除左侧空白
   *
   * @return QString&  当前对象引用
   */
  QString& trim_left()
  {
    uint32_t start = 0;

    while (size_impl() > start && is_space(data_impl()[start]))
    {
      ++start;
    }

    if (0 < start)
    {
      erase_impl(0, start);
    }

    return *this;
  }

  /**
   * @brief  动态字符串 格式化操作 去除右侧空白
   *
   * @return QString&  当前对象引用
   */
  QString& trim_right()
  {
    uint32_t end = size_impl();

    while (0 < end && is_space(data_impl()[end - 1]))
    {
      --end;
    }

    if (size_impl() > end)
    {
      resize_impl(end);
    }

    return *this;
  }

  /**
   * @brief  动态字符串 数值转换 将数值转换为字符串
   *
   * @param  value      要转换的数值
   * @param  base       进制基数，默认为10
   * @param  uppercase  是否大写，默认为false
   * @param  add_prefix 是否添加前缀，默认为false
   * @return QString    转换后的字符串
   */
  template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
  static QString to_string(T value, int base = 10, bool uppercase = false, bool add_prefix = false)
  {
    char buffer[64];
    int  result = system::algorithm::Format::format(buffer, sizeof(buffer), value, base, uppercase, add_prefix);

    if (result > 0)
    {
      return QString(buffer, result);
    }
    else
    {
      return QString();
    }
  }

  /**
   * @brief  动态字符串 数值转换 将浮点数转换为字符串
   *
   * @param  value           要转换的浮点数
   * @param  precision       精度，默认为6
   * @param  use_scientific  是否使用科学计数法，默认为false
   * @return QString         转换后的字符串
   */
  template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
  static QString to_string(T value, int precision = 6, bool use_scientific = false)
  {
    char buffer[64];
    int  result = system::algorithm::Format::format(buffer, sizeof(buffer), value, precision, use_scientific);

    if (result > 0)
    {
      return QString(buffer, result);
    }
    else
    {
      return QString();
    }
  }

  /**
   * @brief  动态字符串 数值转换 从字符串解析数值
   *
   * @param  str     要解析的字符串
   * @param  result  解析结果
   * @param  base    进制基数，默认为10
   * @return bool    解析成功返回true，失败返回false
   */
  template <typename T, typename = std::enable_if_t<std::is_integral<T>::value && !std::is_same<T, char>::value>>
  static bool parse(const QString& str, T& result, int base = 10)
  {
    return system::algorithm::Parse::parse(str.data_impl(), str.size_impl(), result, base);
  }

  /**
   * @brief  动态字符串 数值转换 从字符串解析浮点数
   *
   * @param  str     要解析的字符串
   * @param  result  解析结果
   * @return bool    解析成功返回true，失败返回false
   */
  template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::value && !std::is_same<T, char>::value>>
  static bool parse(const QString& str, T& result)
  {
    return system::algorithm::Parse::parse(str.data_impl(), str.size_impl(), result);
  }

  /**
   * @brief  动态字符串 数值转换 转换为整数
   *
   * @param  base  进制基数，默认为10
   * @return int   转换结果
   */
  int to_int(int base = 10) const
  {
    int result = 0;
    parse(*this, result, base);
    return result;
  }

  /**
   * @brief  动态字符串 数值转换 转换为无符号整数
   *
   * @param  base           进制基数，默认为10
   * @return unsigned int   转换结果
   */
  unsigned int to_uint(int base = 10) const
  {
    unsigned int result = 0;
    parse(*this, result, base);
    return result;
  }

  /**
   * @brief  动态字符串 数值转换 转换为8位整数
   *
   * @param  base     进制基数，默认为10
   * @return int8_t   转换结果
   */
  int8_t to_int8(int base = 10) const
  {
    int8_t result = 0;
    parse(*this, result, base);
    return result;
  }

  /**
   * @brief  动态字符串 数值转换 转换为8位无符号整数
   *
   * @param  base     进制基数，默认为10
   * @return uint8_t  转换结果
   */
  uint8_t to_uint8(int base = 10) const
  {
    uint8_t result = 0;
    parse(*this, result, base);
    return result;
  }

  /**
   * @brief  动态字符串 数值转换 转换为16位整数
   *
   * @param  base     进制基数，默认为10
   * @return int16_t  转换结果
   */
  int16_t to_int16(int base = 10) const
  {
    int16_t result = 0;
    parse(*this, result, base);
    return result;
  }

  /**
   * @brief  动态字符串 数值转换 转换为16位无符号整数
   *
   * @param  base       进制基数，默认为10
   * @return uint16_t   转换结果
   */
  uint16_t to_uint16(int base = 10) const
  {
    uint16_t result = 0;
    parse(*this, result, base);
    return result;
  }

  /**
   * @brief  动态字符串 数值转换 转换为32位整数
   *
   * @param  base     进制基数，默认为10
   * @return int32_t  转换结果
   */
  int32_t to_int32(int base = 10) const
  {
    int32_t result = 0;
    parse(*this, result, base);
    return result;
  }

  /**
   * @brief  动态字符串 数值转换 转换为32位无符号整数
   *
   * @param  base       进制基数，默认为10
   * @return uint32_t   转换结果
   */
  uint32_t to_uint32(int base = 10) const
  {
    uint32_t result = 0;
    parse(*this, result, base);
    return result;
  }

  /**
   * @brief  动态字符串 数值转换 转换为64位整数
   *
   * @param  base     进制基数，默认为10
   * @return int64_t  转换结果
   */
  int64_t to_int64(int base = 10) const
  {
    int64_t result = 0;
    parse(*this, result, base);
    return result;
  }

  /**
   * @brief  动态字符串 数值转换 转换为64位无符号整数
   *
   * @param  base       进制基数，默认为10
   * @return uint64_t   转换结果
   */
  uint64_t to_uint64(int base = 10) const
  {
    uint64_t result = 0;
    parse(*this, result, base);
    return result;
  }

  /**
   * @brief  动态字符串 数值转换 转换为浮点数
   *
   * @return float  转换结果
   */
  float to_float() const
  {
    float result = 0.0f;
    parse(*this, result);
    return result;
  }

  /**
   * @brief  动态字符串 数值转换 转换为双精度浮点数
   *
   * @return double  转换结果
   */
  double to_double() const
  {
    double result = 0.0;
    parse(*this, result);
    return result;
  }

  /**
   * @brief  动态字符串 类型转换 转换为布尔值
   *
   * @return true   转换结果为真
   * @return false  转换结果为假
   */
  explicit operator bool() const noexcept
  {
    return !size_impl() == 0;
  }

  /**
   * @brief  动态字符串 字符串操作 字符串拼接（QString + QString）
   *
   * @param  lhs      左操作数
   * @param  rhs      右操作数
   * @return QString  拼接结果
   */
  friend QString operator+(const QString& lhs, const QString& rhs)
  {
    QString result(lhs);
    result.append(rhs);
    return result;
  }

  /**
   * @brief  动态字符串 字符串操作 字符串拼接（QString + C字符串）
   *
   * @param  lhs      左操作数
   * @param  rhs      右操作数
   * @return QString  拼接结果
   */
  friend QString operator+(const QString& lhs, const char* rhs)
  {
    QString result(lhs);

    uint32_t len = 0;
    while (rhs[len] != '\0')
    {
      ++len;
    }

    result.append(rhs, len);
    return result;
  }

  /**
   * @brief  动态字符串 字符串操作 字符串拼接（C字符串 + QString）
   *
   * @param  lhs      左操作数
   * @param  rhs      右操作数
   * @return QString  拼接结果
   */
  friend QString operator+(const char* lhs, const QString& rhs)
  {
    uint32_t len = 0;
    while (lhs[len] != '\0')
    {
      ++len;
    }

    QString result(lhs, len);
    result.append(rhs);
    return result;
  }

  /**
   * @brief  动态字符串 字符串操作 字符串拼接（QString + 字符）
   *
   * @param  lhs      左操作数
   * @param  rhs      右操作数
   * @return QString  拼接结果
   */
  friend QString operator+(const QString& lhs, char rhs)
  {
    QString result(lhs);
    result.push_back(rhs);
    return result;
  }

  /**
   * @brief  动态字符串 字符串操作 字符串拼接（字符 + QString）
   *
   * @param  lhs      左操作数
   * @param  rhs      右操作数
   * @return QString  拼接结果
   */
  friend QString operator+(char lhs, const QString& rhs)
  {
    QString result(lhs, 1u);
    result.append(rhs);
    return result;
  }

  /**
   * @brief  动态字符串 字符串操作 字符串拼接（QString + 整数或浮点数）
   *
   * @param  lhs      左操作数
   * @param  rhs      右操作数
   * @return QString  拼接结果
   */
  template <typename T, typename = std::enable_if_t<!std::is_same<T, char>::value>>
  friend QString operator+(const QString& lhs, T rhs)
  {
    QString result(lhs);
    result.append(rhs);
    return result;
  }

  /**
   * @brief  动态字符串 字符串操作 字符串拼接（整数 + QString或浮点数）
   *
   * @param  lhs      左操作数
   * @param  rhs      右操作数
   * @return QString  拼接结果
   */
  template <typename T, typename = std::enable_if_t<!std::is_same<T, char>::value>>
  friend QString operator+(T lhs, const QString& rhs)
  {
    QString result;
    result.append(lhs);
    result.append(rhs);
    return result;
  }

  /**
   * @brief  动态字符串 拼接运算符 字符串左移拼接（相当于+=）
   *
   * @param  other      要拼接的QString对象
   * @return QString&   当前对象引用
   */
  QString& operator<<(const QString& other)
  {
    return append(other);
  }

  /**
   * @brief  动态字符串 拼接运算符 字符串左移拼接（相当于+=）
   *
   * @param  str        要拼接的C字符串
   * @return QString&   当前对象引用
   */
  QString& operator<<(const char* str)
  {
    return append(str);
  }

  /**
   * @brief  动态字符串 拼接运算符 字符串左移拼接（相当于+=）
   *
   * @param  ch         要拼接的字符
   * @return QString&   当前对象引用
   */
  QString& operator<<(char ch)
  {
    return append(ch);
  }

  /**
   * @brief  动态字符串 拼接运算符 字符串左移拼接（相当于+=）
   *
   * @param  value      要拼接的整数或浮点数
   * @return QString&   当前对象引用
   */
  template <typename T, typename = std::enable_if_t<!std::is_same<T, char>::value>>
  QString& operator<<(T value)
  {
    return append(value);
  }

  /**
   * @brief  动态字符串 解析运算符 字符串右移（用于字符串解析）
   *
   * @param  value      要解析的变量引用
   * @return QString&   当前对象引用
   */
  template <typename T>
  QString& operator>>(T& value)
  {
    parse(*this, value);
    return *this;
  }

  /**
   * @brief  动态字符串 取地址运算符 获取数据指针
   *
   * @return char*  数据指针
   */
  char* operator&() noexcept
  {
    return data_impl();
  }

  /**
   * @brief  动态字符串 取地址运算符 获取常量数据指针
   *
   * @return const char*  常量数据指针
   */
  const char* operator&() const noexcept
  {
    return data_impl();
  }

  /**
   * @brief  动态字符串 字符串操作 交换两个QString对象
   *
   * @param  first   第一个QString对象
   * @param  second  第二个QString对象
   */
  friend void swap(QString& first, QString& second) noexcept
  {
    swap(static_cast<Base&>(first), static_cast<Base&>(second));
  }
};
} /* namespace container */
} /* namespace QAQ */

#endif /* __QSTRING_HPP__ */
