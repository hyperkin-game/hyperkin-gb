lib_LTLIBRARIES = libxcam_rkisp.la

XCAMRKISP_CXXFLAGS = $(XCAM_CXXFLAGS)
XCAMRKISP_LIBS = \
    $(NULL)

if USE_LOCAL_ATOMISP
XCAMRKISP_CXXFLAGS += \
    -I$(top_srcdir)/ext/atomisp  \
    $(NULL)
endif

if ENABLE_IA_AIQ
XCAMRKISP_CXXFLAGS += \
    $(IA_IMAGING_CFLAGS)  \
    $(NULL)

XCAMRKISP_LIBS += \
    $(IA_IMAGING_LIBS)  \
    $(NULL)
endif

if ENABLE_RK_IQ
XCAMRKISP_CXXFLAGS += \
    $(RKISP_CFLAGS)  \
    $(NULL)

XCAMRKISP_LIBS += \
    $(RKISP_LIBS)  \
    $(NULL)
endif

xcam_rkisp_sources = \
    rkisp_device.cpp          \
	isp_poll_thread.cpp       \
	isp_controller.cpp        \
	isp_config_translator.cpp \
	x3a_isp_config.cpp        \
	x3a_statistics_queue.cpp  \
	sensor_descriptor.cpp     \
	isp_image_processor.cpp   \
    $(NULL)

if ENABLE_IA_AIQ
xcam_rkisp_sources += \
    aiq_handler.cpp             \
    hybrid_analyzer.cpp         \
    hybrid_analyzer_loader.cpp  \
    x3a_analyzer_aiq.cpp        \
    $(NULL)
endif

if HAVE_LIBDRM
XCAMRKISP_CXXFLAGS += $(LIBDRM_CFLAGS)
XCAMRKISP_LIBS += \
    $(LIBDRM_LIBS)  \
    $(NULL)
endif

libxcam_rkisp_la_SOURCES = \
    $(xcam_rkisp_sources)  \
    $(NULL)

libxcam_rkisp_la_CXXFLAGS = \
    $(XCAMRKISP_CXXFLAGS)            \
    -I$(top_builddir)/xcore        \
    -I$(top_builddir)/modules/rkisp  \
    $(NULL)

libxcam_rkisp_la_CFLAGS = $(libxcam_rkisp_la_CXXFLAGS)

libxcam_rkisp_la_LIBADD = \
    $(top_builddir)/xcore/libxcam_core.la \
    $(XCAMRKISP_LIBS)                       \
    $(NULL)

libxcam_rkisp_la_LDFLAGS = \
    $(XCAM_LT_LDFLAGS)  \
    $(PTHREAD_LDFLAGS)  \
    $(NULL)

libxcam_rkisp_la_LIBTOOLFLAGS = --tag=disable-static
