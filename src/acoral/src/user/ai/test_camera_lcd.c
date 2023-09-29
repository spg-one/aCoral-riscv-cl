/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <string.h>
#include "board_config.h"
#include "dvp.h"
#include "fpioa.h"
#include "gpiohs.h"
#include "iomem.h"
#include "lcd.h"
#include "nt35310.h"
#include "plic.h"
#include "sysctl.h"
#include "uarths.h"
#include "unistd.h"
#include "gc0328.h"
#include "w25qxx.h"
#include "dmac.h"
#include "region_layer.h"

uint16_t g_camera_565[320*240] = {0};
static region_layer_t detect_rl;
#define ANCHOR_NUM 5
float g_anchor[ANCHOR_NUM * 2] = {1.08, 1.19, 3.42, 4.41, 6.63, 11.38, 9.42, 5.11, 16.62, 10.52};
kpu_model_context_t task;
uint8_t *model_data_yolo;
#define CLASS_NUMBER 20
extern const uint8_t gImage_image[] __attribute__((aligned(128)));
uint8_t model_input[320*240*3];

volatile uint8_t g_dvp_finish_flag;
volatile uint8_t g_ai_done_flag;

typedef struct
{
    char *str;
    uint16_t color;
    uint16_t height;
    uint16_t width;
    uint32_t *ptr;
} class_lable_t;

class_lable_t class_lable[CLASS_NUMBER] =
{
    {"aeroplane", GREEN},
    {"bicycle", GREEN},
    {"bird", GREEN},
    {"boat", GREEN},
    {"bottle", 0xF81F},
    {"bus", GREEN},
    {"car", GREEN},
    {"cat", GREEN},
    {"chair", 0xFD20},
    {"cow", GREEN},
    {"diningtable", GREEN},
    {"dog", GREEN},
    {"horse", GREEN},
    {"motorbike", GREEN},
    {"person", 0xF800},
    {"pottedplant", GREEN},
    {"sheep", GREEN},
    {"sofa", GREEN},
    {"train", GREEN},
    {"tvmonitor", 0xF9B6}
};

static uint32_t lable_string_draw_ram[115 * 16 * 8 / 2];

static void lable_init(void)
{
#if (CLASS_NUMBER > 1)
    uint8_t index;

    class_lable[0].height = 16;
    class_lable[0].width = 8 * strlen(class_lable[0].str);
    class_lable[0].ptr = lable_string_draw_ram;
    lcd_ram_draw_string(class_lable[0].str, class_lable[0].ptr, BLACK, class_lable[0].color);
    for (index = 1; index < CLASS_NUMBER; index++) {
        class_lable[index].height = 16;
        class_lable[index].width = 8 * strlen(class_lable[index].str);
        class_lable[index].ptr = class_lable[index - 1].ptr + class_lable[index - 1].height * class_lable[index - 1].width / 2;
        lcd_ram_draw_string(class_lable[index].str, class_lable[index].ptr, BLACK, class_lable[index].color);
    }
#endif
}


void rgb888_to_lcd(uint8_t *src, uint16_t *dest, size_t width, size_t height)
{
    size_t i, chn_size = width * height;
    for (size_t i = 0; i < width * height; i++)
    {
        uint8_t r = src[i];
        uint8_t g = src[chn_size + i];
        uint8_t b = src[chn_size * 2 + i];

        uint16_t rgb = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
        size_t d_i = i % 2 ? (i - 1) : (i + 1);
        dest[d_i] = rgb;
    }
}

void rgb565_to_rgb888(uint16_t *src, uint8_t *dest, size_t width, size_t height)
{
    size_t i, chn_size = width * height;
    for (size_t i = 0; i < width * height; i++)
    {
        size_t d_i = i % 2 ? (i - 1) : (i + 1);
        uint8_t r = (src[d_i] >> 8) & 0x00f8;
        uint8_t g = (src[d_i] >> 3) & 0x00fc;
        uint8_t b = (src[d_i] << 3) & 0x00f8;

        dest[i] = r;
        dest[chn_size+i] = g;
        dest[chn_size*2+i] = b;
    }
}




static void drawboxes(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t class, float prob)
{
    if (x1 >= 320)
        x1 = 319;
    if (x2 >= 320)
        x2 = 319;
    if (y1 >= 240)
        y1 = 239;
    if (y2 >= 240)
        y2 = 239;

#if (CLASS_NUMBER > 1)
    lcd_draw_rectangle(x1, y1, x2, y2, 2, class_lable[class].color);
    lcd_draw_picture(x1 + 1, y1 + 1, class_lable[class].width, class_lable[class].height, class_lable[class].ptr);
#else
    lcd_draw_rectangle(x1, y1, x2, y2, 2, RED);
#endif
}


static int ai_done(void *ctx)
{
    g_ai_done_flag = 1;
    return 0;
}



static int on_irq_dvp(void *ctx)
{
    if(dvp_get_interrupt(DVP_STS_FRAME_FINISH))
    {
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
        dvp_clear_interrupt(DVP_STS_FRAME_FINISH);
        g_dvp_finish_flag = 1;
        // lcd_draw_picture(0, 0, 320, 240, g_camera_565);
        
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
        // g_dvp_finish_flag = 0;
    } else
    {
        if(g_dvp_finish_flag == 0)
            dvp_start_convert();
        dvp_clear_interrupt(DVP_STS_FRAME_START);
    }

    return 0;
}

static void io_init(void)
{
    /* Init DVP IO map and function settings */
    fpioa_set_function(DVP_RST_PIN, FUNC_CMOS_RST);
    fpioa_set_function(DVP_PWDN_PIN, FUNC_CMOS_PWDN);
    fpioa_set_function(DVP_XCLK_PIN, FUNC_CMOS_XCLK);
    fpioa_set_function(DVP_VSYNC_PIN, FUNC_CMOS_VSYNC);
    fpioa_set_function(DVP_HREF_PIN, FUNC_CMOS_HREF);
    fpioa_set_function(DVP_PCLK_PIN, FUNC_CMOS_PCLK);
    fpioa_set_function(DVP_SCCB_SCLK_PIN, FUNC_SCCB_SCLK);
    fpioa_set_function(DVP_SCCB_SDA_PIN, FUNC_SCCB_SDA);

    /* Init SPI IO map and function settings */
    fpioa_set_function(LCD_DC_PIN, FUNC_GPIOHS0 + LCD_DC_IO);
    fpioa_set_function(LCD_CS_PIN, FUNC_SPI0_SS3);
    fpioa_set_function(LCD_RW_PIN, FUNC_SPI0_SCLK);
    fpioa_set_function(LCD_RST_PIN, FUNC_GPIOHS0 + LCD_RST_IO);

    sysctl_set_spi0_dvp_data(1);
}

static void io_set_power(void)
{
    /* Set dvp and spi pin to 1.8V */
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}

int test_camera_lcd(void)
{
    /* Set CPU and dvp clk */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 400000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    uarths_init();

    /*SPG 初始化外部中断的优先级、CPU中断阈值，清除所有中断挂起标志*/
    plic_init(); 

    /*初始化DVP和LCD引脚*/
    io_init();
    io_set_power();

    lable_init();
    /* LCD init */
    printf("LCD init\n");
    lcd_init();
    lcd_set_direction(DIR_YX_RLDU);
    lcd_clear(BLUE);

    /* DVP init */
    printf("DVP init\n");
    dvp_init(8); //SPG 这里完成了对dvp接口中像素输出速率时钟pclk和控制信号时钟scl的初始化
    dvp_set_xclk_rate(24000000); //SPG 设置dvp接口的输入基准时钟信号xclk来自于apb总线频率，xclk就是上面scl和pclk的基准
    dvp_enable_burst();
    dvp_set_output_enable(0, 1);
    dvp_set_output_enable(1, 1);
    dvp_set_image_format(DVP_CFG_RGB_FORMAT); //SPG 指定dvp接收到的图像格式为16位的RGB565，这样接收到320*240*16这么多位数据后就可以产生一个中断DVP_STS_FRAME_FINISH表示一帧完成

    dvp_set_image_size(320, 240); //SPG上面320*240的来源

    gc0328_init();
    open_gc0328_0();

    dvp_set_ai_addr((uint32_t)0x40600000, (uint32_t)0x40612C00, (uint32_t)0x40625800);
    dvp_set_display_addr((uint32_t)g_camera_565);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    dvp_disable_auto();

    /* DVP interrupt config */
    printf("DVP interrupt config\n");
    plic_set_priority(IRQN_DVP_INTERRUPT, 1);
    plic_irq_register(IRQN_DVP_INTERRUPT, on_irq_dvp, NULL);
    plic_irq_enable(IRQN_DVP_INTERRUPT);

    /* enable global interrupt */
    sysctl_enable_irq();

    detect_rl.anchor_number = ANCHOR_NUM;
    detect_rl.anchor = g_anchor;
    detect_rl.threshold = 0.5;
    detect_rl.nms_value = 0.2;
    region_layer_init(&detect_rl, 10, 7, 125, 320, 240);

    printf("flash init\n");
    w25qxx_init(3, 0);
    w25qxx_enable_quad_mode();

#define KMODEL_SIZE (1351976)

    model_data_yolo = (uint8_t *)malloc(KMODEL_SIZE);
    w25qxx_read_data(0xA00000, model_data_yolo, KMODEL_SIZE, W25QXX_QUAD_FAST);
    /* system start */
    printf("system start\n");
    if (kpu_load_kmodel(&task, model_data_yolo) != 0)
    {
        printf("\nmodel init error\n");
        while (1);
    }

    float *output;
    size_t output_size;
    g_dvp_finish_flag = 0;
    g_ai_done_flag = 0;
    dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
    // printf("len:%lu\n",strlen(gImage_image));
    // printf("r:%d,g:%d,b:%d",gImage_image[0],gImage_image[320*240],gImage_image[320*240*2]);
    while(1)
    {
        dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
        
        while(g_dvp_finish_flag == 0)
            ;
        g_dvp_finish_flag = 0;
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
        rgb565_to_rgb888(g_camera_565,model_input,320,240);
        kpu_run_kmodel(&task, model_input, DMAC_CHANNEL5, ai_done, NULL);
        while(g_ai_done_flag == 0)
            ;
        /* display pic*/
        kpu_get_output(&task, 0, &output, &output_size);
        detect_rl.input = output;
        output_size /= sizeof(float);

        /* start region layer */
        region_layer_run(&detect_rl, NULL);
        lcd_draw_picture(0, 0, 320, 240, g_camera_565);
        region_layer_draw_boxes(&detect_rl, drawboxes);
        msleep(50);
        g_ai_done_flag = 0;
    }

    // g_ai_done_flag = 0;
    //     /* start to calculate */
    // // rgb888_to_lcd(gImage_image, g_camera_565, 320, 240);
    // // rgb565_to_rgb888(g_camera_565,model_input,320,240);
    // kpu_run_kmodel(&task, gImage_image, DMAC_CHANNEL5, ai_done, NULL);
    // while(!g_ai_done_flag);

    // kpu_get_output(&task, 0, &output, &output_size);
    // detect_rl.input = output;
    // output_size /= sizeof(float);

    // /* start region layer */
    // region_layer_run(&detect_rl, NULL);

    // /* display pic*/
    
    // rgb888_to_lcd(gImage_image, g_camera_565, 320, 240);
    // rgb565_to_rgb888(g_camera_565, gImage_image, 320, 240);
    // rgb888_to_lcd(gImage_image, g_camera_565, 320, 240);
    // lcd_draw_picture(0, 0, 320, 240, g_camera_565);

    /* draw boxs */
    
    // region_layer_draw_boxes(&detect_rl, drawboxes);
    printf("done\n");
    return 0;
}
