#ifndef GL_HEADERS_H_
#define GL_HEADERS_H_

#define GL_GLEXT_PROTOTYPES

#include <string>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#ifndef GL_WRITE_ONLY
#define GL_WRITE_ONLY GL_WRITE_ONLY_OES
#endif
#ifndef GL_DEPTH_COMPONENT24
#define GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24_OES
#endif
#ifndef GL_DEPTH_COMPONENT32
#define GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT32_OES
#endif
#ifndef GL_RGBA8
#define GL_RGBA8 GL_RGBA8_OES
#endif
#ifndef GL_RGB8
#define GL_RGB8 GL_RGB8_OES
#endif

/**
 * Struct that holds pointers to functions that belong to extensions
 * in either GL2.0 or GLES2.0.
 */
struct GLExtensions {
    /**
     * Whether the current context has support for a GL extension.
     *
     * @return true if the extension is supported
     */
    static bool support(const std::string &ext);

    static void* (*MapBuffer) (GLenum target, GLenum access);
    static GLboolean (*UnmapBuffer) (GLenum target);
};

#endif
