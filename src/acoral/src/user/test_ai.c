#include "acoral.h"
#include "w25qxx.h"
#include "fpioa.h"
#include "spi.h"
#include "sysctl.h"
#include "dmac.h"
#include "kpu.h"
#include <assert.h>
#include <stdio.h>

#define KMODEL_SIZE (720)
#define PLL1_OUTPUT_FREQ 400000000UL
uint8_t *model_data;
kpu_model_context_t task1;

int test_ai(){
#ifdef DEBUG_INFO
    printf_debug("in test ai\n");
#endif
    model_data = (uint8_t *)acoral_malloc(KMODEL_SIZE);

    sysctl_pll_set_freq(SYSCTL_PLL1, PLL1_OUTPUT_FREQ);
    uarths_init();

    w25qxx_init(3, 0);
    w25qxx_enable_quad_mode();
    w25qxx_read_data(0xC00000, model_data, KMODEL_SIZE, W25QXX_QUAD_FAST);
#ifdef DEBUG_INFO
    printf_debug("after read ai model\n");
#endif
    if (kpu_load_kmodel(&task1, model_data) != 0)
    {
        printf("Cannot load kmodel.\n");
    }
#ifdef DEBUG_INFO
    printf_debug("out test ai\n");
#endif
}