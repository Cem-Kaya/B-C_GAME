#ifndef OBJ_HPP
#define OBJ_HPP

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
//#include "texture.cpp"
#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>
const glm::vec3 term = glm::vec3(1, 2, 1);

struct TransformInfo
{
public:
	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 rotation; // degrees

	

	glm::mat4 getTransformMatrix()
	{
		return
			glm::translate(glm::mat4(1.0f), this->position) 
			* glm::toMat4(glm::quat(rotation)) 
			* glm::scale(glm::mat4(1.0f), this->scale);
	}
};

class GameObj3D: public drawable
{
public:
	GameObj3D( VAO&  model, ShaderProgram& sh_prog);
	ShaderProgram& shad_prog;
	virtual void draw() override;
	void load(VAO* model);

	void translate(float x, float y, float z);
	void scale(float x, float y, float z);
	void rotate(float x, float y, float z); // degrees

	glm::vec3 position();
	glm::vec3 scale();
	glm::vec3 rotation();
	glm::mat4 getTransform();
	
	object_properties obj_properties;


	// physics & collusion 
	//physics
	glm::vec3 current_momentum;
	glm::vec3 force;
	glm::vec3 terminal_momentum;
	bool is_not_movable = true;
	int health = 1;
	bool is_hit = false;

	virtual bool get_movable() { return is_not_movable; }
	virtual void set_movable(bool in) { is_not_movable = in; }
	virtual glm::vec3 get_current_momentum() {return current_momentum;}
	virtual void set_current_momentum(glm::vec3 momentum_awakens) {current_momentum = momentum_awakens;}
	virtual glm::vec3 get_force() {return force;}
	virtual void set_force(glm::vec3 force_awakens) { force = force_awakens; }
	virtual void set_terminal_momentum(glm::vec3 terminal_awakens) { terminal_momentum = terminal_awakens; }
	
	//Gameplay Functions
	void health_down(){health--;}
	void health_up(){health++;}
	void health_up(int set){health += set;}
	void health_set(int set){health = set;}
	int get_health(){return health;}
	void got_hit(){is_hit = true;}
	void not_hit(){is_hit = false;}
	bool get_is_hit(){return is_hit;}



	virtual float get_max_x() { return max_point.x; }
	virtual float get_min_x() { return min_point.x; }
	virtual float get_max_y() { return max_point.y; }
	virtual float get_min_y() { return min_point.y; }
	virtual float get_max_z() { return max_point.z; }
	virtual float get_min_z() { return min_point.z; }
	virtual glm::vec3 get_origin();


	//collusion
	glm::vec3 _front = glm::vec3(0, 0, -1);
	glm::vec3 _up = glm::vec3(0, 1, 0);
	glm::vec3 _right = glm::vec3(-1, 0, 0);

	virtual void set_collider();
	glm::vec3 max_point;
	glm::vec3 min_point;
	glm::vec3 center;

	virtual glm::vec3 get_min(){return min_point;}
	virtual glm::vec3 get_max(){return max_point;}

	virtual int intersect(drawable* other);


	// for box
	glm::vec3 init_x, init_y, init_z;
	glm::vec3 to_x, to_y, to_z;

	void update_minmax();

	virtual void physics_update(float delta_time );

	////////////////////////////////////////////////
	//functions regarding obj_properties
	void set_obj_data(glm::mat4 projection, glm::mat4 model, glm::mat4 view, glm::vec3 ambient, float ambient_strength, glm::vec3 light_pos, glm::vec3 light_color, glm::vec3 view_pos, float specular_strength);
	void set_light_pos(glm::vec3 new_light_pos){obj_properties.light_pos = new_light_pos;}
	void set_view_pos(glm::vec3 new_view_pos) { obj_properties.view_pos = new_view_pos; }
	
	void set_perspective(glm::mat4 new_pers_mat) { obj_properties.projection = new_pers_mat; }
	void set_view(glm::mat4 new_view_mat) { obj_properties.view = new_view_mat; }
	glm::vec3 get_scale(){return transformInfo.scale;} //buraya ekledim!!!

	void transfer_obj_data();

	VAO* model3d = nullptr;
	TransformInfo transformInfo;
	string name = "default";
	virtual string get_name() { return name; };
	virtual void set_name(string new_name ) {  name = new_name; };

};
 
 glm::vec3 GameObj3D::get_origin() {
	update_minmax();
	return (min_point + max_point) / 2.0f;
 }

int GameObj3D::intersect(drawable * other) {
	if (get_min_x() <= other->get_max_x() && get_max_x() >= other->get_min_x() &&
			get_min_y() <= other->get_max_y() && get_max_y() >= other->get_min_y() &&
			get_min_z() <= other->get_max_z() && get_max_z() >= other->get_min_z()) {
		return 1;
	}
	else {

		return 0; // no bang bang boom 
	}

}


void GameObj3D::physics_update(float delta_time ) {
	if (!is_not_movable) {
		current_momentum += force* delta_time;

		glm::vec3 tran_vec;
		if (current_momentum.x > 0) {
			if (current_momentum.x < terminal_momentum.x) {
				tran_vec.x = position().x + current_momentum.x;
			}
			else {
				tran_vec.x = position().x + terminal_momentum.x;
				current_momentum.x = terminal_momentum.x;

			}
		}
		else {
			if (abs(current_momentum.x) < terminal_momentum.x) {
				tran_vec.x = position().x + current_momentum.x;
			}
			else {
				tran_vec.x = position().x - terminal_momentum.x;
				current_momentum.x =   -terminal_momentum.x;
			}
		}
			 

		if (current_momentum.y > 0) {
			if (current_momentum.y < terminal_momentum.y) {
				tran_vec.y = position().y + current_momentum.y;
			}
			else {
				tran_vec.y = position().y + terminal_momentum.y;
				current_momentum.y = terminal_momentum.y;
			}
		}
		else{
			if (abs(current_momentum.y) < terminal_momentum.y) {
				tran_vec.y = position().y + current_momentum.y;
			}
			else {
				tran_vec.y = position().y - terminal_momentum.y;
				current_momentum.y = -terminal_momentum.y;
			}
		}

		
		if (current_momentum.z > 0) {
			if (current_momentum.z < terminal_momentum.z) {
				tran_vec.z = position().z + current_momentum.z;
			}
			else {
				tran_vec.z = position().z + terminal_momentum.z;
				current_momentum.z = terminal_momentum.z;

			}
		}
		else {
			if (abs(current_momentum.z) < terminal_momentum.z) {
				tran_vec.z = position().z + current_momentum.z;
			}
			else {
				tran_vec.z = position().z - terminal_momentum.z;
				current_momentum.z = -terminal_momentum.z;
			}
		}


		translate(tran_vec.x, tran_vec.y, tran_vec.z);
	}
	else {
		current_momentum = glm::vec3(0, 0, 0);
	}
}








void GameObj3D::set_collider()
{
	
	max_point.x = std::numeric_limits<float>::min(), min_point.x = std::numeric_limits<float>::max();
	max_point.y = std::numeric_limits<float>::min(), min_point.y = std::numeric_limits<float>::max();
	max_point.z = std::numeric_limits<float>::min(), min_point.z = std::numeric_limits<float>::max();


	for (std::vector<glm::vec3>::iterator t = this->model3d->vao_positions.begin(); t != this->model3d->vao_positions.end(); ++t) {
		float x = t->x, y = t->y, z = t->z;
		if (x > max_point.x) max_point.x = x;
		else if (x < min_point.x) min_point.x = x;
		if (y > max_point.y) max_point.y = y;
		else if (y < min_point.y) min_point.y = y;
		if (z > max_point.z) max_point.z = z;
		else if (z < min_point.z) min_point.z = z;
	}
	float xD = (max_point.x - min_point.x) * 0.5, yD = (max_point.y - min_point.y) * 0.5, zD = (max_point.z- min_point.z) * 0.5;

	center = glm::vec3(min_point.x + xD, min_point.y + yD, min_point.z + zD);
	to_x = glm::vec3(xD, 0, 0);
	to_y = glm::vec3(0, yD, 0);
	to_z = glm::vec3(0, 0, zD);
	init_x = to_x;
	init_y = to_y;
	init_z = to_z;

	// for box
		
	update_minmax();

}
void GameObj3D::update_minmax() {
	float max_XD = (to_x.x >= 0 ? to_x.x : -to_x.x) + (to_y.x >= 0 ? to_y.x : -to_y.x) + (to_z.x >= 0 ? to_z.x : -to_z.x);
	float max_YD = (to_x.y >= 0 ? to_x.y : -to_x.y) + (to_y.y >= 0 ? to_y.y : -to_y.y) + (to_z.y >= 0 ? to_z.y : -to_z.y);
	float max_ZD = (to_x.z >= 0 ? to_x.z : -to_x.z) + (to_y.z >= 0 ? to_y.z : -to_y.z) + (to_z.z >= 0 ? to_z.z : -to_z.z);

	max_point.x = center.x + max_XD;
	min_point.x = center.x - max_XD;
	max_point.y = center.y + max_YD;
	min_point.y = center.y - max_YD;
	max_point.z = center.z + max_ZD;
	min_point.z = center.z - max_ZD;
};

GameObj3D::GameObj3D( VAO&   model, ShaderProgram& sh_prog ):shad_prog(sh_prog)
{
	terminal_momentum = term;
	current_momentum = glm::vec3(0);
	obj_properties = object_properties();	
	max_point = model.max_point;
	min_point = model.min_point;
	this->transformInfo.position = glm::vec3(0);
	center = (max_point + min_point ) /2.0f;
	this->transformInfo.scale = glm::vec3(1);
	this->transformInfo.rotation = glm::vec3(0);

	this -> model3d = &model;

	// physics & collusion 
	//float xD = (max_point.x - min_point.x) * 0.5, yD = (max_point.y - min_point.y) * 0.5, zD = (max_point.z- min_point.z) * 0.5;

	//center = glm::vec3(min_point.x + xD, min_point.y + yD, min_point.z + zD);
	//to_x = glm::vec3(xD, 0, 0);
	//to_y = glm::vec3(0, yD, 0);
	//to_z = glm::vec3(0, 0, zD);
	//init_x = to_x;
	//init_y = to_y;
	//init_z = to_z;

	//// for box
		
	//update_minmax();


	set_collider();
	

}

void GameObj3D::set_obj_data(
	glm::mat4 projection = glm::perspective(glm::radians(70.0f), 800.0f / 600.0f, 0.1f, 5000.0f),
	glm::mat4 model = glm::mat4(1), 
	glm::mat4 view = glm::mat4(1),
	glm::vec3 ambient =  glm::vec3(0.1, 0.25, 0.2),
	float ambient_strength = 0.25f,
	glm::vec3 light_pos = glm::vec3(0, 0, 0),
	glm::vec3 light_color= glm::vec3(1,1,1),
	glm::vec3 view_pos = glm::vec3(1, 1, 1) ,
	float specular_strength = 0.3f) {


	obj_properties.projection = projection;
	obj_properties.model = this->getTransform(); // if bug use model 
	obj_properties.view = view;
	obj_properties.ambient = ambient;
	obj_properties.ambient_strength = ambient_strength;
	obj_properties.light_pos = light_pos;
	obj_properties.light_color = light_color;
	obj_properties.view_pos = view_pos;
	obj_properties.specular_strength = specular_strength;
}


void GameObj3D::transfer_obj_data()
{
	shad_prog.set_shader(obj_properties);
}


void GameObj3D::draw()
{
	shad_prog.use();
	//set_obj_data();
	obj_properties.model = this->getTransform(); // if bug use model ; 
	transfer_obj_data();
	this->model3d->draw( this->getTransform()  );
}

void GameObj3D::load(VAO*model)
{
	delete (this -> model3d);
	this->model3d = model3d;
};

void GameObj3D::translate(float x, float y, float z)
{
	
	const glm::vec3 pos_prev = this->transformInfo.position;
	center += glm::vec3(x - pos_prev.x, y - pos_prev.y, z - pos_prev.z);

	this->transformInfo.position = glm::vec3(x, y, z);

	update_minmax();

}

void GameObj3D::scale(float x, float y, float z)
{

	this->transformInfo.scale = glm::vec3(x, y, z);

	// update collider
	to_x = init_x * x;
	to_y = init_y * y;
	to_z = init_z * z;

	update_minmax();

}

void GameObj3D::rotate(float x, float y, float z)
{


	const glm::vec3 rt = glm::radians(glm::vec3(x, y, z));
	this->transformInfo.rotation = rt;

	glm::mat4 trans = glm::toMat4(glm::quat(rt));
	_up = glm::vec3(trans * glm::vec4(glm::vec3(0, 1, 0), 1));
	_front = glm::vec3(trans * glm::vec4(glm::vec3(0, 0, -1), 1));
	_right = glm::cross(_front, _up);

	update_minmax();

}

glm::vec3 GameObj3D::position()
{
	return this->transformInfo.position;
}

glm::vec3 GameObj3D::scale()
{
	return this->transformInfo.scale;
}

glm::vec3 GameObj3D::rotation()
{
	return this->transformInfo.rotation;
}

glm::mat4 GameObj3D::getTransform() {
	return this->transformInfo.getTransformMatrix();
}


#endif