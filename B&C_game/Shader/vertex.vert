#version 330 core

layout (location = 0) in vec3 apos;
layout (location = 1) in vec3 acolor;
layout (location = 2) in vec2 atex_coord;
layout (location = 3) in vec3 anormal;

struct object_properties {
	//matrix
	mat4 projection;
	mat4 model;
	mat4 view;	
	//ambient
	vec3 ambient;
	float ambient_strength;	
	//color and light
	vec3 light_pos;
	vec3 light_color;
	vec3 view_pos;
	//specular
	float specular_strength;
}; 

uniform object_properties obj;

out vec3 vertex_position;
out vec3 our_color;
out vec2 tex_coord;
out vec3 our_normal;
out vec3 frag_pos;

/*out gl_PerVertex
{
  vec4 gl_Position;
  float gl_ClipDistance[3];
};
*/

void main()
{
    gl_Position = obj.projection * obj.view * obj.model *vec4(apos, 1.0);
    vertex_position = apos;
    our_color = acolor;
    tex_coord = atex_coord;
    our_normal = mat3(transpose(inverse(obj.model))) * anormal;  
    //ourNormal = aNormal;
    frag_pos = vec3(obj.model * vec4(apos, 1.0));
}


