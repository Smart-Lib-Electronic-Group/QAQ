#define PT_SOURCE_CODE

#include "hardware/inc/pt_dma_api.h"
#include "stm32h7xx_ll_bus.h"

static uint8_t s_pt_dma_channel_used_bitmap[MAX_DMA_CHANNELS / 8];

uint32_t __pt_dma_system_init(void)
{
  PT_DMA_TRACE("Initializing DMA system\n");

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);

  memset(s_pt_dma_channel_used_bitmap, 0, sizeof(s_pt_dma_channel_used_bitmap));

  PT_DMA_TRACE("DMA system initialized with %d channels\n", MAX_DMA_CHANNELS);

  return PT_SUCCEED;
}
