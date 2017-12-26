#ifndef DMA_BUFFER_H_
#define DMA_BUFFER_H_

#include <stdlib.h>

struct DmaBuffer
{
    int width;
    int height;

    int dma_fd;
    size_t offset;
    size_t stride;
    unsigned handle;
};

class DmaBufferManager
{
public:
    DmaBufferManager(int drm_fd);
    ~DmaBufferManager();

    bool createDmaBuffer(int width, int height, int bpp, void *data, DmaBuffer *buffer);
    bool exportDmaBuffer(DmaBuffer *buffer);
    bool destoryDmaBuffer(DmaBuffer *buffer);

private:
    int _drm_fd;
};

#endif // DMA_BUFFER_H_
