// #include "acoral.h"
// #include "w25qxx.h"
// #include "fpioa.h"
// #include "spi.h"
// #include "sysctl.h"
// #include "dmac.h"
// #include "kpu.h"
// #include <assert.h>
// #include <stdio.h>
// #include <float.h>

// #define KMODEL_SIZE (720)
// #define PLL1_OUTPUT_FREQ 400000000UL
// uint8_t *model_data;
// kpu_model_context_t task1;
// volatile uint32_t g_ai_done_flag;
// const float features[] = {5.1,3.8,1.9,0.4};
// const char *labels[] = { "setosa", "versicolor", "virginica" };

// static void ai_done(void* userdata)
// {
//     g_ai_done_flag = 1;
    
//     float *features;
//     size_t count;
//     kpu_get_output(&task1, 0, (uint8_t **)&features, &count);
//     count /= sizeof(float);

//     size_t i;
//     for (i = 0; i < count; i++)
//     {
//         if (i % 64 == 0)
//             printf("\n");
//         printf("%f, ", features[i]);
//     }

//     printf("\n");
// }

// size_t argmax(const float *src, size_t count)
// {
//     float max = FLT_MIN;
//     size_t max_id = 0, i;
//     for (i = 0; i < count; i++)
//     {
//         if (src[i] > max)
//         {
//             max = src[i];
//             max_id = i;
//         }
//     }

//     return max_id;
// }


// int test_iris(){
// #ifdef DEBUG_INFO
//     printf_debug("in test iris\n");
// #endif
//     model_data = (uint8_t *)acoral_malloc(KMODEL_SIZE);

//     sysctl_pll_set_freq(SYSCTL_PLL1, PLL1_OUTPUT_FREQ); //SPG pll1是给AI用的

//     w25qxx_init(3, 0); //SPG用的SPI3
//     w25qxx_enable_quad_mode();
//     w25qxx_read_data(0xC00000, model_data, KMODEL_SIZE, W25QXX_QUAD_FAST);
// #ifdef DEBUG_INFO
//     printf_debug("after read iris model\n");
// #endif
//     if (kpu_load_kmodel(&task1, model_data) != 0)
//     {
//         printf("Cannot load kmodel.\n");
//         return -1;
//     }

//     int j;
//     for (j = 0; j < 1; j++)
//     {
//         g_ai_done_flag = 0;

//         if (kpu_run_kmodel(&task1, (const uint8_t *)features, 5, ai_done, NULL) != 0)
//         {
//             printf("Cannot run kmodel.\n");
//            return -1;
//         }
// 		while (!g_ai_done_flag);

//         float *output;
//         size_t output_size;
//         kpu_get_output(&task1, 0, (uint8_t **)&output, &output_size);
//         puts(labels[argmax(output, output_size / 4)]);
//     }

// #ifdef DEBUG_INFO
//     printf_debug("out test iris\n");
// #endif
// }