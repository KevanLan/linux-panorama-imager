#include "dma-buffer.h"
#include "log.h"

#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <drm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

DmaBufferManager::DmaBufferManager(int drm_fd)
{
    _drm_fd = drm_fd;
}

DmaBufferManager::~DmaBufferManager()
{
}

bool DmaBufferManager::createDmaBuffer(int width, int height, int bpp, void *data, DmaBuffer *buffer)
{
    struct drm_mode_create_dumb create_arg;
    struct drm_mode_map_dumb map_arg;
    void *map;
    int ret;

    if (_drm_fd <= 0) {
        Log::error("init drm state first\n");
        return false;
    }

    /* alloc */
    memset(&create_arg, 0, sizeof(create_arg));
    create_arg.bpp = bpp;
    create_arg.width = width;
    create_arg.height = height;
    ret = drmIoctl(_drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_arg);
    if (ret) {
        Log::error("failed to create dumb buffer\n");
        return false;
    }

    buffer->width = width;
    buffer->height = height;
    buffer->handle = create_arg.handle;

    /* mmap */
    memset(&map_arg, 0, sizeof(map_arg));
    map_arg.handle = create_arg.handle;
    ret = drmIoctl(_drm_fd, DRM_IOCTL_MODE_MAP_DUMB, &map_arg);
    if (ret) {
        Log::error("failed to map dumb buffer\n");
        return false;
    }

    map = mmap(0, create_arg.size, PROT_READ | PROT_WRITE, MAP_SHARED, _drm_fd, map_arg.offset);
    if (map == MAP_FAILED) {
        Log::error("failed to map data\n");
        return false;
    }

    // copy an image data.
    memcpy(map, data, strlen((char*)data));

    /* unmap */
    munmap(map, create_arg.size);

    /* export dma-buffer */
    exportDmaBuffer(buffer);

    return true;
}

bool DmaBufferManager::exportDmaBuffer(DmaBuffer *buffer)
{
    int ret;

    ret = drmPrimeHandleToFD(_drm_fd, buffer->handle, 0, &buffer->dma_fd);
    if (ret != 0) {
        Log::error("failed to export gem bo handler\n");
        return false;
    }

    return true;
}

bool DmaBufferManager::destoryDmaBuffer(DmaBuffer *buffer)
{
    struct drm_mode_destroy_dumb arg;
    int ret;

    memset(&arg, 0, sizeof(arg));
    arg.handle = buffer->handle;
    ret = drmIoctl(_drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &arg);
    if (ret) {
        Log::error("failed to destroy dumb buffer\n");
        return false;
    }

    return true;
}
