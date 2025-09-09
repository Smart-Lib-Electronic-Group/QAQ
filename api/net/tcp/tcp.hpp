#ifndef __TCP_HPP__
#define __TCP_HPP__

#include "tcp_server_base.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 网络
namespace net
{
/// @brief 名称空间 TCP
namespace tcp
{
/**
 * @brief  TCP 服务器模板类
 *
 * @tparam Client_Count   最大可支持的客户端数量
 * @tparam Base_Device    设备基类
 * @tparam Stack_Size     服务器线程栈大小
 * @tparam Queue_Size     服务器线程信号队列大小
 */
template <uint8_t Client_Count, typename Base_Device, uint32_t Stack_Size = TX_MINIMUM_STACK, uint32_t Queue_Size = 0>
using Tcp_Server = net_internal::tcp_internal::Tcp_Server_Base<Client_Count, Base_Device::stream_type(), Base_Device, Stack_Size, Queue_Size, void>;

/**
 * @brief  TCP 服务器模板类 - CPRT
 *
 * @tparam Client_Count   最大可支持的客户端数量
 * @tparam Base_Device    设备基类
 * @tparam Derived        派生类
 * @tparam Stack_Size     服务器线程栈大小
 * @tparam Queue_Size     服务器线程信号队列大小
 */
template <uint8_t Client_Count, typename Base_Device, typename Derived, uint32_t Stack_Size = TX_MINIMUM_STACK, uint32_t Queue_Size = 0>
using Tcp_Cprt_Server = net_internal::tcp_internal::Tcp_Server_Base<Client_Count, Base_Device::stream_type(), Base_Device, Stack_Size, Queue_Size, Derived>;

/**
 * @brief  TCP 输入输出设备模板类
 *
 * @tparam In_Buf_Size  输入缓冲区大小
 * @tparam Out_Buf_Size 输出缓冲区大小
 */
template <uint32_t In_Buf_Size, uint32_t Out_Buf_Size>
using Tcp_IODevice = system::device::Stream_Device<QAQ::system::device::Stream_Type::READ_WRITE, In_Buf_Size, Out_Buf_Size>;

/**
 * @brief  TCP 输入设备模板类
 *
 * @tparam In_Buf_Size 输入缓冲区大小
 */
template <uint32_t In_Buf_Size>
using Tcp_IDevice = system::device::Stream_Device<QAQ::system::device::Stream_Type::READ_ONLY, In_Buf_Size, 0>;

/**
 * @brief  TCP 输出设备模板类
 *
 * @tparam Out_Buf_Size 输出缓冲区大小
 */
template <uint32_t Out_Buf_Size>
using Tcp_ODevice = system::device::Stream_Device<QAQ::system::device::Stream_Type::WRITE_ONLY, 0, Out_Buf_Size>;
} /* namespace tcp */
} /* namespace net */
} /* namespace QAQ */

#endif /* __TCP_HPP__ */
