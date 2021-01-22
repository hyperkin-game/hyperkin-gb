#include "fb.h"

int rk_fb_open(struct fb* fb) {
  fb->fd = open(FB0, O_RDWR);
  if (fb->fd <= 0) {
    printf("open %s failed.\n", FB0);
    return -1;
  }

  if (ioctl(fb->fd, FBIOGET_VSCREENINFO, &fb->vi) < 0) {
    printf("Error:fb read vi info failed.\n");
    return -1;
  }

  //fb->vi.xres;
  //fb->vi.yres;

  return 0;
}

void rk_fb_close(struct fb* fb) {
  if (fb->fd)
    close(fb->fd);
  fb->fd = 0;
}

static unsigned int color_key_565(struct color_key key) {
  key.red &= 0xF8;
  key.green &= 0xFC;
  key.blue &= 0xF8;
  return ((key.enable << 31) | ((key.red & 0xFF) << 22)
          | ((key.green & 0xFF) << 12) | ((key.blue & 0xFF) << 2));
}

int rk_fb_config_color_key(struct fb* fb, struct color_key color_key) {
  struct color_key_cfg key;
  key.win0_color_key_cfg = color_key_565(color_key);
  key.win1_color_key_cfg = 0;
  key.win2_color_key_cfg = 0;
  if (ioctl(fb->fd, RK_FBIOPUT_COLOR_KEY_CFG, (unsigned long)(&key), 0) < 0) {
    printf("RK_FBIOPUT_COLOR_KEY_CFG failed.\n");
    return -1;
  }
  return 0;
}


int rk_fb_config(struct fb* fb, int winId, unsigned short width, unsigned short height,
                 u8 dataFmt, short ionFd, unsigned int phyAddr) {
  int j;
//  printf("ionfd %d \n",ionFd);
  fb->win_cfg.wait_fs = 0;
  fb->win_cfg.win_par[winId].win_id = 0;
  fb->win_cfg.win_par[winId].g_alpha_val = 0;
  fb->win_cfg.win_par[winId].z_order = 0;
  fb->win_cfg.win_par[winId].area_par[winId].acq_fence_fd = -1;
  fb->win_cfg.win_par[winId].area_par[winId].data_format = dataFmt;
  fb->win_cfg.win_par[winId].area_par[winId].ion_fd = ionFd;
  fb->win_cfg.win_par[winId].area_par[winId].phy_addr = phyAddr;
  fb->win_cfg.win_par[winId].area_par[winId].x_offset = 0;
  fb->win_cfg.win_par[winId].area_par[winId].y_offset = 0;
  fb->win_cfg.win_par[winId].area_par[winId].xpos = 0;
  fb->win_cfg.win_par[winId].area_par[winId].ypos = 0;
  fb->win_cfg.win_par[winId].area_par[winId].xsize = width;
  fb->win_cfg.win_par[winId].area_par[winId].ysize = height;
  fb->win_cfg.win_par[winId].area_par[winId].xact = width;
  fb->win_cfg.win_par[winId].area_par[winId].yact = height;
  fb->win_cfg.win_par[winId].area_par[winId].xvir = width;
  fb->win_cfg.win_par[winId].area_par[winId].yvir = height;

//  printf("cfg win %d \n",winId);
  if (ioctl(fb->fd, RK_FBIOSET_CONFIG_DONE,
            (unsigned long)(&fb->win_cfg)) < 0) {
    printf("FBIOSET_CONFIG failed(%s)\n", strerror(errno));
    return -1;
  }

//  printf("cfg win done \n");
  for (j = 0; j < RK_MAX_BUF_NUM; j++)
    if (fb->win_cfg.rel_fence_fd[j] > 0)
      close(fb->win_cfg.rel_fence_fd[j]);

  if (fb->win_cfg.ret_fence_fd > 0)
    close(fb->win_cfg.ret_fence_fd);

  return 0;
}





