/**
 * @file disp.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include <string.h>

#include "tft.h"
#include "stm32f4xx.h"
#include "fsmc_ssd1963.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_disp_drv_t disp_drv;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * Initialize your display here
 */
void tft_init(void)
{
  static lv_color_t disp_buf1[TFT_HOR_RES * 40];
  static lv_disp_buf_t buf;
  lv_disp_buf_init(&buf, disp_buf1, NULL, TFT_HOR_RES * 40);

  lv_disp_drv_init(&disp_drv);
  fsmc_ssd1963_init(0, false);

  disp_drv.buffer = &buf;
  disp_drv.flush_cb = fsmc_ssd1963_flush;
  disp_drv.hor_res = TFT_HOR_RES;
  disp_drv.ver_res = TFT_VER_RES;
#if TFT_USE_GPU != 0
  DMA2D_Config();
  disp_drv.gpu_blend_cb = gpu_mem_blend;
  disp_drv.gpu_fill_cb = gpu_mem_fill;
#endif
  lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
