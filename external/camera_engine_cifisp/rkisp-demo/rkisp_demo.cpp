/*
 * V4L2 video capture example
 * AUTHOT : Jacob Chen
 * DATA : 2018-02-25
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h> /* getopt_long() */
#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <dlfcn.h>

#include <linux/videodev2.h>
#include "rkisp_interface.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

const char* _iq_file="/etc/cam_iq.xml";
void* _rkisp_engine;
typedef int (*rkisp_start_func)(void* &engine, int vidFd, const char* ispNode, const char* tuningFile);
typedef int (*rkisp_stop_func)(void* &engine);

struct RKIspFunc {
    void* cam_ra_handle;
    void* rkisp_engine_handle;
    rkisp_start_func start_func;
    rkisp_stop_func stop_func;
};
struct RKIspFunc _RKIspFunc;

struct buffer {
        void *start;
        size_t length;
};

static int verbose = 0;

static int fd = -1;
struct buffer *buffers;
static unsigned int n_buffers;

FILE *fp;
static char out_file[255];
static char dev_name[255];
static int width=1920;
static int height=1080;
static int frame_count = 1;
static int format = V4L2_PIX_FMT_NV12;

static void errno_exit(const char *s)
{
        fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
        exit(EXIT_FAILURE);
}

static int xioctl(int fh, int request, void *arg)
{
        int r;
        do {
                r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);
        return r;
}

static void process_image(const void *p, int size)
{
        if (verbose)
                printf("process_image size: %d\n",size);
        fwrite(p,size, 1, fp);
}

static int read_frame(FILE *fp)
{
        struct v4l2_buffer buf;
        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) 
                errno_exit("VIDIOC_DQBUF");

        process_image(buffers[buf.index].start, buf.bytesused);

        if (verbose)
        	printf("time stamp: %ld - %ld\n", buf.timestamp.tv_sec, buf.timestamp.tv_usec);

        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
            errno_exit("VIDIOC_QBUF"); 

        return 1;
}

static void mainloop(void)
{
        unsigned int count = 0;
        count = frame_count;
        while (count-- > 0) {
            printf(">");
            fflush(stdout);
            if ((frame_count - count) % 30 == 0)
                printf("\n");
            read_frame(fp);
        }
        printf("\nREAD AND SAVE DONE!\n");
}

static void stop_capturing(void)
{
        enum v4l2_buf_type type;

    	if (_RKIspFunc.stop_func != NULL) {
    	    printf ("deinit rkisp engine\n");
    	    _RKIspFunc.stop_func(_rkisp_engine);
    	    dlclose(_RKIspFunc.cam_ra_handle);
    	    dlclose(_RKIspFunc.rkisp_engine_handle);
    	}

        printf ("Call VIDIOC_STREAMOFF\n");
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
            errno_exit("VIDIOC_STREAMOFF");
}

static void start_capturing(void)
{
        unsigned int i;
        enum v4l2_buf_type type;


    	if (_RKIspFunc.start_func != NULL) {
    	    printf ("device manager start, capture dev fd: %d\n", fd);
    	    _RKIspFunc.start_func(_rkisp_engine, fd, "/dev/video1", _iq_file);
    	    printf ("device manager isp_init\n");

    	    if (_rkisp_engine == NULL) {
    	        printf ("rkisp_init engine failed\n");
    	    } else {
    	        printf ("rkisp_init engine succeed\n");
    	    }
    	}

        for (i = 0; i < n_buffers; ++i) {
                struct v4l2_buffer buf;

                CLEAR(buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;

                if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                        errno_exit("VIDIOC_QBUF");
        }
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
                errno_exit("VIDIOC_STREAMON");
}

static void uninit_device(void)
{
        unsigned int i;

        for (i = 0; i < n_buffers; ++i)
                if (-1 == munmap(buffers[i].start, buffers[i].length))
                        errno_exit("munmap");

        free(buffers);
}



static void init_mmap(void)
{
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        fprintf(stderr, "%s does not support "
                                 "memory mapping\n", dev_name);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_REQBUFS");
                }
        }

        if (req.count < 2) {
                fprintf(stderr, "Insufficient buffer memory on %s\n",
                         dev_name);
                exit(EXIT_FAILURE);
        }

        buffers = (struct buffer*)calloc(req.count, sizeof(*buffers));

        if (!buffers) {
                fprintf(stderr, "Out of memory\n");
                exit(EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
                struct v4l2_buffer buf;

                CLEAR(buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = n_buffers;

                if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
                        errno_exit("VIDIOC_QUERYBUF");

                buffers[n_buffers].length = buf.length;
                buffers[n_buffers].start =
                        mmap(NULL /* start anywhere */,
                              buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              fd, buf.m.offset);

                if (MAP_FAILED == buffers[n_buffers].start)
                        errno_exit("mmap");
        }
}


static void init_device(void)
{
        struct v4l2_capability cap;
        struct v4l2_format fmt;

        if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
                if (EINVAL == errno) {
                        fprintf(stderr, "%s is no V4L2 device\n",
                                 dev_name);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_QUERYCAP");
                }
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
                fprintf(stderr, "%s is no video capture device\n",
                         dev_name);
                exit(EXIT_FAILURE);
        }

        if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
                fprintf(stderr, "%s does not support streaming i/o\n",
                    dev_name);
                exit(EXIT_FAILURE);
        }



        CLEAR(fmt);
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = width;
        fmt.fmt.pix.height = height;
        fmt.fmt.pix.pixelformat = format;
        fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

        if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
                errno_exit("VIDIOC_S_FMT");

        init_mmap();

	//INIT RKISP
	_RKIspFunc.cam_ra_handle = dlopen("/usr/lib/libcam_ia.so", RTLD_LAZY | RTLD_GLOBAL);
    	if (_RKIspFunc.cam_ra_handle == NULL) {
    	    printf ("open /usr/lib/libcam_ia.so failed, error %s\n", dlerror());
    	}
	_RKIspFunc.rkisp_engine_handle = dlopen("/usr/lib/libcam_engine_cifisp.so", RTLD_LAZY | RTLD_GLOBAL);
    	if (_RKIspFunc.rkisp_engine_handle == NULL) {
    	    printf ("open user-defined lib(%s) failed, reason:%s", "/usr/lib/libcam_engine_cifisp.so", dlerror ());

    	} else {
    	    printf ("Load libcam_engine_cifisp.so successed\n");
    	    _RKIspFunc.start_func=(rkisp_start_func)dlsym(_RKIspFunc.rkisp_engine_handle, "rkisp_start");
    	    _RKIspFunc.stop_func=(rkisp_stop_func)dlsym(_RKIspFunc.rkisp_engine_handle, "rkisp_stop");
    	    if (_RKIspFunc.start_func == NULL) {
    	        printf ("func rkisp_start not found.");
    	        const char *errmsg;
    	        if ((errmsg = dlerror()) != NULL) {
    	            printf("dlsym rkisp_start fail errmsg: %s", errmsg);
    	        }
    	    } else {
    	        printf("dlsym rkisp_start success\n");
    	    }
    	}

}

static void close_device(void)
{
        if (-1 == close(fd))
                errno_exit("close");

        fd = -1;
}

static void open_device(void)
{
        fd = open(dev_name, O_RDWR /* required */ /*| O_NONBLOCK*/, 0);

        if (-1 == fd) {
                fprintf(stderr, "Cannot open '%s': %d, %s\n",
                         dev_name, errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
}

void parse_args(int argc, char **argv)
{
   int c;
   int digit_optind = 0;

   while (1) {
       int this_option_optind = optind ? optind : 1;
       int option_index = 0;
       static struct option long_options[] = {
           {"width",    required_argument, 0, 'w' },
           {"height",   required_argument, 0, 'h' },
           {"format",   required_argument, 0, 'f' },
           {"device",   required_argument, 0, 'd' },
           {"output",   required_argument, 0, 'o' },
           {"count",    required_argument, 0, 'c' },
           {"help",     no_argument,       0, 'p' },
           {"verbose",  no_argument,       0, 'v' },
           {0,          0,                 0,  0  }
       };

       c = getopt_long(argc, argv, "w:h:f:d:o:pv",
           long_options, &option_index);
       if (c == -1)
           break;

       switch (c) {
       case 'c':
           frame_count = atoi(optarg);
           break;
       case 'w':
           width = atoi(optarg);
           break;
       case 'h':
           height = atoi(optarg);
           break;
       case 'f':
           format = v4l2_fourcc(optarg[0], optarg[1], optarg[2], optarg[3]);
           break;
       case 'd':
           strcpy(dev_name, optarg);
           break;
       case 'o':
           strcpy(out_file, optarg);
           break;
       case '?':
       case 'p':
           printf("Usage: %s to capture cif_isp10 frames\n"
                  "             --width,  default 1920, optional, width of image\n"
                  "             --height, default 1080, optional, height of image\n"
                  "             --format, default NV12, optional, fourcc of format\n"
                  "             --count,  default    1, optional, how many frames to capture\n"
                  "             --device,               required, path of video device\n"
                  "             --output,               required, output file path\n"
                  "             --verbose,              optional, print more log\n",
                  argv[0]);
           exit(-1);

       default:
           printf("?? getopt returned character code 0%o ??\n", c);
       }
   }

   if (strlen(out_file) == 0 || strlen(dev_name) == 0) {
        fprintf(stderr, "arguments --output and --device are required\n");
        exit(-1);
   }

}


int main(int argc, char *argv[])
{
        parse_args(argc, argv);

        if ((fp = fopen(out_file, "w")) == NULL) {
            perror("Creat file failed");
            exit(0);
        }
        open_device();
        init_device();
        start_capturing();
        mainloop();
        fclose(fp);
        stop_capturing();
        uninit_device();
        close_device();
        return 0;
}
