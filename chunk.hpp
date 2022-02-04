#ifndef CHUNK_H
#define CHUNK_H

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
#include "drawable.hpp"
#include "Object.hpp"
#include "perlin.hpp"
#include "helper.hpp"
 
#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include<cmath>


using namespace std;

class chunk
{
public:
	
	chunk();
	vector<drawable*> draw_list ;
	drawable* LOD;

	~chunk();
	void draw_chunk();

	virtual void physics_update(float delta_time);
	void add_to_chunk(drawable* chonky);
	void remove_from_chunk(drawable* chonky);
	drawable* get_from_chunk(drawable* chonky);
	void chunk_update(glm::mat4 view, glm::vec3 view_pos, glm::vec3 light_pos);
	drawable* if_not_fit(int x, int y, int size);
	bool is_there(drawable* chonky);
	void draw_lod();
	void lod_update(glm::mat4 view, glm::vec3 view_pos, glm::vec3 light_pos);
};

bool chunk::is_there(drawable* chonky) {
	for (auto tmp : draw_list) {
		if (tmp == chonky) {
			return true;
		}		
	}
	return false;	
}



void chunk::lod_update(glm::mat4 view, glm::vec3 view_pos, glm::vec3 light_pos)
{
	LOD->set_view(view);		
	LOD->set_view_pos(view_pos);
	LOD->set_light_pos(light_pos);
	LOD->set_perspective(glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 5000.0f));
}

void chunk::draw_lod()
{ 
	LOD->draw();

}

chunk::chunk()
{
	
	
}

chunk::~chunk()
{
	for (auto& chonky : draw_list) {
		delete chonky;
	}
}


void chunk::chunk_update(glm::mat4 view, glm::vec3 view_pos, glm::vec3 light_pos){
	for (auto chonky : draw_list) {
		chonky->set_view(view);		
		chonky->set_view_pos(view_pos);
		chonky->set_light_pos(light_pos);
		chonky->set_perspective(glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 5000.0f));
		//chonky->
	}	
}

void chunk::draw_chunk()
{
	for(int i = 0; i < draw_list.size();i++){
		//cout << "Chunk's minpoint: " << draw_list.at(i)->get_min() << endl;
		//cout << "Chunk's maxpoint: " << draw_list.at(i)->get_max() << endl << endl;
		draw_list.at(i)->draw();
	}
	

	/*for(auto& chonky: draw_list){
		chonky->draw();		
	}
	*/

}

drawable* chunk::if_not_fit(int x, int y, int size) {
		
	//for (auto d : draw_list) {
	if (draw_list.size() == 3) {
		//cout << "draw_list size: " << draw_list.size() << endl << endl;
	}
	for (int i = 0; i < draw_list.size(); i++) {
		drawable* d = draw_list.at(i); //[] 
		/*if (d->get_name() == "sph") {
			cout << "sph_x: " << d->position().x << " sph_z :" << d->position().z <<endl;
			cout << "input x: " << x << " input y(z): " << y << "input size: " << size << endl << endl << endl;
		}*/

		
		float my_x = d->get_origin().x;
		float my_z = d->get_origin().z;

		//if (d->get_name() == "sph") {
		//	cout << "sph_x: " << d->get_origin().x << " sph_z :" << d->get_origin().z << endl;
		//}

		if (  ( ( x < my_x) && ( my_x < ( (x + size)  ) )) ) { //!' ý yeni koyduk
			if (  (   y < my_z) && ( my_z < (y+ size)  ) ) {
				;
			}
			else {
				auto tmp = get_from_chunk(d);
				return tmp ;
			}
		}
		else {
			auto tmp = get_from_chunk(d);
			return tmp ;
		}
	}
	return nullptr;
}

void chunk::physics_update(float delta_time)
{

	for (int i = 0;i < draw_list.size();i++) {
		draw_list.at(i)->physics_update(delta_time);
		for (int j = i +1 ; j < draw_list.size(); j++) {
			short res = draw_list.at(i)->intersect(draw_list.at(j));
			
			
			auto a = draw_list.at(i);
			auto b = draw_list.at(j);


			if (res > 0) {
				//cout << "a and b pos: a.x: " << a->position().x << " a.z: " << a->position().z << endl << "b.x: " << b->position().x << " b.z: " << b->position().z << endl;
				//cout << "res'ten girdi yani collision var" << endl;
				if ( (a->get_name() == "hero" && b->get_name() == "bad")  ) {
					a->got_hit();
					b->got_hit();
				}
				if ( (a->get_name() == "bad" && b->get_name() == "hero")) {
					a->got_hit();
					b->got_hit();
				}
				
				if(a->get_name() == "bullet" && b->get_name() == "bad" ){
					b->got_hit();
					a->got_hit();
				}

				if(a->get_name() == "bad" && b->get_name() == "bullet"){
					b->got_hit();
					a->got_hit();
				}
				if ( b->get_name() == "bullet") {
					b->got_hit();					
				}
				if (a->get_name() == "bullet") {
					a->got_hit();
				}
				if ( b->get_name() == "party" && a->get_name() == "default") {
					b->got_hit();					
				}
				if (a->get_name() == "party" && b->get_name() == "default") {
					a->got_hit();
				}
			
				//cout << " b speed: " << b->get_current_momentum().y <<  " b force: " << b->get_force().y << "   " << " a speed: " << a->get_current_momentum().y << " a force: " << a->get_force().y << "  " << endl;
				glm::vec3 relative = b->get_current_momentum() - a->get_current_momentum() ;
				glm::vec3 norm = glm::vec3(0,0,0);
				float e = 1.0f; // restatuion aka bouncyness parameter 
				// callculate normal 
				


				bool in_x=  a->get_min_x() <= b->get_max_x() && a->get_max_x() >= b->get_min_x();
				bool in_y = a->get_min_y() <= b->get_max_y() && a->get_max_y() >= b->get_min_y();								
				bool in_z = a->get_min_z() <= b->get_max_z() && a->get_max_z() >= b->get_min_z();


				float a_size_x = fabs(a->get_max_x()) - fabs(a->get_min_x());
				float a_size_y = fabs(a->get_max_y()) - fabs(a->get_min_y());
				float a_size_z = fabs(a->get_max_z()) - fabs(a->get_min_z());
				a_size_x = a_size_x / 2.0f;
				a_size_y = a_size_y / 2.0f;
				a_size_z = a_size_z / 2.0f;


				float b_size_x = fabs(b->get_max_x()) - fabs(b->get_min_x());
				float b_size_y = fabs(b->get_max_y()) - fabs(b->get_min_y());
				float b_size_z = fabs(b->get_max_z()) - fabs(b->get_min_z());
				b_size_x = b_size_x / 2.0f;
				b_size_y = b_size_y / 2.0f;
				b_size_z = b_size_z / 2.0f;
				
				glm::vec3 a_center = glm::vec3(a->get_min_x() + a_size_x, a->get_min_y() + a_size_y, a->get_min_z() + a_size_z);

				glm::vec3 b_center = glm::vec3(b->get_min_x() + b_size_x, b->get_min_y() + b_size_y, b->get_min_z() + b_size_z);
				if (in_x) {					
					norm += (a_center.x < b_center.x)? glm::vec3(1, 0, 0) : glm::vec3(-1, 0, 0);
				}				
				if (in_y) {
					norm += (a_center.y < b_center.y) ? glm::vec3(0, 1, 0) : glm::vec3(0, -1, 0);
				}
				if (in_z){
					norm += (a_center.z < b_center.z) ? glm::vec3(0, 0, 1) : glm::vec3(0, 0, -1);
				}
				if( ! (in_x&& in_y&& in_z))  {
					//cout << "collusion error " << endl;
				}

				float j = -(1.0f + e) * glm::dot(relative, norm); //2.si normal olacak


				glm::vec3 move_norm = norm * 3.75f ;
				if (a->get_movable() && b->get_movable()) { //00
					;// pass
					//cout << "error station obj collude " << endl;
				}
				else if (!a->get_movable() && b->get_movable()) { //10
					j /= (1.0f / 100000000) + (1.0f / 1);
					
					a->translate(a->position().x - move_norm.x, a->position().y - move_norm.y, a->position().z - move_norm.z);

				}
				else if (a->get_movable() && ! b->get_movable()) { //10
					j /= (1.0f / 100000000 ) + (1.0f / 1);
					b->translate(b->position().x + move_norm.x, b->position().y + move_norm.y, b->position().z + move_norm.z);
				}
				else if (!a->get_movable() && !b->get_movable()) { //10
					j /= (1.0f / 2 ) + (1.0f / 1);
					b->translate(b->position().x +  move_norm.x, b->position().y + move_norm.y, b->position().z + move_norm.z);
					a->translate(a->position().x -  move_norm.x, a->position().y - move_norm.y, a->position().z - move_norm.z);
				}




				


				



				glm::vec3 a_momentum = a->get_current_momentum(); 
				glm::vec3 b_momentum = b->get_current_momentum();



				//cout << "a_initial momentum: " << a->get_current_momentum().y << "   b_initial momentum: " << b->get_current_momentum().y << endl;
				
				//cout << "1 a_momentum : " << a_momentum.y << endl;
				//cout << "1 norm : " << norm.y << endl;
				a_momentum =  a_momentum - j * norm;
				//cout << "2 a_momentum : " << a_momentum.y << endl;

				b_momentum = b_momentum + j * norm;

				if (a_momentum.length()< 0.1f ) {
					a_momentum = a_momentum * (0.2f/ a_momentum.length());
				}
				
				if(b_momentum.length() < 0.1f){
					b_momentum = b_momentum* (0.2f/a_momentum.length());
				}

				if (norm.x == 0 && norm.y == 0 && norm.z == 0) {
					cout << "error in norm " << endl;
				}
				glm::vec3 ranvec = glm::vec3( random_float(-0.5f, 0.5f) , random_float(0.05f, 0.5f), random_float(-0.5f, 0.5f)); //randomness
				b_momentum += ranvec;
				a_momentum += ranvec;

				a->set_current_momentum(a_momentum);
				b->set_current_momentum(b_momentum);

				//cout << "a_next momentum: " << a->get_current_momentum().y << "   b_next momentum: " << b->get_current_momentum().y << ""  <<endl;

				//a->physics_update(delta_time);
				//b->physics_update(delta_time);
				//cout << j << endl;
				//cout << "a_next momentum2: " << a->get_current_momentum().y << "   b_next momentum2: " << b->get_current_momentum().y << "     " << endl << endl <<endl;


				/*
				auto a_size_x = abs(a->get_max_x()) - abs(a->get_min_x());
				auto a_size_y = abs(a->get_max_y()) - abs(a->get_min_y());
				auto a_size_z = abs(a->get_max_z()) - abs(a->get_min_z());


				auto b_size_x = abs(b->get_max_x())- abs(b->get_min_x());
				auto b_size_y = abs(b->get_max_y())- abs(b->get_min_y());
				auto b_size_z = abs(b->get_max_z()) - abs(b->get_min_z());

				// collision x-axis?
				bool collision_x = a->position().x + a_size_x >= b->position().x &&
					b->position().x + b_size_x >= a->position().x;
				// collision y-axis?
				bool collision_y = a->position().y + a_size_y >= b->position().y &&
					b->position().y + b_size_y >= a->position().y;
				// collision z-axis?
				bool collision_z = a->position().z + a_size_z >= b->position().z &&
					b->position().z + b_size_z >= a->position().z;





				if (a->get_movable() && b->get_movable()) { //00
					;// pass
					cout << "error station obj collude " << endl;
				}
				else if (!a->get_movable() && b->get_movable()) { //10
					//a->set_force(glm::vec3(0,0,0));
					if (collision_x) {
						a->set_current_momentum( glm::vec3(a->get_current_momentum().x , -1.0f*a->get_current_momentum().y, -1.0f * a->get_current_momentum().z));
					}
					else if (collision_y) {
						if (a->get_max_y()  < b->get_max_y() ) {
							//alttan carpti
							a->translate(a->position().x, a->position().y-7 , a->position().z);
						}
						else {// ustten carpti
							a->translate(a->position().x, a->position().y+7  , a->position().z);

						}
						a->set_current_momentum(glm::vec3(a->get_current_momentum().x * -1.0f, a->get_current_momentum().y, -1.0f * a->get_current_momentum().z));

					}
					else if (collision_z) {
						a->set_current_momentum(glm::vec3(-1.0f * a->get_current_momentum().x, -1.0f * a->get_current_momentum().y,  a->get_current_momentum().z));
					}
					else {
						a->set_current_momentum(-1.0f* a->get_current_momentum() );

					}

					a->physics_update(delta_time);

				}
				else if (a->get_movable() && !b->get_movable()) { //01
					//b->set_force(glm::vec3(0, 0, 0));
					if (collision_x) {
						b->set_current_momentum(glm::vec3(b->get_current_momentum().x, -1.0f * b->get_current_momentum().y, -1.0f * b->get_current_momentum().z));
					}
					else if (collision_y) {
						if (b->get_max_y()  < a->get_max_y() ) {
							//alttan carpti
							b->translate(b->position().x, b->position().y-7 , b->position().z);
						}
						else {// ustten carpti
							b->translate(b->position().x, b->position().y+7  , b->position().z);
						}

						b->set_current_momentum(glm::vec3(b->get_current_momentum().x * -1.0f, b->get_current_momentum().y, -1.0f * b->get_current_momentum().z));
					}
					else if (collision_z) {
						b->set_current_momentum(glm::vec3(-1.0f * b->get_current_momentum().x, -1.0f * b->get_current_momentum().y, b->get_current_momentum().z));
					}
					else {
						b->set_current_momentum(-1.0f * b->get_current_momentum());
					}
					b->physics_update(delta_time);
				}
				else if (!a->get_movable() && !b->get_movable()) { //11
					//a->set_force(glm::vec3(0, 0, 0));
					if (collision_x) {
						a->set_current_momentum(glm::vec3(a->get_current_momentum().x, -1.0f * a->get_current_momentum().y, -1.0f * a->get_current_momentum().z));
						b->set_current_momentum(glm::vec3(b->get_current_momentum().x, -1.0f * b->get_current_momentum().y, -1.0f * b->get_current_momentum().z));
					}
					else if (collision_y) {
						a->set_current_momentum(glm::vec3(a->get_current_momentum().x * -1.0f, a->get_current_momentum().y, -1.0f * a->get_current_momentum().z));
						b->set_current_momentum(glm::vec3(b->get_current_momentum().x * -1.0f, b->get_current_momentum().y, -1.0f * b->get_current_momentum().z));
					}
					else if (collision_z) {
						a->set_current_momentum(glm::vec3(-1.0f * a->get_current_momentum().x, -1.0f * a->get_current_momentum().y, a->get_current_momentum().z));
						b->set_current_momentum(glm::vec3(-1.0f * b->get_current_momentum().x, -1.0f * b->get_current_momentum().y, b->get_current_momentum().z));
					}
					else {
						a->set_current_momentum(-1.0f * a->get_current_momentum());
						b->set_current_momentum(-1.0f * b->get_current_momentum());
					}


					a->physics_update(delta_time);

					//b->set_force(glm::vec3(0, 0, 0));
					b->physics_update(delta_time);

				}
			}



			/*
				cout << " detected !! " << i << "  " << j  << " res : " << res << endl;
				// 00 // not posable stay still
				auto a = draw_list.at(i);
				auto b = draw_list.at(j);
				if (a->get_movable() && b->get_movable()) { //00
					// pass
				}
				else if (!a->get_movable() && b->get_movable()) { //10
					glm::vec3 tmp;
					if (res == 1) {
						tmp = glm::vec3(-a->get_current_momentum().x, 0, 0);
					}
					else if (res == 2) {
						tmp = glm::vec3(0, -a->get_current_momentum().y, 0);
					}
					else if (res == 3) {
						tmp = glm::vec3(0, 0, -a->get_current_momentum().z);
						// set force to less for exiss
					}
					a->translate(a->position().x + tmp.x
						, a->position().y + tmp.y
						, a->position().z + tmp.z);
				}
				else if (a->get_movable() && !b->get_movable()) { //01

					glm::vec3 tmp;
					if (res == 1) {
						tmp = glm::vec3(-b->get_current_momentum().x, 0, 0);
					}
					else if (res == 2) {
						tmp = glm::vec3(0, -b->get_current_momentum().y, 0);
					}
					else if (res == 3) {
						tmp = glm::vec3(0, 0, -b->get_current_momentum().z);
						// set force to less for exiss
					}
					b->translate(b->position().x + tmp.x
						, b->position().y + tmp.y
						, b->position().z + tmp.z);
				}
				else {	// 11 bounce both ?

					glm::vec3 tmp;
					if (res == 1) {
						tmp = glm::vec3(-a->get_current_momentum().x, 0, 0);
					}
					else if (res == 2) {
						tmp = glm::vec3(0, -a->get_current_momentum().y, 0);
					}
					else if (res == 3) {
						tmp = glm::vec3(0, 0, -a->get_current_momentum().z);
						// set force to less for exiss
					}
					a->translate(a->position().x + tmp.x
						, a->position().y + tmp.y
						, a->position().z + tmp.z);

					if (res == 1) {
						tmp = glm::vec3(-b->get_current_momentum().x, 0, 0);
					}
					else if (res == 2) {
						tmp = glm::vec3(0, -b->get_current_momentum().y, 0);
					}
					else if (res == 3) {
						tmp = glm::vec3(0, 0, -b->get_current_momentum().z);
						// set force to less for exiss
					}
					b->translate(b->position().x + tmp.x
						, b->position().y + tmp.y
						, b->position().z + tmp.z);
				}

				//01 move 2
			}*/
			}
		}
	}
}

void chunk::add_to_chunk(drawable* chonky)
{
	draw_list.push_back(chonky);
	
}

drawable* chunk::get_from_chunk(drawable* chonky){
	for (int i = 0; i < draw_list.size(); i++) {
		
		if (draw_list.at(i) == chonky) {
			auto tmp  = draw_list.at(i);
			auto tmp2 = draw_list.at(draw_list.size()-1);
			draw_list.at(i) = tmp2;
			draw_list.at(draw_list.size() - 1)= tmp;
			//draw_list.pop_back();
			/*
			if ((i + 1) < draw_list.size()) {
				draw_list.at(i) = draw_list.at((i + 1));
				draw_list.at((i + 1)) = chonky;
			}
			*/
		}
	}
	drawable* tmp = draw_list.back();
	draw_list.pop_back();	
	return tmp;
	
}

void chunk::remove_from_chunk(drawable* chonky)
{
	for (int i = 0; i < draw_list.size(); i++) {
		if (draw_list.at(i) == chonky ) {	
			auto tmp  = draw_list.at(i);
			auto tmp2 = draw_list.at(draw_list.size()-1);
			draw_list.at(i) = tmp2;
			draw_list.at(draw_list.size() - 1)= tmp;
		}		
	}
	draw_list.pop_back();
	delete chonky;
	chonky = nullptr;
}
















#endif