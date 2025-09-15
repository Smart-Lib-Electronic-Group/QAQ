#ifndef __FORMAT_HPP__
#define __FORMAT_HPP__

#include "system_include.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 系统
namespace system
{
/// @brief 命名空间 算法函数
namespace algorithm
{
/// @brief 字符格式化
class Format
{
public:
  /// @brief 字符格式化 错误码
  class Error_Code
  {
  public:
    static constexpr int Success          = 0;  /* 成功 */
    static constexpr int Buffer_Too_Small = -1; /* 缓冲区太小 */
    static constexpr int Invalid_Base     = -2; /* 无效的进制 */
  };

private:
  /// @brief 字符格式化 字符表
  static inline constexpr char digits[]            = "0123456789abcdefghijklmnopqrstuvwxyz";

  /// @brief 字符格式化 10的幂次表(正数)
  static inline constexpr double pow_10_positive[] = { 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29, 1e30, 1e31, 1e32, 1e33, 1e34, 1e35, 1e36, 1e37, 1e38, 1e39, 1e40, 1e41, 1e42, 1e43, 1e44, 1e45, 1e46, 1e47, 1e48, 1e49, 1e50, 1e51, 1e52, 1e53, 1e54, 1e55, 1e56, 1e57, 1e58, 1e59, 1e60, 1e61, 1e62, 1e63, 1e64, 1e65, 1e66, 1e67, 1e68, 1e69, 1e70, 1e71, 1e72, 1e73, 1e74, 1e75, 1e76, 1e77, 1e78, 1e79, 1e80, 1e81, 1e82, 1e83, 1e84, 1e85, 1e86, 1e87, 1e88, 1e89, 1e90, 1e91, 1e92, 1e93, 1e94, 1e95, 1e96, 1e97, 1e98, 1e99, 1e100, 1e101, 1e102, 1e103, 1e104, 1e105, 1e106, 1e107, 1e108, 1e109, 1e110, 1e111, 1e112, 1e113, 1e114, 1e115, 1e116, 1e117, 1e118, 1e119, 1e120, 1e121, 1e122, 1e123, 1e124, 1e125, 1e126, 1e127, 1e128, 1e129, 1e130, 1e131, 1e132, 1e133, 1e134, 1e135, 1e136, 1e137, 1e138, 1e139, 1e140, 1e141, 1e142, 1e143, 1e144, 1e145, 1e146, 1e147, 1e148, 1e149, 1e150, 1e151, 1e152, 1e153, 1e154, 1e155, 1e156, 1e157, 1e158, 1e159, 1e160, 1e161, 1e162, 1e163, 1e164, 1e165, 1e166, 1e167, 1e168, 1e169, 1e170, 1e171, 1e172, 1e173, 1e174, 1e175, 1e176, 1e177, 1e178, 1e179, 1e180, 1e181, 1e182, 1e183, 1e184, 1e185, 1e186, 1e187, 1e188, 1e189, 1e190, 1e191, 1e192, 1e193, 1e194, 1e195, 1e196, 1e197, 1e198, 1e199, 1e200, 1e201, 1e202, 1e203, 1e204, 1e205, 1e206, 1e207, 1e208, 1e209, 1e210, 1e211, 1e212, 1e213, 1e214, 1e215, 1e216, 1e217, 1e218, 1e219, 1e220, 1e221, 1e222, 1e223, 1e224, 1e225, 1e226, 1e227, 1e228, 1e229, 1e230, 1e231, 1e232, 1e233, 1e234, 1e235, 1e236, 1e237, 1e238, 1e239, 1e240, 1e241, 1e242, 1e243, 1e244, 1e245, 1e246, 1e247, 1e248, 1e249, 1e250, 1e251, 1e252, 1e253, 1e254, 1e255, 1e256, 1e257, 1e258, 1e259, 1e260, 1e261, 1e262, 1e263, 1e264, 1e265, 1e266, 1e267, 1e268, 1e269, 1e270, 1e271, 1e272, 1e273, 1e274, 1e275, 1e276, 1e277, 1e278, 1e279, 1e280, 1e281, 1e282, 1e283, 1e284, 1e285, 1e286, 1e287, 1e288, 1e289, 1e290, 1e291, 1e292, 1e293, 1e294, 1e295, 1e296, 1e297, 1e298, 1e299, 1e300, 1e301, 1e302, 1e303, 1e304, 1e305, 1e306, 1e307, 1e308 };

  /// @brief 字符格式化 10的幂次表(负数)
  static inline constexpr double pow_10_negative[] = { 1e0, 1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7, 1e-8, 1e-9, 1e-10, 1e-11, 1e-12, 1e-13, 1e-14, 1e-15, 1e-16, 1e-17, 1e-18, 1e-19, 1e-20, 1e-21, 1e-22, 1e-23, 1e-24, 1e-25, 1e-26, 1e-27, 1e-28, 1e-29, 1e-30, 1e-31, 1e-32, 1e-33, 1e-34, 1e-35, 1e-36, 1e-37, 1e-38, 1e-39, 1e-40, 1e-41, 1e-42, 1e-43, 1e-44, 1e-45, 1e-46, 1e-47, 1e-48, 1e-49, 1e-50, 1e-51, 1e-52, 1e-53, 1e-54, 1e-55, 1e-56, 1e-57, 1e-58, 1e-59, 1e-60, 1e-61, 1e-62, 1e-63, 1e-64, 1e-65, 1e-66, 1e-67, 1e-68, 1e-69, 1e-70, 1e-71, 1e-72, 1e-73, 1e-74, 1e-75, 1e-76, 1e-77, 1e-78, 1e-79, 1e-80, 1e-81, 1e-82, 1e-83, 1e-84, 1e-85, 1e-86, 1e-87, 1e-88, 1e-89, 1e-90, 1e-91, 1e-92, 1e-93, 1e-94, 1e-95, 1e-96, 1e-97, 1e-98, 1e-99, 1e-100, 1e-101, 1e-102, 1e-103, 1e-104, 1e-105, 1e-106, 1e-107, 1e-108, 1e-109, 1e-110, 1e-111, 1e-112, 1e-113, 1e-114, 1e-115, 1e-116, 1e-117, 1e-118, 1e-119, 1e-120, 1e-121, 1e-122, 1e-123, 1e-124, 1e-125, 1e-126, 1e-127, 1e-128, 1e-129, 1e-130, 1e-131, 1e-132, 1e-133, 1e-134, 1e-135, 1e-136, 1e-137, 1e-138, 1e-139, 1e-140, 1e-141, 1e-142, 1e-143, 1e-144, 1e-145, 1e-146, 1e-147, 1e-148, 1e-149, 1e-150, 1e-151, 1e-152, 1e-153, 1e-154, 1e-155, 1e-156, 1e-157, 1e-158, 1e-159, 1e-160, 1e-161, 1e-162, 1e-163, 1e-164, 1e-165, 1e-166, 1e-167, 1e-168, 1e-169, 1e-170, 1e-171, 1e-172, 1e-173, 1e-174, 1e-175, 1e-176, 1e-177, 1e-178, 1e-179, 1e-180, 1e-181, 1e-182, 1e-183, 1e-184, 1e-185, 1e-186, 1e-187, 1e-188, 1e-189, 1e-190, 1e-191, 1e-192, 1e-193, 1e-194, 1e-195, 1e-196, 1e-197, 1e-198, 1e-199, 1e-200, 1e-201, 1e-202, 1e-203, 1e-204, 1e-205, 1e-206, 1e-207, 1e-208, 1e-209, 1e-210, 1e-211, 1e-212, 1e-213, 1e-214, 1e-215, 1e-216, 1e-217, 1e-218, 1e-219, 1e-220, 1e-221, 1e-222, 1e-223, 1e-224, 1e-225, 1e-226, 1e-227, 1e-228, 1e-229, 1e-230, 1e-231, 1e-232, 1e-233, 1e-234, 1e-235, 1e-236, 1e-237, 1e-238, 1e-239, 1e-240, 1e-241, 1e-242, 1e-243, 1e-244, 1e-245, 1e-246, 1e-247, 1e-248, 1e-249, 1e-250, 1e-251, 1e-252, 1e-253, 1e-254, 1e-255, 1e-256, 1e-257, 1e-258, 1e-259, 1e-260, 1e-261, 1e-262, 1e-263, 1e-264, 1e-265, 1e-266, 1e-267, 1e-268, 1e-269, 1e-270, 1e-271, 1e-272, 1e-273, 1e-274, 1e-275, 1e-276, 1e-277, 1e-278, 1e-279, 1e-280, 1e-281, 1e-282, 1e-283, 1e-284, 1e-285, 1e-286, 1e-287, 1e-288, 1e-289, 1e-290, 1e-291, 1e-292, 1e-293, 1e-294, 1e-295, 1e-296, 1e-297, 1e-298, 1e-299, 1e-300, 1e-301, 1e-302, 1e-303, 1e-304, 1e-305, 1e-306, 1e-307, 1e-308 };

  /// @brief 字符格式化 常用log10值表
  static inline constexpr double log_10_table[]    = { 0.0, 0.0, 0.301029995663981, 0.477121254719662, 0.602059991327962, 0.698970004336019, 0.778151250383644, 0.845098040014257, 0.903089986991944, 0.954242509439325, 1.0, 1.041392685158225, 1.079181246047625, 1.113943352306837, 1.146128035678238, 1.176091259055681 };

  /**
   * @brief  字符格式化 快速pow10函数
   *
   * @param  exp     指数
   * @return double  10的exp次幂
   */
  static QAQ_INLINE double QAQ_O3 fast_pow_10(int exp)
  {
    if (exp >= 0 && exp < static_cast<int>(sizeof(pow_10_positive) / sizeof(pow_10_positive[0])))
    {
      return pow_10_positive[exp];
    }
    if (exp < 0 && -exp < static_cast<int>(sizeof(pow_10_negative) / sizeof(pow_10_negative[0])))
    {
      return pow_10_negative[-exp];
    }
    else
    {
      return std::pow(10.0, static_cast<double>(exp));
    }
  }

  /**
   * @brief  字符格式化 快速log10函数
   *
   * @param  value   值
   * @return double  log10(value)
   */
  static QAQ_INLINE double QAQ_O3 fast_log_10(double value)
  {
    if (value >= 1.0 && value <= 15.0 && value == (int)value)
    {
      return log_10_table[(int)value];
    }
    else
    {
      return log10(value);   // 不常用值回退到标准函数
    }
  }

  /**
   * @brief  字符格式化 二进制格式化函数
   *
   * @tparam T        类型
   * @param  buffer   缓冲区
   * @param  value    值
   * @param  buf_size 缓冲区大小
   * @return int      格式化后的长度
   */
  template <typename T>
  static QAQ_INLINE int QAQ_O3 format_as_bin(char* buffer, T value, uint32_t buf_size)
  {
    static_assert(std::is_integral<T>::value, "T must be integral type");

    if (buf_size == 0)
    {
      return Error_Code::Buffer_Too_Small;
    }

    // 特殊情况：0
    if (value == 0)
    {
      if (buf_size < 2)
      {
        return Error_Code::Buffer_Too_Small;
      }

      buffer[0] = '0';
      buffer[1] = '\0';
      return 1;
    }

    // 直接在目标缓冲区中构建（从右到左）
    char* ptr = buffer + buf_size - 1;
    *ptr      = '\0';
    ptr--;

    int                                  len    = 0;
    typename std::make_unsigned<T>::type uvalue = static_cast<typename std::make_unsigned<T>::type>(value);

    // 使用位运算快速转换
    while (uvalue > 0 && ptr >= buffer)
    {
      *ptr     = '0' + (uvalue & 1);
      uvalue >>= 1;
      ptr--;
      len++;
    }

    // 移动到缓冲区开始
    ptr++;
    if (ptr > buffer)
    {
      memmove(buffer, ptr, len + 1);
    }

    return len;
  }

  /**
   * @brief  字符格式化 八进制格式化函数
   *
   * @tparam T          类型
   * @param  buffer     缓冲区
   * @param  value      值
   * @param  buf_size   缓冲区大小
   * @param  add_prefix 是否添加前缀
   * @return int        格式化后的长度
   */
  template <typename T>
  static QAQ_INLINE int QAQ_O3 format_as_oct(char* buffer, T value, uint32_t buf_size, bool add_prefix = false)
  {
    static_assert(std::is_integral<T>::value, "T must be integral type");

    if (buf_size == 0)
    {
      return Error_Code::Buffer_Too_Small;
    }

    // 特殊情况：0
    if (value == 0)
    {
      if (buf_size < 2)
      {
        return Error_Code::Buffer_Too_Small;
      }

      buffer[0] = '0';
      buffer[1] = '\0';
      return 1;
    }

    // 直接在目标缓冲区中构建
    char* ptr = buffer + buf_size - 1;
    *ptr      = '\0';
    ptr--;

    int                                  len    = 0;
    typename std::make_unsigned<T>::type uvalue = static_cast<typename std::make_unsigned<T>::type>(value);

    // 使用位运算快速转换
    while (uvalue > 0 && ptr >= buffer)
    {
      *ptr     = '0' + (uvalue & 7);
      uvalue >>= 3;
      ptr--;
      len++;
    }

    // 添加前缀
    if (add_prefix && (ptr - buffer) >= 1)
    {
      *ptr = '0';
      ptr--;
      len++;
    }
    else if (add_prefix)
    {
      return Error_Code::Buffer_Too_Small;   // 缓冲区不足
    }

    // 移动到缓冲区开始
    ptr++;
    if (ptr > buffer)
    {
      memmove(buffer, ptr, len + 1);
    }

    return len;
  }

  /**
   * @brief  字符格式化 十进制格式化函数
   *
   * @tparam T          类型
   * @tparam is_signed  是否有符号
   * @param  buffer     缓冲区
   * @param  value      值
   * @param  buf_size   缓冲区大小
   * @return int        格式化后的长度
   */
  template <typename T, bool is_signed = std::is_integral<T>::value>
  static QAQ_INLINE int QAQ_O3 format_as_dec(char* buffer, T value, uint32_t buf_size)
  {
    if constexpr (is_signed)
    {
      static_assert(std::is_integral<T>::value, "T must be integral type");
    }
    else
    {
      static_assert(std::is_unsigned<T>::value, "T must be unsigned type");
    }

    if (buf_size == 0)
    {
      return Error_Code::Buffer_Too_Small;
    }

    // 特殊情况：0
    if (value == 0)
    {
      if (buf_size < 2)
      {
        return Error_Code::Buffer_Too_Small;
      }

      buffer[0] = '0';
      buffer[1] = '\0';
      return 1;
    }

    // 在缓冲区末尾构建数字（逆序）
    char* end = buffer + buf_size - 1;
    *end      = '\0';
    end--;

    int len = 0;

    if constexpr (is_signed)
    {
      // 处理符号
      bool is_negative = value < 0;
      if (is_negative)
      {
        value = -value;
      }

      typename std::make_unsigned<T>::type uvalue = static_cast<typename std::make_unsigned<T>::type>(value);

      // 快速转换
      while (uvalue > 0 && end >= buffer)
      {
        *end    = '0' + (uvalue % 10);
        uvalue /= 10;
        end--;
        len++;
      }

      // 添加符号
      if (is_negative && end >= buffer)
      {
        *end = '-';
        end--;
        len++;
      }
    }
    else
    {
      // 快速转换
      while (value > 0 && end >= buffer)
      {
        *end   = '0' + (value % 10);
        value /= 10;
        end--;
        len++;
      }
    }

    // 移动到缓冲区开始
    end++;
    if (end > buffer)
    {
      memmove(buffer, end, len + 1);
    }

    return len;
  }

  /**
   * @brief  字符格式化 十六进制格式化函数
   *
   * @tparam T          类型
   * @param  buffer     缓冲区
   * @param  value      值
   * @param  buf_size   缓冲区大小
   * @param  uppercase  是否大写
   * @param  add_prefix 是否添加前缀
   * @return int        格式化后的长度
   */
  template <typename T>
  static QAQ_INLINE int QAQ_O3 format_as_hex(char* buffer, T value, uint32_t buf_size, bool uppercase = false, bool add_prefix = false)
  {
    static_assert(std::is_integral<T>::value, "T must be integral type");

    if (buf_size == 0)
    {
      return Error_Code::Buffer_Too_Small;
    }

    // 特殊情况：0
    if (value == 0)
    {
      if (buf_size < 2)
      {
        return Error_Code::Buffer_Too_Small;
      }

      buffer[0] = '0';
      buffer[1] = '\0';
      return 1;
    }

    // 在缓冲区末尾构建数字（逆序）
    char* end = buffer + buf_size - 1;
    *end      = '\0';
    end--;

    int                                  len    = 0;
    typename std::make_unsigned<T>::type uvalue = static_cast<typename std::make_unsigned<T>::type>(value);

    // 使用位运算快速转换
    const char* digit_chars                     = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    while (uvalue > 0 && end >= buffer)
    {
      *end     = digit_chars[uvalue & 0xF];
      uvalue >>= 4;
      end--;
      len++;
    }

    // 添加前缀
    if (add_prefix && (end - buffer) >= 2)
    {
      *end = uppercase ? 'X' : 'x';
      end--;
      *end = '0';
      end--;
      len += 2;
    }
    else if (add_prefix)
    {
      return Error_Code::Buffer_Too_Small;   // 缓冲区不足
    }

    // 移动到缓冲区开始
    end++;
    if (end > buffer)
    {
      memmove(buffer, end, len + 1);
    }

    return len;
  }

  /**
   * @brief  字符格式化 自定义进制格式化函数
   *
   * @tparam T          类型
   * @param  buffer     缓冲区
   * @param  value      值
   * @param  base       进制
   * @param  buf_size   缓冲区大小
   * @return int        格式化后的长度
   */
  template <typename T>
  static QAQ_INLINE int QAQ_O3 format_as_basis(char* buffer, T value, int base, uint32_t buf_size)
  {
    static_assert(std::is_integral<T>::value, "T must be integral type");

    if (buf_size == 0 || base < 2 || base > 36)
    {
      return Error_Code::Invalid_Base;
    }

    // 特殊情况：0
    if (value == 0)
    {
      if (buf_size < 2)
      {
        return Error_Code::Buffer_Too_Small;
      }

      buffer[0] = '0';
      buffer[1] = '\0';
      return 1;
    }

    // 处理符号
    bool is_negative = false;
    if (std::is_signed<T>::value && value < 0 && base == 10)
    {
      is_negative = true;
      value       = -value;
    }

    // 在缓冲区末尾构建数字（逆序）
    char* end = buffer + buf_size - 1;
    *end      = '\0';
    end--;

    int                                  len    = 0;
    typename std::make_unsigned<T>::type uvalue = static_cast<typename std::make_unsigned<T>::type>(value);

    // 转换
    while (uvalue > 0 && end >= buffer)
    {
      int digit  = uvalue % base;
      *end       = digits[digit];
      uvalue    /= base;
      end--;
      len++;
    }

    // 添加符号
    if (is_negative && end >= buffer)
    {
      *end = '-';
      end--;
      len++;
    }

    // 移动到缓冲区开始
    end++;
    if (end > buffer)
    {
      memmove(buffer, end, len + 1);
    }

    return len;
  }

  /**
   * @brief  字符格式化 科学计数法格式化函数
   *
   * @tparam T          类型
   * @param  value      值
   * @param  precision  精度
   * @return true       应该使用科学计数法
   * @return false      不应该使用科学计数法
   */
  template <typename T>
  static QAQ_INLINE bool QAQ_O3 should_use_scientific(T value, int precision)
  {
    if (value == 0.0)
    {
      return false;
    }

    T abs_value = fabs(value);
    if (abs_value < 1e-4 || abs_value >= 1e15)
    {
      return true;
    }

    // 根据精度和数值大小决定是否使用科学计数法
    if (precision >= 0 && abs_value >= fast_pow_10(precision + 2))
    {
      return true;
    }

    return false;
  }

  /**
   * @brief  字符格式化 科学计数法格式化函数
   *
   * @tparam T          类型
   * @param  buffer     缓冲区
   * @param  value      值
   * @param  precision  精度
   * @param  buf_size   缓冲区大小
   * @return int        格式化后的长度
   */
  template <typename T>
  static QAQ_INLINE int QAQ_O3 format_as_scientific(char* buffer, T value, int precision, uint32_t buf_size)
  {
    static_assert(std::is_floating_point<T>::value, "T must be a floating point type");

    if (buf_size < 6)   // 至少需要 "1e+00" + '\0'
    {
      return Error_Code::Buffer_Too_Small;
    }

    char*    ptr       = buffer;
    uint32_t remaining = buf_size;

    // 处理符号
    if (value < 0)
    {
      if (remaining < 2)
      {
        return Error_Code::Buffer_Too_Small;
      }

      *ptr++ = '-';
      value  = -value;
      remaining--;
    }

    // 计算指数
    int exponent = 0;
    if (value != 0.0)
    {
      exponent  = (int)floor(fast_log_10(value));
      value    *= fast_pow_10(-exponent);
    }

    // 确保第一位数字在1-9之间
    if (value >= 10.0)
    {
      value /= 10.0;
      exponent++;
    }
    else if (value < 1.0 && value > 0.0)
    {
      value *= 10.0;
      exponent--;
    }

    // 转换小数部分
    int int_part  = (int)value;
    T   frac_part = value - int_part;

    // 输出整数部分
    if (remaining < 2)
    {
      return Error_Code::Buffer_Too_Small;
    }

    *ptr++ = '0' + int_part;
    remaining--;

    if (precision > 0)
    {
      if (remaining < 2)
      {
        return Error_Code::Buffer_Too_Small;
      }

      *ptr++ = '.';
      remaining--;

      T scale = 1.0;
      for (int i = 0; i < precision; i++)
        scale *= 10.0;

      int scaled_frac = (int)(frac_part * scale + 0.5);

      // 处理精度
      for (int i = 0; i < precision && remaining > 1; i++)
      {
        int digit = (scaled_frac / (int)fast_pow_10(precision - 1 - i)) % 10;
        *ptr++    = '0' + digit;
        remaining--;
      }
    }

    // 输出指数部分
    if (remaining < 3)
    {
      return Error_Code::Buffer_Too_Small;
    }

    *ptr++ = 'e';
    remaining--;

    if (exponent < 0)
    {
      if (remaining < 2)
      {
        return Error_Code::Buffer_Too_Small;
      }

      *ptr++   = '-';
      exponent = -exponent;
      remaining--;
    }
    else
    {
      if (remaining < 2)
      {
        return Error_Code::Buffer_Too_Small;
      }

      *ptr++ = '+';
      remaining--;
    }

    // 转换指数数字（统一为两位）
    if (remaining < 3)
    {
      return Error_Code::Buffer_Too_Small;
    }

    char exp_buffer[16];
    int  exp_len = format_as_dec<int>(exp_buffer, abs(exponent), sizeof(exp_buffer));

    if (exp_len < 0 || (uint32_t)exp_len >= remaining)
    {
      return Error_Code::Buffer_Too_Small;
    }

    // 确保指数为两位数
    if (abs(exponent) < 10)
    {
      *ptr++ = '0';
      *ptr++ = exp_buffer[0];
    }
    else
    {
      memcpy(ptr, exp_buffer, exp_len);
      ptr += exp_len;
    }

    *ptr = '\0';
    return ptr - buffer;
  }

  /**
   * @brief  字符格式化 小数格式化函数
   *
   * @tparam T                类型
   * @param  buffer           缓冲区
   * @param  value            值
   * @param  precision        精度
   * @param  buf_size         缓冲区大小
   * @param  use_scientific   是否使用科学计数法
   * @return int              格式化后的长度
   */
  template <typename T>
  static QAQ_INLINE int QAQ_O3 format_as_fraction(char* buffer, T value, int precision, uint32_t buf_size, bool use_scientific = false)
  {
    static_assert(std::is_floating_point<T>::value, "T must be a floating point type");

    if (buf_size == 0)
    {
      return Error_Code::Buffer_Too_Small;
    }

    char*    ptr       = buffer;
    uint32_t remaining = buf_size;

    // 处理特殊值
    if (isnan(value))
    {
      if (remaining < 4)
      {
        return Error_Code::Buffer_Too_Small;
      }

      buffer[0] = 'n';
      buffer[1] = 'a';
      buffer[2] = 'n';
      buffer[3] = '\0';
      return 3;
    }
    if (isinf(value))
    {
      if (value < 0)
      {
        if (remaining < 5)
        {
          return Error_Code::Buffer_Too_Small;
        }

        buffer[0] = '-';
        buffer[1] = 'i';
        buffer[2] = 'n';
        buffer[3] = 'f';
        buffer[4] = '\0';
        return 4;
      }
      else
      {
        if (remaining < 4)
        {
          return Error_Code::Buffer_Too_Small;
        }

        buffer[0] = 'i';
        buffer[1] = 'n';
        buffer[2] = 'f';
        buffer[3] = '\0';
        return 3;
      }
    }

    // 检查是否强制使用科学计数法或自动判断
    if (use_scientific || should_use_scientific(value, precision))
    {
      return format_as_scientific(buffer, value, precision, buf_size);
    }

    // 处理符号
    if (value < 0)
    {
      if (remaining < 2)
      {
        return Error_Code::Buffer_Too_Small;
      }

      *ptr++ = '-';
      value  = -value;
      remaining--;
    }

    // 处理精度为0的情况
    if (precision == 0)
    {
      uint64_t integer_part = (uint64_t)(value + 0.5);
      int      len          = format_as_dec<uint64_t>(ptr, integer_part, remaining);
      return (len < 0) ? len : (ptr - buffer + len);
    }

    // 分离整数部分和小数部分
    uint64_t integer_part    = (uint64_t)value;
    T        fractional_part = value - integer_part;

    // 转换整数部分
    int int_len              = format_as_dec<uint64_t>(ptr, integer_part, remaining);

    if (int_len < 0)
    {
      return int_len;
    }

    ptr       += int_len;
    remaining -= int_len;

    // 处理小数部分
    if (precision > 0 && remaining > 1)
    {
      *ptr++ = '.';
      remaining--;

      if (remaining <= (uint32_t)precision)
      {
        return Error_Code::Buffer_Too_Small;
      }

      double scale             = fast_pow_10(precision);

      uint64_t scaled_fraction = (uint64_t)(fractional_part * scale + 0.5);

      // 处理前导零
      char frac_temp[32];
      int  frac_len     = format_as_dec<uint64_t>(frac_temp, scaled_fraction, sizeof(frac_temp));

      int leading_zeros = precision - frac_len;
      while (leading_zeros > 0 && remaining > 1)
      {
        *ptr++ = '0';
        remaining--;
        leading_zeros--;
      }

      // 复制小数部分
      for (int i = 0; i < frac_len && remaining > 1; i++)
      {
        *ptr++ = frac_temp[i];
        remaining--;
      }
    }

    *ptr = '\0';
    return ptr - buffer;
  }

public:
  /**
   * @brief  字符格式化 格式化函数
   *
   * @tparam T                类型
   * @param  buffer           缓冲区
   * @param  buf_size         缓冲区大小
   * @param  value            值
   * @param  base             进制
   * @param  uppercase        是否大写
   * @param  add_prefix       是否添加前缀
   * @return int              格式化后的长度
   */
  template <typename T>
  static QAQ_INLINE int QAQ_O3 format(char* buffer, uint32_t buf_size, T value, int base = 10, bool uppercase = false, bool add_prefix = false)
  {
    static_assert(std::is_integral<T>::value, "T must be integral type");

    switch (base)
    {
      case 2 :
        return format_as_bin<T>(buffer, value, buf_size);
      case 8 :
        return format_as_oct<T>(buffer, value, buf_size, add_prefix);
      case 10 :
        return format_as_dec<T>(buffer, value, buf_size);
      case 16 :
        return format_as_hex<T>(buffer, value, buf_size, uppercase, add_prefix);
      default :
        return format_as_basis<T>(buffer, value, base, buf_size);
    }
  }

  /**
   * @brief  字符格式化 格式化函数
   *
   * @param  buffer           缓冲区
   * @param  buf_size         缓冲区大小
   * @param  value            值
   * @param  precision        精度
   * @param  use_scientific   是否使用科学计数法
   * @return int              格式化后的长度
   */
  static QAQ_INLINE int QAQ_O3 format(char* buffer, uint32_t buf_size, float value, int precision = 6, bool use_scientific = false)
  {
    return format_as_fraction<float>(buffer, value, precision, buf_size, use_scientific);
  }

  /**
   * @brief  字符格式化 格式化函数
   *
   * @param  buffer           缓冲区
   * @param  buf_size         缓冲区大小
   * @param  value            值
   * @param  precision        精度
   * @param  use_scientific   是否使用科学计数法
   * @return int              格式化后的长度
   */
  static QAQ_INLINE int QAQ_O3 format(char* buffer, uint32_t buf_size, double value, int precision = 6, bool use_scientific = false)
  {
    return format_as_fraction<double>(buffer, value, precision, buf_size, use_scientific);
  }
};
} /* namespace algorithm */
} /* namespace system */
} /* namespace QAQ */

#endif /* __FORMAT_HPP__ */