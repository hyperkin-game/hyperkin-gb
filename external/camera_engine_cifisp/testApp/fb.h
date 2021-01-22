#ifndef __CVR_FB_H__
#define __CVR_FB_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <getopt.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
//#include <cutils/uevent.h>
//#include <cutils/properties.h>
#include <asm/types.h>
#include <linux/videodev2.h>


#define FB0 "/dev/fb0"

#define HAL_PIXEL_FORMAT_YCbCr_422_SP 0x10  /* NV16 */
#define HAL_PIXEL_FORMAT_YCrCb_420_SP 0x11  /* NV21 */
#define HAL_PIXEL_FORMAT_YCrCb_NV12 0x20  /* YUY2 */
#define HAL_PIXEL_FORMAT_YCrCb_444  0x25  /* yuv444 */

#define HAL_PIXEL_FORMAT_RGB_888  3
#define HAL_PIXEL_FORMAT_RGB_565  4
#define HAL_PIXEL_FORMAT_BGRA_8888  5

#define RK_MAX_BUF_NUM          11
#define RK30_MAX_LAYER_SUPPORT  5
#define RK_WIN_MAX_AREA         4

#define RK_FBIOSET_CONFIG_DONE    0x4628
#define RK_FBIOPUT_COLOR_KEY_CFG  0x4626


typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

struct rk_fb_area_par {
  u8  data_format;        /*layer data fmt*/
  short ion_fd;
  u32 phy_addr;
  short acq_fence_fd;
  u16  x_offset;
  u16  y_offset;
  u16 xpos;       /*start point in panel  --->LCDC_WINx_DSP_ST*/
  u16 ypos;
  u16 xsize;      /* display window width/height  -->LCDC_WINx_DSP_INFO*/
  u16 ysize;
  u16 xact;       /*origin display window size -->LCDC_WINx_ACT_INFO*/
  u16 yact;
  u16 xvir;       /*virtual width/height     -->LCDC_WINx_VIR*/
  u16 yvir;
  u8  fbdc_en;
  u8  fbdc_cor_en;
  u8  fbdc_data_format;
  u16 reserved0;
  u32 reserved1;
};

struct rk_fb_win_par {
  u8  win_id;
  u8  z_order;            /*win sel layer*/
  u8  alpha_mode;
  u16 g_alpha_val;
  u8  mirror_en;
  struct rk_fb_area_par area_par[RK_WIN_MAX_AREA];
  u32 reserved0;
};

struct rk_fb_wb_cfg {
  u8  data_format;
  short ion_fd;
  u32 phy_addr;
  u16 xsize;
  u16 ysize;
  u8 reserved0;
  u32 reversed1;
};

struct rk_fb_win_cfg_data {
  u8  wait_fs;
  short ret_fence_fd;
  short rel_fence_fd[RK_MAX_BUF_NUM];
  struct  rk_fb_win_par win_par[RK30_MAX_LAYER_SUPPORT];
  struct  rk_fb_wb_cfg wb_cfg;
};

struct color_key_cfg {
  u32 win0_color_key_cfg;
  u32 win1_color_key_cfg;
  u32 win2_color_key_cfg;
};

struct color_key {
  unsigned int red;
  unsigned int green;
  unsigned int blue;
  unsigned char enable;
};

struct fb {
  int fd;
  struct fb_var_screeninfo vi;
  struct fb_fix_screeninfo fi;
  struct rk_fb_win_cfg_data win_cfg;
};



int rk_fb_open(struct fb* fb);

void rk_fb_close(struct fb* fb);

int rk_fb_config_color_key(struct fb* fb, struct color_key color_key);

int rk_fb_config(struct fb* fb, int winId, unsigned short width, unsigned short height,
                 u8 dataFmt, short ionFd, unsigned int phyAddr);

#ifdef __cplusplus
}
#endif

#endif

