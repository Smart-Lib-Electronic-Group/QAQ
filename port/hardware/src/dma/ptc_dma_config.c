#define PT_SOURCE_CODE

#include "hardware/inc/pt_dma_api.h"

uint32_t __ptc_dma_config(PT_DMA* dma_channel_ptr, uint32_t dma_src_address, uint32_t dma_dst_address, uint32_t dma_memory_size, void (*dma_transferred_callback_function)(PT_DMA* pt_dma_handle, void* arg), void (*dma_error_callback_function)(PT_DMA* pt_dma_handle, void* arg), void* callback_arg) {}