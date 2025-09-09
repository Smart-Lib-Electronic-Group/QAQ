#ifndef __STORAGE_DEVICE_BASE_HPP__
#define __STORAGE_DEVICE_BASE_HPP__

#include "device_manger.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 系统
namespace system
{
/// @brief 命名空间 内部
namespace system_internal
{
/// @brief 名称空间 设备 内部
namespace device_internal
{
/**
 * @brief 储存设备基类
 *
 */
class Storage_Device_Base : public Device_Base
{
  // 禁止拷贝和移动
  NO_COPY_MOVE(Storage_Device_Base)

protected:
  /**
   * @brief  储存设备基类 发送事件
   *
   * @param  event 事件标志
   * @return true  发送成功
   * @return false 发送失败
   */
  bool post_event(uint32_t event) override
  {
    return Device_Manager::instance().post_event(this, event);
  }

  /**
   * @brief  储存设备基类 构造函数
   */
  explicit Storage_Device_Base() {}

  /**
   * @brief  储存设备基类 析构函数
   */
  virtual ~Storage_Device_Base() {}

public:
  /**
   * @brief  储存设备基类 写入数据 - 纯虚函数
   *
   * @param  address   写入地址
   * @param  data      数据缓存
   * @param  size      数据大小
   * @return int64_t   实际写入数据大小
   */
  virtual int64_t write(uint32_t address, const uint8_t* data, uint32_t size) = 0;

  /**
   * @brief  储存设备基类 读取数据 - 纯虚函数
   *
   * @param  address   读取地址
   * @param  data      数据缓存
   * @param  size      数据大小
   * @return int64_t   实际读取数据大小
   */
  virtual int64_t read(uint32_t address, uint8_t* data, uint32_t size)        = 0;

  /**
   * @brief  储存设备基类 擦除数据 - 纯虚函数
   *
   * @param  address   擦除地址
   * @param  size      擦除大小
   * @return int64_t   实际擦除数据大小
   */
  virtual int64_t erase(uint32_t address, uint32_t size)                      = 0;

  /**
   * @brief  储存设备基类 获取设备类型
   *
   * @return Device_Type 设备类型
   */
  device::Device_Type get_type(void) const override
  {
    return device::Device_Type::STORAGE;
  }

  /**
   * @brief  储存设备基类 获取设备类型
   *
   * @return Device_Type 设备类型
   */
  static constexpr device::Device_Type type() noexcept
  {
    return device::Device_Type::STORAGE;
  }
};
} /* namespace device_internal */
} /* namespace system_internal */
} /* namespace system */
} /* namespace QAQ */

#endif /* __STORAGE_DEVICE_BASE_HPP__ */
