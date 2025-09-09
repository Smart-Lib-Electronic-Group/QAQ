# UART 使用说明

## 目录
1. [概述](#概述)
2. [架构设计](#架构设计)
3. [核心组件](#核心组件)
4. [使用方法](#使用方法)
5. [配置模板](#配置模板)
6. [设备类型](#设备类型)
7. [工作模式](#工作模式)
8. [API参考](#api参考)
9. [示例代码](#示例代码)

## 概述

QAQ UART驱动框架是一个基于现代C++设计的嵌入入式UART通信解决方案。该框架提供了完整的UART功能支持，包括中断模式、DMA模式、双缓冲DMA模式等，同时支持RS485通信。

## 架构设计

```
UART Framework
├── Interrupt Manager (中断管理)
├── DMA Base (DMA基础)
├── Device Manager (设备管理)
├── Stream Device (流设备)
└── UART Base (UART基础)
```

## 核心组件

### 1. 中断管理器 (Interrupt_Manager)
- 统一的中断处理机制
- 支持同步和异步中断处理
- 线程安全的中断管理

### 2. DMA管理 (Dma_Base)
- 模板化的DMA配置
- 支持多种DMA模式
- 完善的DMA通道管理

### 3. 设备管理器 (Device_Manager)
- 异步事件处理
- 设备状态管理
- 线程安全的消息队列

### 4. 流设备 (Stream_Device)
- 模板化的缓冲区管理
- 支持多种流类型（只读、只写、读写）
- 环形缓冲区实现

## 使用方法

### 1. 基本UART使用

```cpp
// 1. 定义配置模板
using MyUartConfig = QAQ::base::uart::Uart_Config<5, Uart_Type::DMA_Double_Buffer, Uart_Type::DMA>;

// 2. 定义设备类型
using MyUartDevice = QAQ::base::uart::Uart_IODevice<256, 256>;

// 3. 创建UART实例
QAQ::base::uart::Uart<MyUartConfig, MyUartDevice> uart;

// 4. 使用和配置
uart.open();

uart.config()
    .baud_rate(115200)
    .data_bits(8)
    .stop_bits(1)
    .parity(QAQ::base::uart::Uart_Parity::None);
```

### 2. RS485使用

```cpp
// 1. 定义配置模板
using MyUartConfig = QAQ::base::uart::Uart_Config<5, Uart_Type::DMA_Double_Buffer, Uart_Type::DMA>;

// 2. 定义设备类型
using MyUartDevice = QAQ::base::uart::Uart_IODevice<256, 256>;

// 3. 定义DE/RE引脚
using DE             = Gpio<Pin_Port::PC, 8>;
using RE             = Gpio<Pin_Port::PC, 9>;

// 4. 创建RS485实例
QAQ::base::uart::RS485<MyUartConfig, MyUartDevice, DE, RE> rs485;
```

### 配置参数

| 参数 | 说明 | 类型 |
|------|------|------|
| Baud_Rate | 波特率 | uint32_t |
| Data_Bits | 数据位 | uint32_t |
| Stop_Bits | 停止位 | uint32_t |
| Parity | 校验位 | uint32_t |
| Interrupt_Priority | 中断优先级 | uint32_t |
| Interrupt_Sub_Priority | 中断子优先级 | uint32_t |

## 设备类型

### 1. 输入输出设备 (READ_WRITE)
```cpp
template <uint32_t In_Buf_Size, uint32_t Out_Buf_Size>
using Uart_IODevice = system::device::Stream_Device<Stream_Type::READ_WRITE, In_Buf_Size, Out_Buf_Size>;
```

### 2. 输入设备 (READ_ONLY)
```cpp
template <uint32_t In_Buf_Size>
using Uart_IDevice = system::device::Stream_Device<Stream_Type::READ_ONLY, In_Buf_Size, 0>;
```

### 3. 输出设备 (WRITE_ONLY)
```cpp
template <uint32_t Out_Buf_Size>
using Uart_ODevice = system::device::Stream_Device<Stream_Type::WRITE_ONLY, 0, Out_Buf_Size>;
```

## 工作模式

### 1. 正常模式 (Normal)
- 轮询方式数据传输
- 适用于简单应用场景

### 2. 中断模式 (Interrupt)
- 中断驱动数据接收
- 减少CPU占用率

### 3. DMA模式 (DMA)
- DMA数据传输
- 高效的数据处理

### 4. DMA双缓冲模式 (DMA_Double_Buffer)
- 双缓冲DMA传输
- 零拷贝数据处理

## API参考

### UART配置接口

```cpp
// 配置设置
Uart_Set_Config& baud_rate(uint32_t baud_rate);
Uart_Set_Config& data_bits(uint32_t data_bits);
Uart_Set_Config& stop_bits(uint32_t stop_bits);
Uart_Set_Config& parity(uint32_t parity);
Uart_Set_Config& interrupt_priority(uint32_t priority);
Uart_Set_Config& interrupt_sub_priority(uint32_t sub_priority);

// 配置获取
uint32_t baud_rate();
uint32_t data_bits();
uint32_t stop_bits();
uint32_t parity();
uint32_t interrupt_priority();
uint32_t interrupt_sub_priority();
uint32_t port_num();
```

### 设备操作接口

```cpp
// 基本操作
system::device::Device_Error_Code open();
system::device::Device_Error_Code close();
system::device::Device_Error_Code config(uint32_t param, uint32_t value);
const uint32_t get_config(uint32_t param);

// 数据读写
int64_t read(uint8_t* data, uint32_t size, uint32_t timeout_ms = TX_WAIT_FOREVER);
int64_t write(const uint8_t* data, uint32_t size, uint32_t timeout_ms = 0);
system::device::Device_Error_Code flush(uint32_t timeout_ms = TX_WAIT_FOREVER);

// 缓冲区操作
uint32_t available() const;
bool empty() const;
bool full() const;
system::device::Device_Error_Code clear();
int64_t peek(uint8_t* data, uint32_t request, uint32_t timeout_ms = 0);
void roll_back();
```

## 示例代码

### 1. 基本UART通信

```cpp
#include "uart.hpp"

// 自定义UART配置
using STM32UartConfig = QAQ::base::uart::Uart_Config<5, Uart_Type::DMA_Double_Buffer, Uart_Type::DMA>;

// 定义设备类型
using MyUartDevice = QAQ::base::uart::Uart_IODevice<256, 256>;

// 创建UART实例
QAQ::base::uart::Uart<STM32UartConfig, MyUartDevice> uart;

int main() {
    // 打开设备
    if (uart.open() == system::device::Device_Error_Code::OK) {

      // 配置UART
      uart.config()
          .baud_rate(115200)
          .data_bits(8)
          .stop_bits(1)
          .parity(QAQ::base::uart::Uart_Parity::None);

        uint8_t buffer[100];
        
        // 发送数据
        const char* message = "Hello UART!";
        uart.write(reinterpret_cast<const uint8_t*>(message), strlen(message));
        
        // 接收数据
        int64_t received = uart.read(buffer, sizeof(buffer), 1000);
        if (received > 0) {
            // 处理接收到的数据
        }
    }
    
    return 0;
}
```

### 2. RS485通信

```cpp
// 定义DE引脚
using DE_Pin = Gpio<Pin_Port::PC, 8>;
using RE_Pin = Gpio<Pin_Port::PC, 9>;

// 创建RS485实例
using RS485Device = QAQ::base::uart::Uart_IODevice<256, 256>;
QAQ::base::uart::RS485<DMAUartConfig, RS485Device, DE_Pin, RE_Pin> rs485;

void receive_complete(internal::device_internal::IODevice_Base* device) {
    // 接收完成回调
}

// 使用信号处理
rs485.signal_send_complete.connect(receive_complete);
```

## 注意事项

1. **模板特化**：根据不同硬件平台特化配置模板
2. **内存管理**：注意缓冲区大小的合理配置
3. **中断安全**：在中断服务程序中避免阻塞操作
4. **错误处理**：合理处理各种错误码
5. **资源释放**：确保正确关闭设备和释放资源

## 性能优化建议

1. 根据应用需求选择合适的工作模式
2. 合理配置缓冲区大小
3. 优化中断优先级配置
4. 使用DMA模式提高数据传输效率
5. 避免不必要的数据拷贝

---
*文档版本：1.0*
*最后更新：2024*