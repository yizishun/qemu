/*
 * udmabuf helper functions.
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */
#include "qemu/osdep.h"
#include "qapi/error.h"
#include "ui/console.h"
#include "qemu/error-report.h"
#include "standard-headers/linux/udmabuf.h"

#include <sys/ioctl.h>

static int udmabuf_fd(void)
{
    static bool first = true;
    static int udmabuf;

    if (!first) {
        return udmabuf;
    }
    first = false;

    udmabuf = open("/dev/udmabuf", O_RDWR);
    if (udmabuf < 0) {
        warn_report("open /dev/udmabuf: %s", strerror(errno));
    }
    return udmabuf;
}

bool udmabuf_available(void)
{
    return (udmabuf_fd() >= 0);
}

int udmabuf_do_create(const struct udmabuf_create *create)
{
    int udmabuf, dmabuf_fd;

    udmabuf = udmabuf_fd();
    if (udmabuf < 0) {
        return -1;
    }

    dmabuf_fd = ioctl(udmabuf, UDMABUF_CREATE, create);
    if (dmabuf_fd < 0) {
        warn_report("%s: UDMABUF_CREATE: %s", __func__, strerror(errno));
    }
    return dmabuf_fd;
}

int udmabuf_do_create_list(const struct udmabuf_create_list *list)
{
    int udmabuf, dmabuf_fd;
    
    if (list->count < 1) {
        errno = EINVAL;
        return -1;
    }
    
    if (list->count == 1) {
        struct udmabuf_create item = {
            .memfd = list->list[0].memfd,
            .flags = list->flags,
            .offset = list->list[0].offset,
            .size = list->list[0].size
        };
        return udmabuf_do_create(&item);
    }

    udmabuf = udmabuf_fd();
    if (udmabuf < 0) {
        return -1;
    }

    dmabuf_fd = ioctl(udmabuf, UDMABUF_CREATE_LIST, list);
    if (dmabuf_fd < 0) {
        warn_report("%s: UDMABUF_CREATE_LIST: %s", __func__, strerror(errno));
    }
    return dmabuf_fd;
}
