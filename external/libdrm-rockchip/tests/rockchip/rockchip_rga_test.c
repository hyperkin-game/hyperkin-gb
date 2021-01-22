/*
 * Copyright (C) 2013 Samsung Electronics Co.Ltd
 * Authors:
 *	Yakir Yang <ykk@rock-chips.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

//#define DEST_RGB_DISP		1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <sys/mman.h>
#include <linux/stddef.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libkms.h>
#include <drm_fourcc.h>

#include "rockchip_drm.h"
#include "rockchip_drmif.h"
#include "rockchip_rga.h"

#define DRM_MODULE_NAME		"rockchip"
#define MAX_TEST_CASE		1

struct rga_context *ctx;

struct connector {
	uint32_t id;
	char mode_str[64];
	char format_str[5];
	unsigned int fourcc;
	drmModeModeInfo *mode;
	drmModeEncoder *encoder;
	int crtc;
	int pipe;
	int plane_zpos;
	unsigned int fb_id[2], current_fb_id;
	struct timeval start;

	int swap_count;
};

struct rga_test {
	struct rockchip_device *dev;
	struct rockchip_bo *dst_bo;
	struct rockchip_bo *src_bo;
	struct connector src_con;
	struct connector dst_con;

	struct rga_image src_img;
	struct rga_image dst_img;
};

static void connector_find_mode(int fd, struct connector *c, drmModeRes *resources)
{
	drmModeConnector *connector;
	int i, j;

	/* First, find the connector & mode */
	c->mode = NULL;

	for (i = 0; i < resources->count_connectors; i++) {
		connector = drmModeGetConnector(fd, resources->connectors[i]);

		if (!connector) {
			fprintf(stderr, "could not get connector %i: %s\n",
				resources->connectors[i], strerror(errno));
			drmModeFreeConnector(connector);
			continue;
		}

		if (!connector->count_modes) {
			drmModeFreeConnector(connector);
			continue;
		}

		if (connector->connector_id != c->id) {
			drmModeFreeConnector(connector);
			continue;
		}

		for (j = 0; j < connector->count_modes; j++) {
			c->mode = &connector->modes[j];
			if (!strcmp(c->mode->name, c->mode_str))
				break;
		}

		/* Found it, break out */
		if (c->mode)
			break;

		drmModeFreeConnector(connector);
	}

	if (!c->mode) {
		fprintf(stderr, "failed to find mode \"%s\"\n", c->mode_str);
		return;
	}

	/* Now get the encoder */
	for (i = 0; i < resources->count_encoders; i++) {
		c->encoder = drmModeGetEncoder(fd, resources->encoders[i]);

		if (!c->encoder) {
			fprintf(stderr, "could not get encoder %i: %s\n",
				resources->encoders[i], strerror(errno));
			drmModeFreeEncoder(c->encoder);
			continue;
		}

		if (c->encoder->encoder_id  == connector->encoder_id)
			break;

		drmModeFreeEncoder(c->encoder);
	}

	if (c->crtc == -1)
		c->crtc = c->encoder->crtc_id;
}

static int drm_set_crtc(struct rockchip_device *dev, struct connector *c,
			unsigned int fb_id)
{
	int ret;

	ret = drmModeSetCrtc(dev->fd, c->crtc, fb_id, 0, 0, &c->id, 1, c->mode);
	if (ret) {
		printf("failed to set mode: %s\n", strerror(errno));
		goto err;
	}

	return 0;

err:
	return ret;
}

static struct rockchip_bo *rockchip_create_buffer(struct rockchip_device *dev,
						  unsigned long size,
						  unsigned int flags)
{
	struct rockchip_bo *bo;

	bo = rockchip_bo_create(dev, size, flags);
	if (!bo)
		return bo;

	if (!rockchip_bo_map(bo)) {
		rockchip_bo_destroy(bo);
		return NULL;
	}

	return bo;
}

static void rockchip_destroy_buffer(struct rockchip_bo *bo)
{
	rockchip_bo_destroy(bo);
}


static int rga_context_test(struct rga_test *test)
{
	struct rockchip_device *dev = test->dev;
	struct rga_image test1 = test->src_img;
	struct rga_image test2 = test->dst_img;
	uint8_t * _src = test->src_bo->vaddr;
	uint8_t * _dst = test->dst_bo->vaddr;
	unsigned int img_w, img_h;
	unsigned int i, j;
	int ret;

	img_w = 720;
	img_h = 306;

	test1.width = img_w;
	test1.height = img_h;
	test1.stride = test1.width;
	test1.color_mode = DRM_FORMAT_NV12;

	test2.width = img_h;
	test2.height = img_w;
	test2.stride = test2.width;
	test2.color_mode = DRM_FORMAT_NV12;

	test1.fill_color = 0xAA;
	ret = rga_solid_fill(ctx, &test1, 0, 0, test1.width, test1.height);
	ret = rga_exec(ctx);
	if (ret)
		return ret;

	test2.fill_color = 0x00;
	ret = rga_solid_fill(ctx, &test2, 0, 0, test2.width, test2.height);
	ret = rga_exec(ctx);
	if (ret)
		return ret;

	rga_multiple_transform(ctx, &test1, &test2, 0, 0, test1.width, test1.height,
			       0, 0, test2.width, test2.height, 90, 0, 0);
	ret = rga_exec(ctx);
	if (ret < 0)
		return ret;

	for (i = 0; i < img_w; i++) {
		for (j = 0; j < img_h; j++) {
			if (_src[j * img_w + i] != _dst[j * img_w + i]) {
				printf("*[RGA ERROR]* : src (%d, %d) [%x]  !=  "
				       "dst (%d, %d) [%x]\n", i, j, _src[j * img_w + i],
				       j, i, _dst[j* img_w + i]);
				return -1;
			}
		}
	}
	
	return 0;
}

static int rga_rot_scale_test(struct rga_test *test)
{
	struct rga_image *src = &test->src_img;
	struct rga_image *dst = &test->dst_img;
	struct rga_image test_img = *src;
	unsigned int i, j;
	int ret;

	printf("-------- Fill source buffer pattern\n");
	src->fill_color = 0x0;
	ret = rga_solid_fill(ctx, src, 0, 0, src->width, src->height);
	ret = rga_exec(ctx);
	if (ret)
		return ret;

	src->fill_color = 0xff00;
	rga_solid_fill(ctx, src, 5, 5, 500, 100);
	src->fill_color = 0xff;
	rga_solid_fill(ctx, src, 5, 105, 500, 100);
	src->fill_color = 0xff0000;
	rga_solid_fill(ctx, src, 5, 205, 500, 100);
	src->fill_color = 0xffffffff;
	rga_solid_fill(ctx, src, 50, 5, 50, 400);
	ret = rga_exec(ctx);
	if (ret)
		return ret;

	dst->fill_color = 0x0;
	ret = rga_solid_fill(ctx, dst, 0, 0, dst->width, dst->height);
	ret = rga_exec(ctx);
	if (ret)
		return ret;

	for (i = 100; i < dst->width; i++) {
		for (j = 100; j < dst->height; j+=1) {
			printf("------- 0 degree (500, 400) --> (%d, %d)\n", i, j);
			rga_multiple_transform(ctx, src, dst, 0, 0, 500, 400,
					0, 0, i, j, 0, 0, 0);
			ret = rga_exec(ctx);
			if (ret < 0)
				return ret;

#if 0
			/*
			 * RGA API Related:
			 *
			 * This code would SCALING and RORATE 90 Degree the source
			 * framebuffer, and place the output to dest framebuffer,
			 * and the window size is:
			 */
			printf("------- 90 degree (500, 400) --> (%d, %d)\n", i, j);
			rga_multiple_transform(ctx, src, dst, 0, 0, 500, 400,
					0, 0, i, j, 90, 0, 0);
			ret = rga_exec(ctx);
			if (ret < 0)
				return ret;

			printf("------- 180 degree (500, 400) --> (%d, %d)\n", i, j);
			rga_multiple_transform(ctx, src, dst, 0, 0, 500, 400,
					0, 0, i, j, 180, 0, 0);
			ret = rga_exec(ctx);
			if (ret < 0)
				return ret;

			printf("------- 270 degree (500, 400) --> (%d, %d)\n", i, j);
			rga_multiple_transform(ctx, src, dst, 0, 0, 500, 400,
					0, 0, i, j, 270, 0, 0);
			ret = rga_exec(ctx);
			if (ret < 0)
				return ret;
#endif
		}
	}

	return 0;
}

static int rga_cmdlist_test(struct rga_test *test)
{
	struct rga_image *dst = &test->dst_img;
	int ret;

	while (1) {
		dst->fill_color = 0x0;
		rga_solid_fill(ctx, dst, 0, 0, dst->width, dst->height);
		ret = rga_exec(ctx);
		if (ret)
			return ret;

		dst->fill_color = 0xff00;
		rga_solid_fill(ctx, dst, 5, 5, 500, 100);
		dst->fill_color = 0xff;
		rga_solid_fill(ctx, dst, 5, 105, 500, 100);
		dst->fill_color = 0xff0000;
		rga_solid_fill(ctx, dst, 5, 205, 500, 100);
		dst->fill_color = 0xffffffff;
		rga_solid_fill(ctx, dst, 50, 5, 50, 400);
		ret = rga_exec(ctx);
		if (ret)
			return ret;

		getchar();
	}

	return 0;
}

static int rga_color_fill_test(struct rga_test *test)
{
	int ret;
	struct rga_image *dst = &test->dst_img;

	/*
	 * RGA API Related:
	 *
	 * Initialize the source framebuffer and dest framebuffer with BLACK color.
	 *
	 * The "->fill_color" variable is corresponding to RGA target color, and it's
	 * ARGB8888 format, like if you want the source framebuffer filled with
	 * RED COLOR, then you should fill the "->fill_color" with 0x00ff0000.
	 */
	dst->fill_color = 0xff0000ff;
	rga_solid_fill(ctx, dst, 0, 0, dst->width, dst->height);
	ret = rga_exec(ctx);
	if (ret)
		return ret;
	sleep(3);

	dst->fill_color = 0xff00ff00;
	rga_solid_fill(ctx, dst, 0, 0, dst->width, dst->height);
	ret = rga_exec(ctx);
	if (ret)
		return ret;
	sleep(3);

	dst->fill_color = 0xffff0000;
	rga_solid_fill(ctx, dst, 0, 0, dst->width, dst->height);
	ret = rga_exec(ctx);
	if (ret)
		return ret;
	sleep(3);

	return 0;
}

static int rga_csc_test(struct rockchip_device *dev, struct rga_image *src, struct rga_image *dst)
{
	struct rga_image test = *src;
	int ret;

	test.width = src->width;
	test.height = src->height;
	test.stride = test.width * 4;
	test.color_mode = DRM_FORMAT_ARGB8888;

	test.fill_color = 0xffffffff;
	rga_solid_fill(ctx, &test, 0, 0, test.width, test.height);
	ret = rga_exec(ctx);
	if (ret)
		return ret;
	sleep(1);

	ret = rga_multiple_transform(ctx, &test, dst, 0, 0, test.width, test.height,
			0, 0, dst->width, dst->height, 0, 0, 0);
	ret = rga_exec(ctx);
	if (ret)
		return ret;

	return 0;
}

static int rga_test(struct rga_test *test)
{
	int ret;

	ret = rga_cmdlist_test(test);
	if (ret) {
		printf("*[RGA ERROR]*: Failed at cmdlist test\n");
		return ret;
	}

	ret = rga_color_fill_test(test);
	if (ret) {
		printf("*[RGA ERROR]*: Failed at color fill test\n");
		return ret;
	}

	/*
	ret = rga_context_test(test);
	if (ret) {
		printf("*[RGA ERROR]*: Failed at context test\n");
		return ret;
	}
	*/

	ret = rga_rot_scale_test(test);
	if (ret) {
		printf("*[RGA ERROR]*: Failed at rotate / scale test\n");
		return ret;
	}

	return 0;
}

static int rga_copy_nv12_to_nv12_test(struct rga_test *test, enum e_rga_buf_type type)
{
	struct rockchip_device *dev = test->dev;
	struct rockchip_bo *src = test->src_bo;
	struct rockchip_bo *dst = test->dst_bo;
	struct connector *src_con = &test->src_con;
	struct connector *dst_con = &test->dst_con;
	struct rga_image src_img = {0}, dst_img = {0};
	unsigned int img_w, img_h;
	int dst_fd, src_fd;

	/*
	 * RGA API Related:
	 *
	 * Due to RGA API only accept the fd of dma_buf, so we need
	 * to conver the dma_buf Handle to dma_buf FD.
	 *
	 * And then just assigned the src/dst framebuffer FD to the
	 * "struct rga_img".
	 *
	 * And for now, RGA driver only support GEM buffer type, so
	 * we also need to assign the src/dst buffer type to RGA_IMGBUF_GEM.
	 *
	 * For futher, I would try to add user point support.
	 */
	drmPrimeHandleToFD(dev->fd, dst->handle, 0 , &dst_fd);
	drmPrimeHandleToFD(dev->fd, src->handle, 0 , &src_fd);

	dst_img.bo[0] = dst_fd;
	src_img.bo[0] = src_fd;

	/*
	 * RGA API Related:
	 *
	 * Configure the source FB width / height / stride / color_mode.
	 * 
	 * The width / height is correspond to the framebuffer width /height
	 *
	 * The stride is equal to (width * pixel_width).
	 *
	 * The color_mode should configure to the standard DRM color format
	 * which defined in "/user/include/drm/drm_fourcc.h"
	 *
	 */
	img_w = src_con->mode->hdisplay;
	img_h = src_con->mode->vdisplay;

	src_img.width = img_w;
	src_img.height = img_h;
	src_img.stride = img_w * 4;
	src_img.buf_type = type;
	src_img.color_mode = DRM_FORMAT_ARGB8888;

	img_w = dst_con->mode->hdisplay;
	img_h = dst_con->mode->vdisplay;

	dst_img.width = img_w;
	dst_img.height = img_h;
	dst_img.buf_type = type;
#ifdef DEST_RGB_DISP
	dst_img.stride = img_w * 4;
	dst_img.color_mode = DRM_FORMAT_ARGB8888;
#else
	dst_img.stride = img_w;
	dst_img.color_mode = DRM_FORMAT_NV21;
#endif

	/*
	 * RGA Tested Related:
	 *
	 * Start to run test between source FB and dest FB
	 */

	test->dst_img = dst_img;
	test->src_img = src_img;

	rga_test(test);

	close(src_fd);
	close(dst_fd);

	return 0;
}

static struct rockchip_bo *init_crtc(struct connector *con,
				     struct rockchip_device *dev)
{
	struct rockchip_bo *bo;
	unsigned int screen_width, screen_height;
	drmModeRes *resources;

	resources = drmModeGetResources(dev->fd);
	if (!resources) {
		fprintf(stderr, "drmModeGetResources failed: %s\n",
				strerror(errno));
		return NULL;
	}

	connector_find_mode(dev->fd, con, resources);
	drmModeFreeResources(resources);
	if (!con->mode) {
		fprintf(stderr, "failed to find usable connector\n");
		return NULL;
	}

	screen_width = con->mode->hdisplay;
	screen_height = con->mode->vdisplay;

	if (screen_width == 0 || screen_height == 0) {
		fprintf(stderr, "failed to find sane resolution on connector\n");
		return NULL;
	}

	printf("screen width = %d, screen height = %d\n", screen_width, screen_height);

	bo = rockchip_create_buffer(dev, screen_width * screen_height * 4, 0);
	if (!bo) {
		return NULL;
	}

	con->plane_zpos = -1;

	return bo;
}

static int rga_nv12_to_nv12_test(struct rga_test *test)
{
	struct rockchip_device *dev = test->dev;
	struct rockchip_bo *dst_bo = test->dst_bo;
	struct connector *dst_con = &test->dst_con;
	uint32_t handles[4] = {0}, pitches[4] = {0}, offsets[4] = {0};
	unsigned int dst_fb_id;
	int ret, modes;

	/*
	 * Dest FB Displayed Related:
	 *
	 * Add the dest framebuffer to DRM connector, note that for NV12
	 * display, the virtual stride is (width), that's why pitches[0]
	 * is hdisplay.
	 */
#ifdef DEST_RGB_DISP
	modes = DRM_FORMAT_XRGB8888;
	pitches[0] = dst_con->mode->hdisplay * 4;
#else
	modes = DRM_FORMAT_NV12;
	pitches[0] = dst_con->mode->hdisplay;
        handles[1] = dst_bo->handle;
        pitches[1] = dst_con->mode->hdisplay;
        offsets[1] = dst_con->mode->hdisplay * dst_con->mode->vdisplay;
#endif

	handles[0] = dst_bo->handle;
	offsets[0] = 0;
	ret = drmModeAddFB2(dev->fd, dst_con->mode->hdisplay, dst_con->mode->vdisplay,
			    modes, handles, pitches, offsets, &dst_fb_id, 0);
	if (ret < 0)
		return -EFAULT;

	ret = drm_set_crtc(dev, dst_con, dst_fb_id);
	if (ret < 0)
		return -EFAULT;

	/*
	 * TEST RGA Related:
	 *
	 * Start to configure the RGA module and run test
	 */
	ret = rga_copy_nv12_to_nv12_test(test, RGA_IMGBUF_GEM);
	if (ret < 0) {
		fprintf(stderr, "failed to test copy operation.\n");
		return -EFAULT;
	}

	/*
	 * Display Related:
	 *
	 * Released the display framebufffer refer which hold
	 * by DRM display framework
	 */
	drmModeRmFB(dev->fd, dst_fb_id);

	return 0;
}

int main(int argc, char **argv)
{
	struct rockchip_device *dev;
	struct connector src_con, dst_con;
	struct rga_test test = {0};
	int fd;

	fd = drmOpen(DRM_MODULE_NAME, NULL);
	if (fd < 0) {
		fprintf(stderr, "failed to open.\n");
		return fd;
	}

	dev = rockchip_device_create(fd);
	if (!dev) {
		drmClose(dev->fd);
		return -EFAULT;
	}

	/*
	 * RGA API Related:
	 *
	 * Open the RGA device
	 */
	ctx = rga_init(dev->fd);
	if (!ctx)
		return -EFAULT;


	/*
	 * Test Display Related:
	 *
	 * Source framebuffer display connector init. Just a
	 * hack. Directly use the eDP monitor, and force to
	 * use the 1920x1080 display mode.
	 */
	memset(&src_con, 0, sizeof(struct connector));
	src_con.crtc = -1;
	src_con.id = 33;
	src_con.mode = alloca(sizeof(drmModeModeInfo));
	src_con.mode->hdisplay = 720;
	src_con.mode->vdisplay = 720;
	src_con.plane_zpos = -1;

	test.src_bo = rockchip_create_buffer(dev, src_con.mode->hdisplay * src_con.mode->vdisplay * 4, 0);
	if (!test.src_bo) {
		fprintf(stderr, "Failed to create source fb!\n");
		return -EFAULT;
	}


	/*
	 * Test Display Related:
	 *
	 * Dest framebuffer display connector init. Just a
	 * hack. Directly use the eDP monitor, and force to
	 * use the 1280x800 display mode.
	 */
	dst_con.crtc = -1;
	dst_con.id = 29;
	strcpy(dst_con.mode_str, "2560x1600");
	strcpy(dst_con.mode_str, "2048x1536");

	dst_con.id = 27;
	strcpy(dst_con.mode_str, "1200x1920");

	dst_con.id = 30;
	strcpy(dst_con.mode_str, "1920x1080");

	test.dst_bo = init_crtc(&dst_con, dev);
	if (test.dst_bo == NULL) {
		printf("init dst crtc failed \n");
		return 0;
	}

	test.dst_con = dst_con;
	test.src_con = src_con;
	test.dev = dev;

	printf("Satrting NV12 to NV12 RGA test, [Press Enter to continue]\n");
	rga_nv12_to_nv12_test(&test);

	/*
	 * RGA API Related:
	 *
	 * Close the RGA device
	 */
	rga_fini(ctx);

	rockchip_destroy_buffer(test.src_bo);
	rockchip_destroy_buffer(test.dst_bo);

	drmClose(dev->fd);
	rockchip_device_destroy(dev);

	return 0;
}
