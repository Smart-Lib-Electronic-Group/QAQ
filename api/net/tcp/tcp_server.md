# TCP服务器 使用说明

## 目录
1. [概述](#概述)
2. [架构设计](#架构设计)
3. [核心组件](#核心组件)
4. [使用方法](#使用方法)
5. [配置选项](#配置选项)
6. [示例代码](#示例代码)
7. [错误处理](#错误处理)
8. [性能优化建议](#性能优化建议)

## 概述

TCP服务器框架是一个基于C++模板的高性能TCP服务器解决方案，提供了完整的客户端管理、数据收发、超时处理等功能。该框架采用分层设计，具有良好的可扩展性和可维护性。

## 架构设计

```
设备层 (Device Layer)
├── 设备基类 (Device_Base)
├── 流设备基类 (Stream_Device)
└── TCP设备 (Tcp_IODevice/Tcp_IDevice/Tcp_ODevice)

网络层 (Network Layer)
├── TCP套接字 (Tcp_Socket)
├── 服务器客户端 (Tcp_Server_Client)
└── 服务器基类 (Tcp_Server_Base)

管理层 (Management Layer)
├── 设备管理器 (Device_Manager)
└── 网络管理器 (Net_Manager)
```

## 核心组件

### 1. TCP设备类型

#### Tcp_IODevice (读写设备)
```cpp
template <uint32_t In_Buf_Size, uint32_t Out_Buf_Size>
using Tcp_IODevice = system::device::Stream_Device<READ_WRITE, In_Buf_Size, Out_Buf_Size>;
```

#### Tcp_IDevice (只读设备)
```cpp
template <uint32_t In_Buf_Size>
using Tcp_IDevice = system::device::Stream_Device<READ_ONLY, In_Buf_Size, 0>;
```

#### Tcp_ODevice (只写设备)
```cpp
template <uint32_t Out_Buf_Size>
using Tcp_ODevice = system::device::Stream_Device<WRITE_ONLY, 0, Out_Buf_Size>;
```

### 2. TCP服务器类型

#### Tcp_Server (信号模式服务器)
```cpp
template <uint8_t Client_Count, typename Base_Device, uint32_t Stack_Size, uint32_t Queue_Size>
using Tcp_Server = internal::tcp_internal::Tcp_Server_Base<Client_Count, Base_Device::stream_type(), Base_Device, Stack_Size, Queue_Size, void>;
```

#### Tcp_Cprt_Server (CPRT模式服务器)
```cpp
template <uint8_t Client_Count, typename Base_Device, typename Derived, uint32_t Stack_Size, uint32_t Queue_Size>
using Tcp_Cprt_Server = internal::tcp_internal::Tcp_Server_Base<Client_Count, Base_Device::stream_type(), Base_Device, Stack_Size, Queue_Size, Derived>;
```

## 使用方法

### 1. 基本使用步骤

#### 步骤1：定义设备类型
```cpp
// 定义一个读写TCP设备，输入缓冲区1024字节，输出缓冲区512字节
using MyTcpDevice = QAQ::net::tcp::Tcp_IODevice<1024, 512>;
```

#### 步骤2：创建服务器实例
```cpp
// 创建支持最多4个客户端的TCP服务器
QAQ::net::tcp::Tcp_Server<4, MyTcpDevice> tcp_server;
```

#### 步骤3：启动服务器
```cpp
// 启动服务器，监听端口8080，优先级5，超时时间60秒
auto result = tcp_server.start("MyTcpServer", 8080, 5, 60);
if (result != QAQ::net::internal::tcp_internal::Server_Error_Code::SUCCESS) {
    // 处理启动失败
}
```

#### 步骤4：连接信号处理
当前信号系统仅支持全局函数、成员函数作为信号处理函数，不支持Lambda表达式
```cpp
// 连接信号
void client_connected(Client* client)
{
   printf("客户端已连接\n"); 
}

tcp_server.signal_client_connected.connect(client_connected);

// 接收信号
void client_received(Client* client)
{
    printf("收到客户端数据\n");
}

tcp_server.signal_client_received.connect(client_received);

// 断开信号
void client_disconnected(Client* client)
{
    printf("客户端已断开\n");
}

tcp_server.signal_client_disconnected.connect(client_disconnected);
```

### 2. CPRT模式使用

#### 自定义服务器类
```cpp
class MyCustomServer : public QAQ::net::tcp::Tcp_Cprt_Server<4, MyTcpDevice, MyCustomServer>
{
public:
    void client_connected(Client* client)
    {
        printf("客户端连接: %p\n", client);
        // 自定义连接处理逻辑
    }
    
    void client_received(Client* client)
    {
        printf("收到数据: %p\n", client);
        // 自定义数据接收处理逻辑
    }
    
    void client_disconnected(Client* client)
    {
        printf("客户端断开: %p\n", client);
        // 自定义断开处理逻辑
    }
    
    uint32_t client_timeout(Client* client)
    {
        printf("客户端超时: %p\n", client);
        return 30; // 返回延迟关闭时间(秒)
    }
};
```

#### 使用自定义服务器
```cpp
MyCustomServer custom_server;
custom_server.start("CustomServer", 9090, 5, 120);
```

## 配置选项

### 1. 模板参数配置

| 参数 | 类型 | 描述 | 默认值 |
|------|------|------|--------|
| `Client_Count` | `uint8_t` | 最大客户端连接数 | 无默认值 |
| `Base_Device` | `typename` | 设备基类类型 | 无默认值 |
| `Stack_Size` | `uint32_t` | 服务器线程栈大小 | `TX_MINIMUM_STACK` |
| `Queue_Size` | `uint32_t` | 服务器线程信号队列大小 | `0` |

### 2. 启动参数配置

```cpp
Server_Error_Code start(
    const char* name,           // 服务器名称
    uint16_t port,              // 监听端口
    uint16_t priority = 5,      // 线程优先级
    uint32_t timeout_count = 60 // 超时时间(秒)
);
```

### 3. 缓冲区配置

| 设备类型 | 输入缓冲区 | 输出缓冲区 | 说明 |
|----------|------------|------------|------|
| `Tcp_IODevice` | 可配置 | 可配置 | 读写设备 |
| `Tcp_IDevice` | 可配置 | 0 | 只读设备 |
| `Tcp_ODevice` | 0 | 可配置 | 只写设备 |

## 示例代码

### 1. 简单TCP服务器示例

```cpp
#include "tcp.hpp"

// 定义TCP设备类型
using TcpDevice = QAQ::net::tcp::Tcp_IODevice<1024, 512>;

void client_connected(Client* client)
{
    printf("客户端已连接\n");
}

void client_received(Client* client)
{
    uint8_t buffer[256];
        int64_t bytes_read = device->read(buffer, sizeof(buffer), 1000);
        if (bytes_read > 0) {
            printf("收到数据: %.*s\n", (int)bytes_read, buffer);
            // 回显数据
            device->write(buffer, bytes_read);
        }
}

void client_disconnected(Client* client)
{
    printf("客户端已断开\n");
}

int main()
{
    // 创建TCP服务器，支持4个客户端
    QAQ::net::tcp::Tcp_Server<4, TcpDevice> server;
    
    // 启动服务器
    auto result = server.start("SimpleTCPServer", 8080, 5, 60);
    if (result != QAQ::net::internal::tcp_internal::Server_Error_Code::SUCCESS) {
        printf("服务器启动失败\n");
        return -1;
    }
    
    // 连接信号处理
    server.signal_client_connected.connect(client_connected);
    
    server.signal_client_received.connect(client_received);
    
    server.signal_client_disconnected.connect(client_disconnected);
    
    // 服务器运行
    while (true) {
        // 主循环处理
        QAQ::system::thread::sleep(1000);
    }
    
    return 0;
}
```

### 2. 自定义TCP服务器示例

```cpp
#include "tcp.hpp"

using TcpDevice = QAQ::net::tcp::Tcp_IODevice<2048, 1024>;

class EchoServer : public QAQ::net::tcp::Tcp_Cprt_Server<8, TcpDevice, EchoServer>
{
private:
    uint32_t message_count = 0;
    
public:
    void client_connected(Client* client)
    {
        message_count = 0;
        printf("客户端连接，当前连接数: %d\n", get_opened_client_count());
    }
    
    void client_received(Client* client)
    {
        // 获取设备引用
        auto& device = client->device();
        
        // 读取所有可用数据
        uint32_t available = device.available();
        if (available > 0) {
            std::vector<uint8_t> buffer(available);
            int64_t bytes_read = device.read(buffer.data(), available, 0);
            
            if (bytes_read > 0) {
                message_count++;
                printf("消息 #%d: %.*s\n", message_count, (int)bytes_read, buffer.data());
                
                // 回显数据
                std::string response = "Echo: " + std::string((char*)buffer.data(), bytes_read) + "\n";
                device.write((uint8_t*)response.c_str(), response.length());
            }
        }
    }
    
    void client_disconnected(Client* client)
    {
        printf("客户端断开，当前连接数: %d\n", get_opened_client_count());
    }
    
    uint32_t client_timeout(Client* client)
    {
        printf("客户端超时，延迟关闭30秒\n");
        return 30; // 30秒后强制关闭
    }
};

int main()
{
    EchoServer server;
    
    // 启动服务器，监听端口9090
    auto result = server.start("EchoServer", 9090, 5, 120);
    if (result != QAQ::net::internal::tcp_internal::Server_Error_Code::SUCCESS) {
        printf("服务器启动失败\n");
        return -1;
    }
    
    printf("Echo服务器启动成功，监听端口9090\n");
    
    // 服务器运行
    while (true) {
        QAQ::system::thread::sleep(1000);
    }
    
    return 0;
}
```

### 3. 广播服务器示例

```cpp
#include "tcp.hpp"

using TcpDevice = QAQ::net::tcp::Tcp_IODevice<1024, 512>;

class BroadcastServer : public QAQ::net::tcp::Tcp_Cprt_Server<10, TcpDevice, BroadcastServer>
{
public:
    void client_connected(Client* client)
    {
        printf("新客户端连接，总连接数: %d\n", get_opened_client_count());
        
        // 向所有客户端广播新用户加入
        std::string message = "用户加入，当前在线: " + 
                             std::to_string(get_opened_client_count()) + "人\n";
        write_all_opened_client((uint8_t*)message.c_str(), message.length());
    }
    
    void client_received(Client* client)
    {
        auto& device = client->device();
        uint32_t available = device.available();
        
        if (available > 0) {
            std::vector<uint8_t> buffer(available);
            int64_t bytes_read = device.read(buffer.data(), available, 0);
            
            if (bytes_read > 0) {
                // 广播消息给所有客户端
                write_all_opened_client(buffer.data(), bytes_read);
            }
        }
    }
    
    void client_disconnected(Client* client)
    {
        printf("客户端断开，剩余连接数: %d\n", get_opened_client_count());
        
        // 向所有客户端广播用户离开
        std::string message = "用户离开，当前在线: " + 
                             std::to_string(get_opened_client_count()) + "人\n";
        write_all_opened_client((uint8_t*)message.c_str(), message.length());
    }
};

int main()
{
    BroadcastServer server;
    
    auto result = server.start("BroadcastServer", 8080, 5, 300);
    if (result != QAQ::net::internal::tcp_internal::Server_Error_Code::SUCCESS) {
        printf("服务器启动失败\n");
        return -1;
    }
    
    printf("广播服务器启动成功，监听端口8080\n");
    
    while (true) {
        QAQ::system::thread::sleep(1000);
    }
    
    return 0;
}
```

## 错误处理

### 1. 服务器错误码

```cpp
enum class Server_Error_Code
{
    SUCCESS,           // 成功
    NO_FREE_CLIENT,    // 无空闲客户端
    LISTEN_FAILED,     // 监听失败
    RELISTEN_FAILED,   // 重新监听失败
    INIT_ERROR         // 初始化错误
};
```

### 2. 设备错误码

```cpp
enum class Device_Error_Code
{
    OK,                // 成功
    TIMEOUT,           // 超时
    BUSY,              // 繁忙
    INVALID_PARAMETER, // 参数无效
    NOT_OPENED,        // 未打开
    CLOSED,            // 已关闭
    IO_ERROR,          // 输入输出错误
    INIT_FAILED,       // 初始化失败
    DEINIT_FAILED,     // 解除初始化失败
    PORT_ALREADY_USED, // 端口已被使用
    WAIT_FOR_CONNECT   // 等待连接
};
```

### 3. 错误处理示例

```cpp
auto result = tcp_server.start("MyServer", 8080);
switch (result) {
    case QAQ::net::internal::tcp_internal::Server_Error_Code::SUCCESS:
        printf("服务器启动成功\n");
        break;
    case QAQ::net::internal::tcp_internal::Server_Error_Code::NO_FREE_CLIENT:
        printf("无空闲客户端\n");
        break;
    case QAQ::net::internal::tcp_internal::Server_Error_Code::LISTEN_FAILED:
        printf("监听端口失败\n");
        break;
    default:
        printf("服务器启动失败\n");
        break;
}
```

## 性能优化建议

### 1. 缓冲区优化
```cpp
// 根据实际需求调整缓冲区大小
using HighPerformanceDevice = QAQ::net::tcp::Tcp_IODevice<8192, 4096>; // 大缓冲区
using LowMemoryDevice = QAQ::net::tcp::Tcp_IODevice<256, 128>;        // 小缓冲区
```

### 2. 线程配置
```cpp
// 根据系统资源调整线程栈大小和优先级
QAQ::net::tcp::Tcp_Server<4, TcpDevice, 4096, 16> server; // 大栈空间，大队列
```

### 3. 超时配置
```cpp
// 根据网络环境调整超时时间
server.start("Server", 8080, 5, 30);  // 短超时(30秒)
server.start("Server", 8080, 5, 600); // 长超时(10分钟)
```

### 4. 客户端数量优化
```cpp
// 根据实际并发需求设置客户端数量
QAQ::net::tcp::Tcp_Server<2, TcpDevice> small_server;   // 小型服务器
QAQ::net::tcp::Tcp_Server<100, TcpDevice> large_server; // 大型服务器
```

### 5. 内存池优化
```cpp
// 确保网络内存池大小足够
// 在系统初始化时配置合适的内存池大小
```

---

**注意**: 本框架基于NetX网络协议栈设计，使用前请确保已正确初始化NetX系统。