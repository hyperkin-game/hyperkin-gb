#include "updater_recovery_start.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>

int vendor_storage_write(int buf_size, uint8 *buf, uint16 vendor_id)
{
    int ret = 0;
    int fd;
    RK_VERDOR_REQ req;

    fd = open(VERDOR_DEVICE, O_RDWR, 0);
    if (fd < 0) {
        printf("vendor_storage open fail, errno = %d\n", errno);
        return -1;
    }
    req.tag = VENDOR_REQ_TAG;
    req.id = vendor_id;
    req.len = buf_size > VENDOR_DATA_SIZE ? VENDOR_DATA_SIZE : buf_size;
    memcpy(req.data, buf, req.len);
    ret = ioctl(fd, VENDOR_WRITE_IO, &req);
    if (ret) {
        printf("vendor write error, ret = %d\n", ret);
        close(fd);
        return -1;
    }
    fsync(fd);
    close(fd);
    return 0;
}

int vendor_storage_read(int buf_size, uint8 *buf, uint16 vendor_id)
{
    int ret = 0;
    int fd;
    RK_VERDOR_REQ req;

    fd = open(VERDOR_DEVICE, O_RDWR, 0);
    if (fd < 0) {
        printf("vendor_storage open fail, errno = %d\n", errno);
        return -1;
    }
    req.tag = VENDOR_REQ_TAG;
    req.id = vendor_id;
    req.len = buf_size > VENDOR_DATA_SIZE ? VENDOR_DATA_SIZE : buf_size;
    ret = ioctl(fd, VENDOR_READ_IO, &req);
    if (ret) {
        printf("vendor read error, ret = %d\n", ret);
        close(fd);
        return -1;
    }
    close(fd);
    memcpy(buf, req.data, req.len);

    return 0;
}

int fw_flag_check(char* path)
{
    int fdfile = 0;
    int ret = 0;
    STRUCT_PART_INFO partition;

    fdfile = open(path, O_RDONLY);
    if (fdfile <= 0) {
        printf("fw_flag_check open %s failed! \n", path);
        perror("open");
        return -1;
    }

    ret = read(fdfile, &partition, sizeof(STRUCT_PART_INFO));
    if (ret <= 0) {
        close(fdfile);
        printf("fw_flag_check read %s failed! \n", path);
        perror("read");
        return -1;
    }
    close(fdfile);

    if (partition.hdr.uiFwTag != RK_PARTITION_TAG) {
        printf("ERROR: Your firmware(%s) is invalid!\n", path);
        return -1;
    }

    return 0;
}

/* +++++++++++++++ 待扩展 ++++++++++++ */
int fw_md5_check()
{
    return 0;
}
