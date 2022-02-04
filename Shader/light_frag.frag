#version 330 core

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

in vec3 our_color;
in vec2 tex_coord;
in vec3 vertex_position;
in vec3 our_normal;
in vec3 frag_pos; 

out vec4 frag_color;
uniform sampler2D our_texture;


void main()
{    
    //ambient
    vec3 ambient1 = obj.ambient_strength * obj.ambient;
    //vec3 result = ambient1 * ourColor;
    //diffuse
    vec3 norm = normalize(our_normal);
    //vec3 norm = ourNormal;
    vec3 light_dir = normalize(obj.light_pos - frag_pos); 
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * obj.light_color;

    //specular
    vec3 view_dir = normalize(obj.view_pos - frag_pos);
    vec3 reflect_dir = reflect(-1*light_dir, norm);  
    float spec = pow(   max (  dot(view_dir, reflect_dir)   , 0.0),   16 );
    vec3 specular = obj.specular_strength * spec * obj.light_color;  
    vec3 result = (ambient1 + diffuse + specular) * our_color;

    //frag_color = texture(our_texture, tex_coord) *vec4(result, 1.0);
    //frag_color = texture(our_texture, tex_coord) * vec4(obj.light_color,1); 
    frag_color = texture(our_texture, tex_coord) * vec4(1,0,0,1); 


    
}