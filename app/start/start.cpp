#include "start.h"

#include "soft_timer.hpp"
#include "fast_memory.hpp"
#include "event_flags.hpp"

#include "uart.hpp"

#include "arm_math.h"
#include "SEGGER_RTT.h"

#include "net_manager.hpp"
#include "tcp.hpp"

#include "qstring.hpp"
#include "tiered_memory_pool.hpp"

using namespace QAQ::base::gpio;
using namespace QAQ::base::uart;
using namespace QAQ::base::interrupt;
using namespace QAQ::system;
using namespace QAQ::system::signal;
using namespace QAQ::system::thread;
using namespace QAQ::net::net_internal;
using namespace QAQ::net::tcp;
using namespace QAQ::container;

using Led            = Gpio<Pin_Port::PB, 1, Pin_Speed::Low>;
using Relay2         = Gpio<Pin_Port::PE, 0, Pin_Speed::Low>;
using Relay          = Gpio<Pin_Port::PE, 1, Pin_Speed::Low>;
using Key            = Gpio<Pin_Port::PA, 0, Pin_Speed::Low>;

using Uart1_Cfg      = Uart_Config<5, Uart_Type::DMA_Double_Buffer, Uart_Type::DMA>;
using Uart1_IODevice = Uart_IODevice<1024, 1024>;

using DE             = Gpio<Pin_Port::PC, 8>;
using RE             = Gpio<Pin_Port::PC, 9>;
using Uart1          = RS485<Uart1_Cfg, Uart1_IODevice, DE, RE>;

// using Uart2_Cfg      = Uart_Config<4, Uart_Type::Interrupt, Uart_Type::Interrupt>;
// using Uart2_IODevice = Uart_Write_Only_Device<256>;
// using Uart2          = Uart<Uart2_Cfg, Uart2_IODevice>;

using Server_Device  = Tcp_IODevice<2048, 0>;
using Server         = Tcp_Server<4, Server_Device, 2048, 0>;

#define LED_TASK_PRIORITY   5
#define LED_TASK_STACK_SIZE 400
#define LED_TASK_NAME       "LED_Task"
#define LED_TIMER_NAME      "LED_Timer"

Signal<void*, int, float> sig;

struct data_t
{
  int   value;
  float fvalue;
} data;

void key_callback(void* arg)
{
  Relay2::toggle();
  SEGGER_RTT_printf(0, "Time: %ld\n", kernel::System_Clock::now());
}

void key_callback2(void* arg)
{
  sig(nullptr, 1, 1.0f);
}

extern void                                      led_task(int);
Thread<4096, 0>                                  led_th(led_task, 1);
Soft_Timer                                       led_timer(1000, true, key_callback, nullptr);
kernel::Mutex                                    led_mutex;
memory::Memory_Pool<128, sizeof(data_t), data_t> led_pool("led_pool");

uint32_t time_start;
uint32_t time_cnt;

class tim : public Thread<1024, 4, tim>
{
public:
  tim() : Thread() {}

  void handle(void* arg, int, float)
  {
    Led::toggle();
    SEGGER_RTT_printf(0, "Time: %ld\n", kernel::System_Clock::now());
  }

  void run()
  {
    while (1)
    {
      process_signal();
    }
  }
};

tim t;

class idle_task : public Thread<1024, 0, idle_task>
{
public:
  idle_task() : Thread() {}

  void run()
  {
    while (1)
    {
    }
  }
};

idle_task idle;

Uart1 uart1;
// Uart2 uart2;

uint8_t buffer[1500] QAQ_ALIGN(32);

void uart_receive_complete(system_internal::device_internal::IODevice_Base* device)
{
  SEGGER_RTT_printf(0, "uart_receive_complete\n");
}

#include "fx_api.h"
#include "fx_stm32_levelx_nor_driver.h"
#include "lx_api.h"

// #define FX_NOR_QSPI_SECTOR_SIZE    512
// #define FX_NOR_QSPI_NUMBER_OF_FATS 1
// #define FX_NOR_QSPI_HIDDEN_SECTORS 0

// char     FX_NOR_QSPI_VOLUME_NAME[] = "STM32_NOR_QSPI_FLASH_DISK";
// char     path[128];
// FX_MEDIA qspi_media;
// ALIGN_32BYTES(uint32_t fx_nor_qspi_media_memory[FX_NOR_QSPI_SECTOR_SIZE / sizeof(uint32_t)]);

uint16_t ret = 0;
Server   server;

void connected(Server::Device* client)
{
  QAQ_INFO_INFO();
}

void received(Server::Device* client)
{
  uint32_t count = client->available();
  QString  str;
  if (count > 0)
  {
    count = client->read(buffer, count);
    str   = QString(reinterpret_cast<char*>(buffer), count);
  }

  if (count > 0)
  {
    str << ", received count: " << count;
    client->write(str.c_str(), str.length());
  }
}

void disconnected(Server::Device* client)
{
  QAQ_INFO_LOG("disconnected\n");
}

void log_output(const char* str, uint32_t size)
{
  SEGGER_RTT_printf(0, "%s", str);
}

void led_task(int)
{
  System_Monitor::set_output_func(log_output);

  // ret = fx_media_format(&qspi_media, fx_stm32_levelx_nor_driver, (VOID*)LX_NOR_QSPI_DRIVER_ID, (UCHAR*)fx_nor_qspi_media_memory, sizeof(fx_nor_qspi_media_memory), FX_NOR_QSPI_VOLUME_NAME, FX_NOR_QSPI_NUMBER_OF_FATS, 32, FX_NOR_QSPI_HIDDEN_SECTORS, ((LX_STM32_QSPI_FLASH_SIZE - LX_STM32_QSPI_SECTOR_SIZE) / FX_NOR_QSPI_SECTOR_SIZE), FX_NOR_QSPI_SECTOR_SIZE, 8, 1, 1);

  // ret = fx_media_open(&qspi_media, FX_NOR_QSPI_VOLUME_NAME, fx_stm32_levelx_nor_driver, (VOID*)LX_NOR_QSPI_DRIVER_ID, (VOID*)fx_nor_qspi_media_memory, sizeof(fx_nor_qspi_media_memory));

  // sprintf(path, "/");
  // ret = fx_directory_create(&qspi_media, path);

  {
    Led::setup(Pin_Mode::Output);
    Relay::setup(Pin_Mode::Output);
    Relay2::setup(Pin_Mode::Output);
    Key::setup(Pin_Mode::Input);

    // Key::interrupt_add(Pin_Edge::Both, key_callback2, key_callback, nullptr);
    Key::interrupt_add(Interrupt_Type::Queue, Pin_Edge::Both, key_callback2, nullptr);

    uart1.open();
    // uart2.open();

    uart1.config(Config::Baud_Rate, 2000000);
    // uart1.config().baud_rate(2000000).parity(Uart_Parity::None).data_bits(8).stop_bits(1);

    uart1.signal_receive_complete.connect(uart_receive_complete);

    sig.connect(&t, &tim::handle);
  }

  server.start("tcp", 5050, 4);

  server.signal_client_connected.connect(connected);
  server.signal_client_received.connect(received);
  server.signal_client_disconnected.connect(disconnected);

  while (1)
  {
    while (server.get_opened_client_count() == 0)
    {
      Led::toggle();
      sleep(100);
    }

    // Server::Client& client = server.get_first_opened_client();

    // while (client.is_opened())
    // {
    //   client.write(buffer, 1500);
    // }

    Led::toggle();
    sleep(500);
  }
}

void start()
{
  Net_Manager::instance().init();
  Net_Manager::instance().enable_udp();
  Net_Manager::instance().enable_tcp();

  lx_nor_flash_initialize();
  fx_system_initialize();

  // idle.create("idle_task", TX_MAX_PRIORITIES - 1);
  // idle.start();
  led_th.create(LED_TASK_NAME, 5);
  led_th.start();
  t.create(LED_TIMER_NAME, LED_TASK_PRIORITY);
  t.start();
}