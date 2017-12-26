#ifndef GST_EGL_RENDER_H_
#define GST_EGL_RENDER_H_

#define GL_GLEXT_PROTOTYPES
#define GL_EGLEXT_PROTOTYPES

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "dma-buffer.h"

bool egl_setup_graphics (void);
bool egl_get_image_for_dma_buffer (struct DmaBuffer *buf, EGLImageKHR *outImage);
bool egl_texture_for_image (EGLImageKHR image, GLuint *outTex);
bool egl_sample_buffer (struct DmaBuffer *buf);

void egl_release (void);

#endif // GST_EGL_RENDER_H_
