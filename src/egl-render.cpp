#include "egl-render.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <drm_fourcc.h>
#include <unistd.h>

#define PI (3.14159265f)
#define SPHERE_SIZE 63
int dotNumber = SPHERE_SIZE * 6 + (SPHERE_SIZE - 2) * 6 * SPHERE_SIZE;

float mvpMat[16] = {
    167.81992,   0.0,       0.0,        0.0,
    0.0,         167.81992, 0.0,        0.0,
    0.0,         0.0,       -202.01006, -200.0,
    0.025592538, 0.0,       -1.0050251, 0.0
};

static const char gVertexShader[] =
        "attribute vec3 position;\n"
        "attribute vec2 texCoords;\n"
        "uniform mat4 uMvp;\n"
        "varying vec2 outTexCoords;\n"
        "\nvoid main(void) {\n"
        "    outTexCoords = texCoords.xy;\n"
        "    gl_Position = uMvp * vec4(position,1.0);\n"
        "	 gl_Position = gl_Position.xyzz;\n"
        "}\n\n";

static const char gFragmentShader[] =
        "#extension GL_OES_EGL_image_external : require\n"
        "precision mediump float;\n\n"
        "varying vec2 outTexCoords;\n"
        "uniform samplerExternalOES texture;\n"
        "\nvoid main(void) {\n"
        "    gl_FragColor = texture2D(texture, outTexCoords);\n"
        "    gl_FragColor.rgb = gl_FragColor.rgb;\n"
        "}\n\n";

GLfloat* textureCoords = 0;
GLfloat* vertices = 0;
GLushort* indices = 0;

GLuint gTextureProgram = 0;
GLuint gvTexturePositionHandle = 0;
GLuint gvTextureTexCoordsHandle = 0;
GLuint gvTextureSamplerHandle = 0;
GLuint uTextureCoordMatrix = 0;

PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHRProc;
PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHRProc;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOESProc;

static EGLImageKHR eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx, EGLenum target,
                                     EGLClientBuffer buffer, const EGLint *attrib_list)
{
    if (!eglCreateImageKHRProc)
        eglCreateImageKHRProc = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");

    return eglCreateImageKHRProc(dpy, ctx, target, buffer, attrib_list);
}

static void eglDestroyImageKHR(EGLDisplay dpy, EGLImageKHR image)
{
    if (!eglDestroyImageKHRProc)
        eglDestroyImageKHRProc = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");

    eglDestroyImageKHRProc(dpy, image);
}

static void eglEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image)
{
    if (!glEGLImageTargetTexture2DOESProc)
        glEGLImageTargetTexture2DOESProc = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)
                eglGetProcAddress("glEGLImageTargetTexture2DOES");

    return glEGLImageTargetTexture2DOESProc(target, image);
}

static GLuint egl_load_shader(GLenum shaderType, const char *pSource)
{
    GLint compiled = 0;
    GLuint shader = 0;

    shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    fprintf(stderr, "Could not compile shader %d:\n%s\n",
                            shaderType, buf);
                    free(buf);
                }

                glDeleteShader(shader);
                shader = 0;
            }
        }
    }

    return shader;
}

static GLuint egl_create_program(const char *pVertexSource, const char *pFragmentSource)
{
    GLuint vertexShader = 0;
    GLuint pixelShader = 0;
    GLuint program = 0;
    GLint linkStatus = GL_FALSE;

    vertexShader = egl_load_shader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader)
        return 0;

    pixelShader = egl_load_shader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader)
        return 0;

    program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        glAttachShader(program, pixelShader);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    fprintf(stderr, "Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }

    glDeleteShader(vertexShader);
    glDeleteShader(pixelShader);

    return program;
}

static void egl_general_sphere(int numSlices, GLfloat radius, GLfloat **vertices,
                               GLfloat **textureCoords, GLushort **indices)
{
    int i, j;
    int numUvs;
    int vertIndex, index;
    int numVertices, numIndices;

    vertIndex = 0, index = 0;
    numVertices = (numSlices + 1) * (numSlices + 1);
    numIndices = numSlices * (numSlices - 1) * 6;
    *vertices = (GLfloat*) malloc(sizeof(GLfloat) * 3 * numVertices);
    *indices = (GLushort*) malloc(sizeof(GLushort) * numIndices);

    for (j = 0; j <= numSlices; j++) {
        float horAngle = (float) (PI * j / numSlices);
        float z = radius * (float) cos(horAngle);
        float ringRadius = radius * (float) sin(horAngle);

        for (i = 0; i <= numSlices; i++) {
            float verAngle = (float) (2.0 * PI * i / numSlices);
            float x = ringRadius * (float) cos(verAngle);
            float y = ringRadius * (float) sin(verAngle);

            (*vertices)[vertIndex++] = x;
            (*vertices)[vertIndex++] = z;
            (*vertices)[vertIndex++] = y;

            if (i > 0 && j > 0) {
                int a = (numSlices + 1) * j + i;
                int b = (numSlices + 1) * j + i - 1;
                int c = (numSlices + 1) * (j - 1) + i - 1;
                int d = (numSlices + 1) * (j - 1) + i;

                if (j == numSlices) {
                    (*indices)[index++] = a;
                    (*indices)[index++] = d;
                    (*indices)[index++] = c;
                } else if (j == 1) {
                    (*indices)[index++] = a;
                    (*indices)[index++] = c;
                    (*indices)[index++] = b;
                } else {
                    (*indices)[index++] = a;
                    (*indices)[index++] = c;
                    (*indices)[index++] = b;
                    (*indices)[index++] = a;
                    (*indices)[index++] = d;
                    (*indices)[index++] = c;
                }
            }
        }
    }

    numUvs = (numSlices + 1) * (numSlices + 1) * 2;
    *textureCoords = (GLfloat*) malloc(numUvs* sizeof(GLfloat));

    numUvs = 0;
    for (j = 0; j <= numSlices; ++j) {
        for (i = numSlices; i >= 0; --i) {
            float u = (float) i / numSlices;
            (*textureCoords)[numUvs++] =  1.0 - u;
            (*textureCoords)[numUvs++] = (float) j / numSlices;
        }
    }
}

bool egl_setup_graphics(void)
{
    if (!vertices)
        egl_general_sphere(SPHERE_SIZE, 1.0, &vertices, &textureCoords, &indices);

    if (gTextureProgram > 0) {
        glDeleteProgram(gTextureProgram);
        gTextureProgram = 0;
    }

    gTextureProgram = egl_create_program(gVertexShader, gFragmentShader);
    if (!gTextureProgram)
        return false;

    gvTexturePositionHandle = glGetAttribLocation(gTextureProgram, "position");
    gvTextureTexCoordsHandle = glGetAttribLocation(gTextureProgram, "texCoords");
    gvTextureSamplerHandle = glGetUniformLocation(gTextureProgram, "texture");
    uTextureCoordMatrix = glGetUniformLocation(gTextureProgram, "uMvp");

    return true;
}

bool egl_get_image_for_dma_buffer(struct DmaBuffer *buf, EGLImageKHR *outImage)
{
    EGLImageKHR image;
    // NV12 for example.
    EGLint attr[] = {
        EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_NV12,
        EGL_WIDTH, buf->width,
        EGL_HEIGHT, buf->height,
        EGL_DMA_BUF_PLANE0_FD_EXT, buf->dma_fd,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, buf->width,
        EGL_DMA_BUF_PLANE1_FD_EXT, buf->dma_fd,
        EGL_DMA_BUF_PLANE1_OFFSET_EXT, buf->width * buf->height,
        EGL_DMA_BUF_PLANE1_PITCH_EXT, buf->width,
        EGL_NONE
    };

    image = eglCreateImageKHR(eglGetCurrentDisplay(), EGL_NO_CONTEXT,
                              EGL_LINUX_DMA_BUF_EXT, (EGLClientBuffer)0,
                              attr);
    if (!image || image == EGL_NO_IMAGE_KHR) {
        fprintf(stderr, "link imageKHR glError (0x%x)\n", eglGetError());
        return false;
    }

    *outImage = image;

    return true;
}

bool egl_texture_for_image(EGLImageKHR image, GLuint *outTex)
{
    GLuint texture;
    GLenum error;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture);

    // Set the image as level zero
    eglEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, (GLeglImageOES) image);
    error = glGetError();
    if (error != GL_NO_ERROR)
        return false;

    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    *outTex = texture;

    // Draw copied content on the screen.
    glUseProgram(gTextureProgram);
    glUniform1i(gvTextureSamplerHandle, 0);
    glUniformMatrix4fv(uTextureCoordMatrix, 1, GL_FALSE, mvpMat);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture);

    glVertexAttribPointer(gvTexturePositionHandle, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), vertices);
    glEnableVertexAttribArray(gvTexturePositionHandle);
    glVertexAttribPointer(gvTextureTexCoordsHandle, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), textureCoords);
    glEnableVertexAttribArray(gvTextureTexCoordsHandle);
    glDrawElements(GL_TRIANGLES, dotNumber, GL_UNSIGNED_SHORT, indices);

    return true;
}

bool egl_sample_buffer(struct DmaBuffer *buf)
{
    EGLImageKHR imageKHR;
    GLuint gTexture;
    bool result;
    int ret;

    result = false;
    gTexture = 0;

    result = egl_setup_graphics();
    if (!result) {
        printf("Could not general sphere\n");
        goto _destory;
    }

    result = false;
    ret = egl_get_image_for_dma_buffer(buf, &imageKHR);
    if (!ret) {
        printf("Failed to create imageKHR.\n");
        goto _destory;
    }

    ret = egl_texture_for_image(imageKHR, &gTexture);
    if (!ret) {
        printf("Failed to glEGLImageTargetTexture2DOES().\n");
        goto _destory;
    }

    result = true;

_destory:
    if (gTexture > 0)
        glDeleteTextures(1, &gTexture);

    if (gTextureProgram > 0) {
        glDeleteProgram(gTextureProgram);
        gTextureProgram = 0;
    }

    if (imageKHR)
        eglDestroyImageKHR(eglGetCurrentDisplay(), imageKHR);

    return result;
}

void egl_release(void)
{    
    if (gTextureProgram > 0) {
        glDeleteProgram(gTextureProgram);
        gTextureProgram = 0;
    }

    free(textureCoords);
    free(vertices);
    free(indices);
}
