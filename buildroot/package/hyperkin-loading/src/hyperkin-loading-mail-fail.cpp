#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#define MESA_EGL_NO_X11_HEADERS
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

EGLDisplay egl_display;
EGLSurface egl_surface;
EGLint egl_major, egl_minor;
int main(int argc, char** argv){
	printf("Hyperkin Loading Start\n");
	static const int MAX_DEVICES = 4;
	EGLDeviceEXT eglDevs[MAX_DEVICES];
	EGLint numDevices;

	if(check_egl_client_extension("EGL_EXT_platform_base")){
		PFNEGLGETPLATFORMDISPLAYEXTPROC ptr_eglGetPlatformDisplayEXT;
		ptr_eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC) eglGetProcAddress("eglGetPlatformDisplayEXT");
		if (ptr_eglGetPlatformDisplayEXT != NULL){
			ptr_eglGetPlatformDisplayEXT(EGL_NONE, EGL_DEFAULT_DISPLAY, NULL);
		}
	}
  
	PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC)eglGetProcAddress("eglQueryDevicesEXT");
	eglQueryDevicesEXT(MAX_DEVICES, eglDevs, &numDevices);
	printf("Detected %d devices\n", numDevices);

	PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC) eglGetProcAddress("eglGetPlatformDisplayEXT");
	egl_display = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, eglDevs[0], 0);
	if(egl_display == EGL_NO_DISPLAY) {
		printf("Error: No display found!\n");
		return -1;
	}else{
		printf("eglGetDisplay done\n");
	}
	if(!eglInitialize(egl_display, &egl_major, &egl_minor)){
		printf("Error: eglInitialise failed!\n");
		return -1;
	}else{
		printf("eglInitialize done\n");
	}
	printf("EGL Version: \"%s\"\n", eglQueryString(egl_display, EGL_VERSION));
	printf("EGL Vendor: \"%s\"\n", eglQueryString(egl_display, EGL_VENDOR));
	printf("EGL Extensions: \"%s\"\n", eglQueryString(egl_display, EGL_EXTENSIONS));
	return EXIT_SUCCESS;
}
