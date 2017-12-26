#ifndef GL_VISUAL_CONFIG_H_
#define GL_VISUAL_CONFIG_H_

#include <string>

/**
 * Configuration parameters for a GL visual
 */
class GLVisualConfig
{
public:
    GLVisualConfig():
        red(1), green(1), blue(1), alpha(1), depth(1), stencil(0), buffer(1) {}
    GLVisualConfig(int r, int g, int b, int a, int d, int s, int buf):
        red(r), green(g), blue(b), alpha(a), depth(d), stencil(s), buffer(buf) {}
    GLVisualConfig(const std::string &s);

    /**
     * How well a GLVisualConfig matches another target config.
     *
     * The returned score has no meaning on its own. Its only purpose is
     * to allow comparison of how well different configs match a target
     * config, with a higher scores denoting a better match.
     *
     * Also note that this operation is not commutative:
     * a.match_score(b) != b.match_score(a)
     *
     * @return the match score
     */
    int match_score(const GLVisualConfig &target) const;

    int red;
    int green;
    int blue;
    int alpha;
    int depth;
    int stencil;
    int buffer;

private:
    int score_component(int component, int target, int scale) const;
};

#endif
