#ifndef __UART_HPP__
#define __UART_HPP__

#include "uart_config.hpp"
#include "uart_base.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 基本对象
namespace base
{
/// @brief 名称空间 串口
namespace uart
{
/**
 * @brief  Uart 模板类
 *
 * @tparam Config      Uart 配置模版
 * @tparam Base_Device Uart 设备基类
 */
template <typename Config, typename Base_Device>
using Uart = base_internal::uart_internal::Uart_Base<Base_Device::stream_type(), Config, Base_Device>;

/**
 * @brief  RS485 模板类
 *
 * @tparam Config      Uart 配置模版
 * @tparam Base_Device Uart 设备基类
 * @tparam DE_Pin      DE 引脚
 * @tparam RE_Pin      RE 引脚
 */
template <typename Config, typename Base_Device, typename DE_Pin, typename RE_Pin = void>
using RS485 = base_internal::uart_internal::Uart_Base<Base_Device::stream_type(), Config, Base_Device, DE_Pin, RE_Pin>;

/**
 * @brief  Uart 输入输出设备模板类
 *
 * @tparam In_Buf_Size  输入缓冲区大小
 * @tparam Out_Buf_Size 输出缓冲区大小
 */
template <uint32_t In_Buf_Size, uint32_t Out_Buf_Size>
using Uart_IODevice = system::device::Stream_Device<QAQ::system::device::Stream_Type::READ_WRITE, In_Buf_Size, Out_Buf_Size>;

/**
 * @brief  Uart 输入设备模板类
 *
 * @tparam In_Buf_Size 输入缓冲区大小
 */
template <uint32_t In_Buf_Size>
using Uart_IDevice = system::device::Stream_Device<QAQ::system::device::Stream_Type::READ_ONLY, In_Buf_Size, 0>;

/**
 * @brief  Uart 输出设备模板类
 *
 * @tparam Out_Buf_Size 输出缓冲区大小
 */
template <uint32_t Out_Buf_Size>
using Uart_ODevice = system::device::Stream_Device<QAQ::system::device::Stream_Type::WRITE_ONLY, 0, Out_Buf_Size>;
} /* namespace uart */
} /* namespace base */
} /* namespace QAQ */

#endif /* __UART_HPP__ */
