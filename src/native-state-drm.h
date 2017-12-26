#ifndef NATIVE_STATE_DRM_H_
#define NATIVE_STATE_DRM_H_

#include "native-state.h"
#include <csignal>
#include <cstring>
#include <gbm.h>
#include <drm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

class NativeStateDRM : public NativeState
{
public:
    NativeStateDRM() :
        fd_(0),
        resources_(0),
        connector_(0),
        encoder_(0),
        mode_(0),
        dev_(0),
        surface_(0),
        bo_(0),
        fb_(0),
        crtc_set_(false) {}
    ~NativeStateDRM() { cleanup(); }

    bool init_display();
    void* display();
    bool create_window(WindowProperties const& properties);
    void* window(WindowProperties& properties);
    void visible(bool v);
    bool should_quit();
    void flip();

    int get_fd();

private:
    struct DRMFBState
    {
        int fd;
        gbm_bo* bo;
        uint32_t fb_id;
    };

    static void page_flip_handler(int fd, unsigned int frame, unsigned int sec,
                                  unsigned int usec, void* data);
    static void fb_destroy_callback(gbm_bo* bo, void* data);
    static void quit_handler(int signum);
    static volatile std::sig_atomic_t should_quit_;

    DRMFBState* fb_get_from_bo(gbm_bo* bo);
    bool init_gbm();
    bool init();
    void cleanup();

    int fd_;
    drmModeRes* resources_;
    drmModeConnector* connector_;
    drmModeEncoder* encoder_;
    drmModeCrtcPtr crtc_;
    drmModeModeInfo* mode_;
    gbm_device* dev_;
    gbm_surface* surface_;
    gbm_bo* bo_;
    DRMFBState* fb_;
    bool crtc_set_;
};

#endif /* NATIVE_STATE_DRM_H_ */
