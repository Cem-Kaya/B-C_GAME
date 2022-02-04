#ifndef HELPER_HPP
#define HELPER_HPP
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/common.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"


#include <iostream>


#include <sstream>

std::ostream& operator<< (std::ostream& out, const glm::vec3& vec) {
    out << "{"
        <<"x: " << vec.x << " y: " << vec.y << " z: " << vec.z
        << "}";

    return out;
}

#endif