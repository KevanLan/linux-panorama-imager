#ifndef GL_STATE_H_
#define GL_STATE_H_

class GLVisualConfig;

class GLState
{
public:
    virtual ~GLState() {}

    virtual bool init_display(void *native_display, GLVisualConfig& config_pref) = 0;
    virtual bool init_surface(void *native_window) = 0;
    virtual void init_gl_extensions() = 0;
    virtual bool valid() = 0;
    virtual bool reset() = 0;
    virtual void swap() = 0;
    virtual bool gotNativeConfig(int& vid) = 0;
    virtual void getVisualConfig(GLVisualConfig& vc) = 0;
};

#endif /* GL_STATE_H_ */
