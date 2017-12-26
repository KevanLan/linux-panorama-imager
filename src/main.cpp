#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "native-state-drm.h"
#include "gl-state-egl.h"
#include "canvas-generic.h"
#include "dma-buffer.h"
#include "egl-render.h"
#include "log.h"

bool setupGraphics(DmaBufferManager *manager)
{
    void *textureData;
    DmaBuffer *dma_buf;
    bool ret;
    FILE *fp;

    dma_buf = (struct DmaBuffer*) calloc(sizeof(struct DmaBuffer), 1);
    textureData = malloc(1920 * 1080 * 3 / 2);

    fp = fopen("/mnt/1920x1080_nv12.bin", "r");
    if (fp == NULL) {
        Log::error("Open source file failed\n");
        ret = false;
        goto _exit_release;
    }
    fread(textureData, 1920 * 1080 * 3 / 2, 1, fp);
    fclose(fp);

    if (!manager->createDmaBuffer(1920, 1080, 32, textureData, dma_buf)) {
        ret = false;
        goto _exit_release;
    }

    if (!egl_sample_buffer(dma_buf)) {
        ret = false;
        goto _exit_release;
    }

    ret = true;

_exit_release:
    free(dma_buf);
    free(textureData);

    return ret;
}

int main(int argc, char** argv)
{
    /* initialize Log class */
    Log::init("gl2Imager", true);

    NativeStateDRM native_state;
    GLStateEGL gl_state;

    CanvasGeneric canvas(native_state, gl_state);
    if (!canvas.init()) {
        Log::error("%s: Could not initialize canvas\n", __FUNCTION__);
        return 1;
    }

    canvas.print_info();
    canvas.visible(true);

    DmaBufferManager bufferManager(native_state.get_fd());
    /* renderer image and display for 30 seconds */
    if (!setupGraphics(&bufferManager)) {
        Log::error("Could not set up graphics\n");
        return 1;
    }
    canvas.update();
    sleep(30);

    return 0;
}
