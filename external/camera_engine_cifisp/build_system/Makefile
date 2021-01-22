CROSS_COMPILE ?= /usr/local/arm/4.4.3/bin/arm-linux-

export CROSS_COMPILE
LOCAL_PATH:= $(CURDIR)
include $(LOCAL_PATH)/build_system/Makefile.rules

SUBDIRS = stlport common ebase tinyxml2 cam_calibdb isp_cam_api
export BUILD_EVERYTHING
export CLEAN_EVERYTHING

define BUILD_EVERYTHING
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) -f Android.mk ) \
		|| exit 1; \
	done
endef

define CLEAN_EVERYTHING
	-rm -fr ./build
	-rm -f `find ./ -name *.o`
	-rm -f `find ./ -name *.a`
	-rm -f `find ./ -name *.so`
endef

.PHONY:all
all:
	$(BUILD_EVERYTHING)

clean:
	$(CLEAN_EVERYTHING)
	
