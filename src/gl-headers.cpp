#include "gl-headers.h"

void* (*GLExtensions::MapBuffer) (GLenum target, GLenum access) = 0;
GLboolean (*GLExtensions::UnmapBuffer) (GLenum target) = 0;

bool GLExtensions::support(const std::string &ext)
{
    std::string ext_string;
    const char* exts = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
    if (exts) {
        ext_string = exts;
    }

    const size_t ext_size = ext.size();
    size_t pos = 0;

    while ((pos = ext_string.find(ext, pos)) != std::string::npos) {
        char c = ext_string[pos + ext_size];
        if (c == ' ' || c == '\0')
            break;
    }

    return pos != std::string::npos;
}
