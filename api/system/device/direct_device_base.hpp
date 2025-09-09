#ifndef __DIRECT_DEVICE_BASE_HPP__
#define __DIRECT_DEVICE_BASE_HPP__

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
 * @brief 直接传输设备基类
 *
 */
class Direct_Device_Base : public Device_Base, public Input_Base, public Output_Base
{
  // 禁止拷贝和移动
  NO_COPY_MOVE(Direct_Device_Base)

protected:
  /**
   * @brief  直接传输设备基类 发送事件
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
   * @brief  直接传输设备基类 构造函数
   *
   */
  explicit Direct_Device_Base() {}

  /**
   * @brief  直接传输设备基类 析构函数
   *
   */
  virtual ~Direct_Device_Base() {}

public:
  /**
   * @brief  直接传输设备基类 获取设备类型
   *
   * @return Device_Type 设备类型
   */
  device::Device_Type get_type(void) const override
  {
    return device::Device_Type::DIRECTORY;
  }

  /**
   * @brief  直接传输设备基类 获取设备类型
   *
   * @return Device_Type 设备类型
   */
  static constexpr device::Device_Type type() noexcept
  {
    return device::Device_Type::DIRECTORY;
  }
};
} /* namespace device_internal */
} /* namespace system_internal */
} /* namespace system */
} /* namespace QAQ */

#endif /* __DIRECT_DEVICE_BASE_HPP__ */
