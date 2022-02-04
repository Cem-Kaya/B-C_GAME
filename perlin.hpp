#ifndef PERLIN_H
#define PERLIN_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/common.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"

#include "Window.hpp"
#include "VAO.hpp"
#include "ShaderProgram.hpp"
#include "Parametric3DShape.hpp"
#include "Camera.hpp"
#include "Object.hpp"
#include "helper.hpp"

#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include<cmath>
#include <stdio.h>      
#include <stdlib.h>     
#include <time.h> 

// helper function from my ray tracer 



inline float random_float() {
    // Returns a random real in [0,1).
    srand(time(NULL));   
    return  rand() / (RAND_MAX + 1.0);
}

inline float random_float(float min, float max) {
    // Returns a random real in [min,max).
    return min + (max - min) * random_float();
}

inline int random_int(int min , int max ) {
    return (int) (min + (max - min) * random_float() );
}

inline float clamp(float x, float min, float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}



inline glm::vec3 random_vec() {
    return glm::vec3(random_float(), random_float(), random_float());
}

inline glm::vec3 random_vec(float min, float max) {
    return glm::vec3(random_float(min, max), random_float(min, max), random_float(min, max));
}


inline bool near_zero(glm::vec3 e) {
    // Return true if the vector is close to zero in all dimensions.
    const float s = 0.000000001f;
    return (fabs(e.x) < s) && (fabs(e.y) < s) && (fabs(e.z) < s);
}





class perlin {
    public:
        perlin() {
            for (int i = 0; i < point_count; ++i) {
                ranvec.push_back(glm::normalize(random_vec(-1, 1)));
            }
            //cout << "perlin 0: " << ranvec.at(0);
            perm_x = perlin_generate_perm();
            perm_y = perlin_generate_perm();
            perm_z = perlin_generate_perm();
        }

        ~perlin() {
           
        }

        float noise(const glm::vec3& p) const {
            auto u = p.x - floor(p.x);
            auto v = p.y - floor(p.y);
            auto w = p.z - floor(p.z);
            auto i = (int)(floor(p.x));
            auto j = (int)(floor(p.y));
            auto k = (int)(floor(p.z));
            glm::vec3 c[2][2][2];

            for (int di=0; di < 2; di++)
                for (int dj=0; dj < 2; dj++)
                    for (int dk=0; dk < 2; dk++)
                        c[di][dj][dk] = ranvec.at( perm_x[(i+di) & 255] ^ perm_y[(j+dj) & 255] ^ perm_z[(k+dk) & 255]  );

            return perlin_interp(c, u, v, w);
        }

        float turb(const glm::vec3& p, int depth=7) const {
            auto accum = 0.0;
            auto temp_p = p;
            auto weight = 1.0;

            for (int i = 0; i < depth; i++) {
                accum += weight * noise(temp_p);
                weight *= 0.5;
                temp_p *= 2;
            }

            return fabs(accum);
        }

    
        static const int point_count = 256;
        vector<glm::vec3> ranvec;
        vector<int> perm_x;
        vector<int> perm_y;
        vector<int> perm_z;

        vector<int> perlin_generate_perm() { //static int*
            vector<int> p; //burda kaldým

            for (int i = 0; i < point_count; i++)
                p.push_back(i);  

            permute(p, point_count);

            return p;
        }

        static void permute(vector<int>& p, int n) {
            for (int i = n-1; i > 0; i--) {
                int target = random_int(0,i);
                int tmp = p[i];
                p[i] = p[target];
                p[target] = tmp;
            }
        }

        static float perlin_interp(glm::vec3 c[2][2][2], float u, float v, float w) {
            auto uu = u*u*(3-2*u);
            auto vv = v*v*(3-2*v);
            auto ww = w*w*(3-2*w);
            auto accum = 0.0;

            for (int i=0; i < 2; i++)
                for (int j=0; j < 2; j++)
                    for (int k=0; k < 2; k++) {
                        glm::vec3 weight_v(u-i, v-j, w-k);
                        accum += (i*uu + (1-i)*(1-uu))* (j*vv + (1-j)*(1-vv))* (k*ww + (1-k)*(1-ww))*dot(c[i][j][k], weight_v);
                    }

            return accum;
        }
};



#endif