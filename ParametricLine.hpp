#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/common.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace ParametricLine
{
    glm::dvec2 halfCircle(double t)
    {
        // [0, 1]
        t -= 0.5;
        // [-0.5, 0.5]
        t *= glm::pi<double>();
        // [-PI/2, PI/2]
        
        return glm::dvec2(cos(t), sin(t));
    }

    glm::dvec2 circle(double t)
    {
        // [0, 1]
        t -= 0.5;
        // [-0.5, 0.5]
        t *= glm::two_pi<double>();
        // [-PI, PI]

        auto c = glm::dvec2(0.7, 0);
        auto r = 0.3;
        return glm::dvec2(cos(t), sin(t)) * r + c;
    };

    glm::dvec2 spikes(double t)
    {
        // [0, 1]
        t -= 0.5;
        // [-0.5, 0.5]
        t *= glm::two_pi<double>();
        // [-PI, PI]

        auto c = glm::dvec2(0.7, 0);
        auto r = 0.3;
        auto a = 2 + 4 * 2;
        return (glm::dvec2(cos(t) + sin(a * t) / a, sin(t) + cos(a * t) / a)) * r + c;
    };
}
