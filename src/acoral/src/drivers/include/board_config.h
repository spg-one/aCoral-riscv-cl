#ifndef _BOARD_CONFIG_
#define _BOARD_CONFIG_

#include <stdio.h>
// pin definitions

/* CAMERA */
#define		DVP_PWDN_PIN				44
#define		DVP_RST_PIN					42
#define		DVP_VSYNC_PIN				43
#define		DVP_HREF_PIN				45
#define		DVP_PCLK_PIN				47
#define		DVP_XCLK_PIN				46
#define		DVP_SCCB_SCLK_PIN			41
#define		DVP_SCCB_SDA_PIN			40	

/* LCD */
#define		LCD_CS_PIN					36
#define		LCD_DC_PIN					38
#define		LCD_RW_PIN					39
#define		LCD_RST_PIN					37

// IO definitions
#define     LCD_DC_IO                   2
#define     LCD_RST_IO                  0
#define		LCD_BLIGHT_IO				17

#endif
