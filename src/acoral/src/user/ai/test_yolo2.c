/* Copyright 2023 SPG.
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
#include "bsp.h"
#include "kernel.h"

#define ANCHOR_NUM 5 //anchor锚框的数量
#define CLASS_NUMBER 20 //yolo2模型可以识别20类物体
/*模型输入张量为320*240*3，输出张量为7*10*125，其中7*10表示将原始图像分为7*10个grid cell，125=5*(5+20），第一个5表示5个锚框，第二个5表示每个锚框的长宽、中心点坐标和置信度，20表示每个锚框预测20类物体*/
#define KMODEL_SIZE (1351976)

static region_layer_t detect_rl;

kpu_model_context_t task;
uint8_t *model_data_yolo;
volatile uint8_t g_dvp_finish_flag = 0; //摄像头采集完一帧，发中断，置1
volatile uint8_t g_ai_done_flag = 0; //模型跑完置1

uint8_t model_input[320*240*3] = {0}; //yolo2模型输入图像为rgb888格式
uint16_t g_camera_565[320*240] = {0}; //gc0328获得的图像为RGB565格式
float g_anchor[ANCHOR_NUM * 2] = {1.08, 1.19, 3.42, 4.41, 6.63, 11.38, 9.42, 5.11, 16.62, 10.52}; //锚框长宽
static uint32_t lable_string_draw_ram[115 * 16 * 8 / 2];

typedef struct
{
    char *str;
    uint16_t color;
    uint16_t height;
    uint16_t width;
    uint32_t *ptr;
} class_lable_t;

/* class名字与对应的颜色，颜色为RGB565格式 */
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

/**
 * @brief rgb888转rgb565
 * 
 * @param src 待转换像素
 * @param dest 目标像素输出地址
 * @param width 像素（图片）宽度
 * @param height 像素（图片）高度
 */
static void rgb888_to_lcd(uint8_t *src, uint16_t *dest, size_t width, size_t height)
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

/**
 * @brief rgb565转rgb888
 * 
 * @param src 待转换像素
 * @param dest 目标像素输出地址
 * @param width 像素（图片）宽度
 * @param height 像素（图片）高度
 */
static void rgb565_to_rgb888(uint16_t *src, uint8_t *dest, size_t width, size_t height)
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

/**
 * @brief dvp接口开始或完成接收一帧图像
 * 
 * @param ctx 
 * @return int 
 */
static int on_irq_dvp(void *ctx)
{   /* 完成一帧图像接收 */
    if(dvp_get_interrupt(DVP_STS_FRAME_FINISH)) 
    {
        /* 关中断 */
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
        /* 清中断 */
        dvp_clear_interrupt(DVP_STS_FRAME_FINISH);
        g_dvp_finish_flag = 1;

        /* 开中断 */
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
    } else
    {   /* 开始一帧图像接收 */
        if(g_dvp_finish_flag == 0)
            dvp_start_convert();
        /* 清中断 */
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

/* 运行yolo2 */
int test_yolo2(void)
{
    float *output;
    size_t output_size;

    /* Set CPU and dvp clk */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 400000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    uarths_init();

    /* 初始化外部中断的优先级、CPU中断阈值，清除所有中断挂起标志 */
    plic_init(); 

    /*初始化DVP和LCD引脚*/
    io_init();
    io_set_power();

    /* LCD init */
    ACORAL_LOG_TRACE("YOLO2 LCD Init\n");
    lcd_init();
    lcd_set_direction(DIR_YX_RLDU);
    lcd_clear(BLUE);

    /* DVP init */
    ACORAL_LOG_TRACE("YOLO2 DVP Init\n");
    dvp_init(8); //SPG 这里完成了对dvp接口中像素输出速率时钟pclk和控制信号时钟scl的初始化
    dvp_set_xclk_rate(24000000); //SPG 设置dvp接口的输入基准时钟信号xclk来自于apb总线频率，xclk就是上面scl和pclk的基准
    dvp_enable_burst();
    dvp_set_output_enable(0, 1);
    dvp_set_output_enable(1, 1);
    dvp_set_image_format(DVP_CFG_RGB_FORMAT); //SPG 指定dvp接收到的图像格式为16位的RGB565，这样接收到320*240*16这么多位数据后就可以产生一个中断DVP_STS_FRAME_FINISH表示一帧完成
    dvp_set_image_size(320, 240); //SPG上面320*240的来源
    dvp_set_ai_addr((uint32_t)0x40600000, (uint32_t)0x40612C00, (uint32_t)0x40625800);
    dvp_set_display_addr((uint32_t)g_camera_565);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    dvp_disable_auto();

    /* DVP interrupt config */
    ACORAL_LOG_TRACE("YOLO2 DVP Interrupt Config\n");
    plic_set_priority(IRQN_DVP_INTERRUPT, 1);
    plic_irq_register(IRQN_DVP_INTERRUPT, on_irq_dvp, NULL);
    plic_irq_enable(IRQN_DVP_INTERRUPT);

    /* Camera init */
    gc0328_init(); //初始化摄像头
    open_gc0328(); //打开摄像头
    
    /* 初始化画框参数 */
    detect_rl.anchor_number = ANCHOR_NUM;
    detect_rl.anchor = g_anchor;
    detect_rl.threshold = 0.5; //判断框内是否有某一类物体的阈值
    detect_rl.nms_value = 0.2; //极大值抑制交并比IOU阈值，用来去除重复的框
    region_layer_init(&detect_rl, 10, 7, 125, 320, 240); //将图像分割为7*10个grid cell，每个grid cell包含5*（5+20）个数据，原始图像为320*240
    lable_init();

    /* flash init */
    ACORAL_LOG_TRACE("Flash Init Start\n");
    w25qxx_init(3, 0);
    w25qxx_enable_quad_mode();

    /* 加载模型 */
    model_data_yolo = (uint8_t *)malloc(KMODEL_SIZE);
    w25qxx_read_data(0xA00000, model_data_yolo, KMODEL_SIZE, W25QXX_QUAD_FAST);
    
    /* 解析模型 */
    if (kpu_load_kmodel(&task, model_data_yolo) != 0)
    {
        ACORAL_LOG_ERROR("model init error\n");
        while (1);
    }
   
    /* enable global interrupt */
    sysctl_enable_irq();
    
    /* system start */
    ACORAL_LOG_TRACE("YOLO2 System Start\n");
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
        kpu_get_output(&task, 0, (uint8_t**)&output, (size_t*)&output_size);
        detect_rl.input = output;
        output_size /= sizeof(float);

        /* start region layer */
        region_layer_run(&detect_rl, NULL);
        lcd_draw_picture(0, 0, 320, 240, (uint32_t*)g_camera_565);
        region_layer_draw_boxes(&detect_rl, drawboxes);
        msleep(50);
        g_ai_done_flag = 0;
    }
    return 0;
}
