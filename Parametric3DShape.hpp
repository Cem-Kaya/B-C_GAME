#ifndef PARAMETRIC3DSHAPE_HPP
#define PARAMETRIC3DSHAPE_HPP

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/common.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "ParametricLine.hpp"

#include <vector>
using namespace std;

class Parametric3DShape
{
public:
    static void generate(vector<glm::vec3> &positions,
                         vector<unsigned int> &indices,
                         glm::dvec2 (*parametricLine)(double),
                         unsigned vertical,
                         unsigned rotation);

private:
    static glm::vec3 rotated(double t, double r, glm::dvec2 (*parametricLine)(double));
    static unsigned int VHtoIndex(int v, int h, int max_v, int max_h);
};

void Parametric3DShape::generate(vector<glm::vec3> &positions,
                                 vector<unsigned int> &indices,
                                 glm::dvec2 (*parametricLine)(double),
                                 unsigned vertical,
                                 unsigned rotation)
{
    positions.clear();
    indices.clear();


    
    // fill positions
    for (int h = 0; h < rotation; h++)
    {
        for (int v = 0; v < vertical; v++)
        {
            double t = (double)v / (vertical - 1);
            double r = (double)h / rotation;
            positions.push_back(rotated(t, r, parametricLine));
        }
    }

    // fill indices
    for (int r = 0; r < rotation; r++)
    {
        for (int v = 0; v < vertical - 1; v++)
        {
            indices.push_back(VHtoIndex(v, r, vertical, rotation));
            indices.push_back(VHtoIndex(v + 1, r, vertical, rotation));
            indices.push_back(VHtoIndex(v, r + 1, vertical, rotation));

            indices.push_back(VHtoIndex(v, r + 1, vertical, rotation));
            indices.push_back(VHtoIndex(v + 1, r, vertical, rotation));
            indices.push_back(VHtoIndex(v + 1, r + 1, vertical, rotation));
        }
    }
}

glm::vec3 Parametric3DShape::rotated(double t, double r, glm::dvec2 (*parametricLine)(double))
{
    glm::dvec3 point = glm::dvec3((*parametricLine)(t), 0);

    return glm::rotateY(point, r * glm::two_pi<double>());
}

unsigned int Parametric3DShape::VHtoIndex(int v, int h, int max_v, int max_h)
{
    return (h % max_h) * max_v + v;
}

#endif