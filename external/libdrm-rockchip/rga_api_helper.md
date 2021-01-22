
RGA Usage Help Document
============

Rockchip RGA is a kind of hardware 2D accelerator, and it support solid, rotation, scaling, color format transform operations.

This libdrm-rockchip.so have integrated the RGA driver, and it have created some API for userspace to take use of RGA module. And this document would help you to understand how to use those API functions.

Library have provided 8 functions for caller:

- rga_init(...)
- rga_fini(...)
- rga_exec(...)
- rga_solid_fill(...)
- rga_copy_with_scale(...)
- rga_copy_with_rotate(...)
- rga_multiple_transform(...)

It's easy to see that **rga_init** and **rga_fini** are used to open/close RGA device. The **rga_exec** is used for caller to start RGA hardware device transform, and the leftover functions are used for setting the request to RGA transform queue.

There're an important data struct when caller want to send the RGA transform request:
```
struct rga_image {
	unsigned int			color_mode;
	unsigned int			width;
	unsigned int			height;
	unsigned int			stride;
	unsigned int			fill_color;
	enum e_rga_buf_type		buf_type;
	unsigned int			bo[RGA_PLANE_MAX_NR];
	struct drm_rockchip_rga_userptr	user_ptr[RGA_PLANE_MAX_NR];
};
```
Each RGA transform request must have the dst_img that declared with rga_img data struct, and at most case we also have the src_img data struct (like src_img rotate to dst_img). Let me introduce more details about those variables.

- **color_mode**:  This is variable means what color format that images configured, so caller should fill this variable with DRM stander color format (defined in drm_fourcc.h).
- **width / height**:  The width/height of framebuffer.
- **stride**: The number of bytes with single line.
- **fill_color**: This variable only used in `rga_solid_fill` functions, it means the target color values that caller want to fill the images. It's a 32-bit number, and the data format is ARGB8888, like you want the image filled with RED color, then you need to write 0x00FF0000 to it.
- **buf_type**: For now, driver only support RGA_IMGBUF_GEM type
- **bo[]**:  Caller should write the fd of dma-buf to it (`bo[0] = fd`)
- **user_ptr[]**: Reserved.


---------------------------
Here is an simple example code:
```
struct rga_context *ctx;
struct rockchip_bo *dst_bo, *src_bo;
struct rga_image src_img = {0}, dst_img = {0};
int dst_fd, src_fd;

ctx = rga_init(dev->fd);
if (!ctx)
	return -EFAULT;

src_bo = rockchip_create_buffer(dev, 640 * 480 * 4, 0);
if (!src_bo) {
	fprintf(stderr, "Failed to create source fb!\n");
	return -EFAULT;
}


dst_bo = rockchip_create_buffer(dev, 720 * 480 * 2, 0);
if (!dst_bo) {
	fprintf(stderr, "Failed to create source fb!\n");
	return -EFAULT;
}

drmPrimeHandleToFD(dev->fd, dst_bo->handle, 0 , &dst_fd);
drmPrimeHandleToFD(dev->fd, src_bo->handle, 0 , &src_fd);
dst_img.bo[0] = dst_fd;
src_img.bo[0] = src_fd;


/*
 * Source Framebuffer OPS
 */
src_img.width = 640;
src_img.height = 480;
src_img.stride = 640 * 4;
src_img.buf_type = RGA_IMGBUG_GEM;
src_img.color_mode = DRM_FORMAT_ARGB8888;

/*
 * Dest Framebuffer OPS
 */
dst_img.width = 720;
dst_img.height = 480;
dst_img.stride = 720 * 2;
dst_img.buf_type = RGA_IMGBUG_GEM;
dst_img.color_mode = DRM_FORMAT_RGB565;

/*
 * Solid the source fb with blue color
 */
src_img.ill_color = 0xff;
rga_solid_fill(ctx, &src_img, 0, 0, src_img.width, src_img.height);

rga_copy_with_scale(ctx, src_img, dst_img,
		    0, 0, 640, 480,
		    0, 0, 720, 480);
rga_exec();
```
