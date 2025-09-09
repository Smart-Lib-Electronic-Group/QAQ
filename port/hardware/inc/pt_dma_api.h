#ifndef __PT_DMA_H__
#define __PT_DMA_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32h7xx.h"

#define MAX_DMA_CHANNELS 16

#define PT_SUCCEED         ((uint32_t)0x00)
#define PT_DMA_NO_CHANNEL  ((uint32_t)0x01)
#define PT_DMA_INVALID_ARG ((uint32_t)0x02)
#define PT_DMA_BUSY        ((uint32_t)0x03)
#define PT_DMA_ERROR       ((uint32_t)0x04)

  typedef enum
  {
    DMA_DIRE_MEM_TO_MEM = 0,
    DMA_DIRE_MEM_TO_DEV = 1,
    DMA_DIRE_DEV_TO_MEM = 2,
    DMA_DIRE_DEV_TO_DEV = 3,
  } pt_dma_direction_e;

  typedef enum
  {
    DMA_DATA_WIDTH_BYTE     = 0,
    DMA_DATA_WIDTH_HALFWORD = 1,
    DMA_DATA_WIDTH_WORD     = 2,
  } pt_dma_data_width_e;

  typedef enum
  {
    DMA_MODE_NORMAL   = 0,
    DMA_MODE_CIRCULAR = 1,
    DMA_MODE_PFCTRL   = 2,
  } pt_dma_mode_e;

  typedef enum
  {
    DMA_PRIO_LOW       = 0,
    DMA_PRIO_MEDIUM    = 1,
    DMA_PRIO_HIGH      = 2,
    DMA_PRIO_VERY_HIGH = 3,
  } pt_dma_priority_e;

  typedef struct PT_DMA_STRUCT
  {
    DMA_TypeDef* pt_dma_ptr;
    uint32_t     pt_dma_stream_number;

    pt_dma_direction_e  pt_dma_direction;
    pt_dma_data_width_e pt_dma_data_width;
    pt_dma_mode_e       pt_dma_mode;
    pt_dma_priority_e   pt_dma_priority;
    uint8_t             pt_dma_src_address_is_auto_increment;
    uint8_t             pt_dma_dst_address_is_auto_increment;

    uint8_t  pt_dma_is_double_buffer_mode;
    uint32_t pt_dma_memory0_address;
    uint32_t pt_dma_memory1_address;
    uint32_t pt_dma_memory_size;
    uint32_t pt_dma_transferred_length;

    void* pt_dma_callback_arg;
    void (*pt_dma_transferred_callback_function)(struct PT_DMA_STRUCT* pt_dma_handle, void* arg);
    void (*pt_dma_memory_switched_callback_function)(struct PT_DMA_STRUCT* pt_dma_handle, void* arg);
    void (*pt_dma_error_callback_function)(struct PT_DMA_STRUCT* pt_dma_handle, void* arg);

#ifdef PT_DMA_ENABLE_PERORMANCE_INFO

    uint32_t pt_dma_total_transfer_count;
    uint32_t pt_dma_total_transfer_size;

#endif
  } PT_DMA;

#ifndef PT_SOURCE_CODE

  #ifdef DMA_API_CHECK

    #define pt_dma_system_init               __pt_dma_system_init
    #define pt_dma_creat                     __ptc_dma_creat
    #define pt_dma_delete                    __ptc_dma_delete
    #define pt_dma_config                    __ptc_dma_config
    #define pt_dma_double_buffer_mode_config __ptc_dma_double_buffer_mode_config
    #define pt_dma_active_memory_get         __ptc_dma_active_memory_get
    #define pt_dma_memory_switch             __ptc_dma_memory_switch
    #define pt_dma_start                     __ptc_dma_start
    #define pt_dma_stop                      __ptc_dma_stop
    #define pt_dma_pause                     __ptc_dma_pause
    #define pt_dma_resume                    __ptc_dma_resume
    #define pt_dma_transferred_length_get    __ptc_dma_transferred_length_get
    #define pt_dma_performance_info_get      __ptc_dma_performance_info_get
    #define pt_dma_irq_handler               __pt_dma_irq_handler
    
  #else

    #define pt_dma_system_init               __pt_dma_system_init
    #define pt_dma_creat                     __pt_dma_creat
    #define pt_dma_delete                    __pt_dma_delete
    #define pt_dma_config                    __pt_dma_config
    #define pt_dma_double_buffer_mode_config __pt_dma_double_buffer_mode_config
    #define pt_dma_active_memory_get         __pt_dma_active_memory_get
    #define pt_dma_memory_switch             __pt_dma_memory_switch
    #define pt_dma_start                     __pt_dma_start
    #define pt_dma_stop                      __pt_dma_stop
    #define pt_dma_pause                     __pt_dma_pause
    #define pt_dma_resume                    __pt_dma_resume
    #define pt_dma_transferred_length_get    __pt_dma_transferred_length_get
    #define pt_dma_performance_info_get      __pt_dma_performance_info_get
    #define pt_dma_irq_handler               __pt_dma_irq_handler

  #endif /* DMA_API_CHECK */

#else

  #include "stm32h7xx_hal.h"

#endif /* PT_SOURCE_CODE */

  uint32_t __pt_dma_system_init(void);
  uint32_t __pt_dma_creat(PT_DMA* dma_channel_ptr, pt_dma_direction_e dma_direction, pt_dma_mode_e dma_mode, uint8_t dma_src_address_is_auto_increment, uint8_t dma_dst_address_is_auto_increment, pt_dma_data_width_e dma_src_data_width, pt_dma_data_width_e dma_dst_data_width, pt_dma_priority_e dma_priority);
  uint32_t __pt_dma_delete(PT_DMA* dma_channel_ptr);
  uint32_t __pt_dma_config(PT_DMA* dma_channel_ptr, uint32_t dma_src_address, uint32_t dma_dst_address, uint32_t dma_memory_size, void (*dma_transferred_callback_function)(PT_DMA* pt_dma_handle, void* arg), void (*dma_error_callback_function)(PT_DMA* pt_dma_handle, void* arg), void* callback_arg);
  uint32_t __pt_dma_double_buffer_mode_config(PT_DMA* dma_channel_ptr, uint32_t dma_memory0_address, uint32_t dma_memory1_address, uint32_t dma_memory_size, void (*dma_memory_switched_callback_function)(PT_DMA* pt_dma_handle, void* arg), void (*dma_error_callback_function)(PT_DMA* pt_dma_handle, void* arg), void* callback_arg);
  uint32_t __pt_dma_active_memory_get(PT_DMA* dma_channel_ptr, uint8_t* dma_active_memory);
  uint32_t __pt_dma_memory_switch(PT_DMA* dma_channel_ptr);
  uint32_t __pt_dma_start(PT_DMA* dma_channel_ptr);
  uint32_t __pt_dma_stop(PT_DMA* dma_channel_ptr);
  uint32_t __pt_dma_pause(PT_DMA* dma_channel_ptr);
  uint32_t __pt_dma_resume(PT_DMA* dma_channel_ptr);
  uint32_t __pt_dma_transferred_length_get(PT_DMA* dma_channel_ptr, uint32_t* dma_transferred_length);
  uint32_t __pt_dma_performance_info_get(PT_DMA* dma_channel_ptr, uint32_t* dma_transferred_length, uint32_t* dma_transferred_time);
  void     __pt_dma_irq_handler(PT_DMA* dma_channel_ptr);

#ifdef DMA_API_CHECK

  uint32_t __ptc_dma_creat(PT_DMA* dma_channel_ptr, pt_dma_direction_e dma_direction, pt_dma_mode_e dma_mode, uint8_t dma_src_address_is_auto_increment, uint8_t dma_dst_address_is_auto_increment, pt_dma_data_width_e dma_src_data_width, pt_dma_data_width_e dma_dst_data_width, pt_dma_priority_e dma_priority);
  uint32_t __ptc_dma_delete(PT_DMA* dma_channel_ptr);
  uint32_t __ptc_dma_config(PT_DMA* dma_channel_ptr, uint32_t dma_src_address, uint32_t dma_dst_address, uint32_t dma_memory_size, void (*dma_transferred_callback_function)(PT_DMA* pt_dma_handle, void* arg), void (*dma_error_callback_function)(PT_DMA* pt_dma_handle, void* arg), void* callback_arg);
  uint32_t __ptc_dma_double_buffer_mode_config(PT_DMA* dma_channel_ptr, uint32_t dma_memory0_address, uint32_t dma_memory1_address, uint32_t dma_memory_size, void (*dma_memory_switched_callback_function)(PT_DMA* pt_dma_handle, void* arg), void (*dma_error_callback_function)(PT_DMA* pt_dma_handle, void* arg), void* callback_arg);
  uint32_t __ptc_dma_active_memory_get(PT_DMA* dma_channel_ptr, uint8_t* dma_active_memory);
  uint32_t __ptc_dma_memory_switch(PT_DMA* dma_channel_ptr);
  uint32_t __ptc_dma_start(PT_DMA* dma_channel_ptr);
  uint32_t __ptc_dma_stop(PT_DMA* dma_channel_ptr);
  uint32_t __ptc_dma_pause(PT_DMA* dma_channel_ptr);
  uint32_t __ptc_dma_resume(PT_DMA* dma_channel_ptr);
  uint32_t __ptc_dma_transferred_length_get(PT_DMA* dma_channel_ptr, uint32_t* dma_transferred_length);
  uint32_t __ptc_dma_performance_info_get(PT_DMA* dma_channel_ptr, uint32_t* dma_transferred_length, uint32_t* dma_transferred_time);

#endif /* DMA_API_CHECK */

#ifdef PT_DMA_ENABLE_TRACING

#else

  #define PT_DMA_TRACE(fmt, ...) ((void)0)

#endif /* PT_DMA_ENABLE_TRACING */

#ifdef __cplusplus
}
#endif

#endif /* __PT_DMA_H__ */
