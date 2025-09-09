#define PT_SOURCE_CODE

#include "hardware/inc/pt_dma_api.h"

uint32_t __pt_dma_double_buffer_mode_config(PT_DMA* dma_channel_ptr, uint32_t dma_memory0_address, uint32_t dma_memory1_address, uint32_t dma_memory_size, void (*dma_memory_switched_callback_function)(PT_DMA* pt_dma_handle, void* arg), void (*dma_error_callback_function)(PT_DMA* pt_dma_handle, void* arg), void* callback_arg) {}