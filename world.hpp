#ifndef WORLD_HPP
#define WORLD_HPP

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
#include "chunk.hpp"
#include "perlin.hpp"

#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>
#include <unordered_map>

using namespace std;
const int chunk_size= 256 ;
const int draw_chunk_depth =6 ;
const int lod_draw_chunk_depth = 16;

const int chunk_block_side = 6;

inline int num_normalizer(const float& num){
	if (num >= 0) {
		int x = (num);
		x = x / chunk_size;
		//x = x * chunk_size;
		return x;
	}
	else {
		int x = fabs(num);
		x = x / chunk_size;
		x++;
		//x = x * chunk_size;
		return -x;
	}
}


inline int def_biom(const int32_t &x , const int32_t &y ) {
	const int32_t trash =  0b00000000000000000000000000001000;
	const int32_t trashy = 0b00000000000000000000000000011000;

	bool first = ( x & trash)== trash ? true :false  ;
	bool second =( y & trashy)== trashy ? true :false 	;


	if (!first && !second){ //00 --> Orman 
		return 0;
		
	}
	else if(!first && second){ // 01 -->  Ova 
		return 1;
	}
	else if(first && !second){ // 10 --> karli
		return 2;
	}
	else{ //11 --> Desert
		return 3;
	}
}



class world {
public:
	unordered_map<int, unordered_map<int, chunk>> Quad_tree; // logical pointers !!!! for memory safe operation 
	unordered_map<int, unordered_map<int, bool>> Quad_init; 
	perlin my_noise;
	
	world();

	void init_chunk(int x, int y, ShaderProgram& sp);
	//
	
	void draw(glm::vec3 & camloc, ShaderProgram& sp, glm::mat4 view, glm::vec3 view_pos, glm::vec3 light_pos, float delta_time);

	void draw(unordered_map<int, unordered_map<int, bool>>&, int x, int y, const int&  start_x, const int& start_y,  ShaderProgram& sp, glm::mat4 view, glm::vec3 view_pos, glm::vec3 light_pos, float delta_time );
	void insert(drawable* input, ShaderProgram& sp);
	bool remove_from(drawable* input);
};


world::world(){
	my_noise = perlin();
}

inline void world::init_chunk(int x, int y, ShaderProgram& sp)
{
	Quad_init[x][y] = true;
	Quad_tree[x][y] = chunk();

	float side_len = 1 * (chunk_size / (10.0f * chunk_block_side));


	int biom= def_biom(x,y);
	if (biom == 0) { //dag with agac biom == 0

		float x_x = x * chunk_size + chunk_size / 2.0f ;
		float z_z = y * chunk_size + chunk_size / 2.0f;
		float perl_ = my_noise.noise(glm::vec3((x_x / 10000.0f + 0.0001f), 0.01f, (z_z / 10000.0f + 0.0001f)));	
		auto lod = new GameObj3D(*get_mesh("./Mesh/Ground1.obj", "./Texture/Baha_mountain_grass.jpg"), sp);
		lod->scale(chunk_size/10.0f - 0.05f, 15, chunk_size / 10.0f - 0.05f);
		lod->translate(x_x - 128, -170   + perl_ * 9000, z_z - 128);
		lod->set_obj_data();
		Quad_tree[x][y].LOD = lod ;
		
		for (int i = 0; i < 1; i++) { // z 
			for (int j = 0; j < chunk_block_side ; j++) { // y 
				for (int k = 0; k < chunk_block_side; k++) { // x
					int RR = rand() % 100;
					GameObj3D* wall;
					if (RR < 20) {
						if (RR < 10) {
							wall = new GameObj3D(*get_mesh("./Mesh/Ground_with_christmas.obj", "./Texture/Cem_tree.jpg"), sp);
						}
						else {
							wall = new GameObj3D(*get_mesh("./Mesh/Ground_with_christmas.obj", "./Texture/Baha_tree.jpg"), sp);
						}
					}
					else {
						wall = new GameObj3D(*get_mesh("./Mesh/Ground1.obj", "./Texture/Baha_mountain_grass.jpg"), sp);
					}


					
					wall->scale(side_len - 0.0005f, 4, side_len - 0.0005f);

					float xx = x * chunk_size +  side_len * k * 10;
					float zz = y * chunk_size + side_len * j * 10;

					float perl = my_noise.noise(glm::vec3((xx / 10000.0f + 0.0001f), 0.01f, (zz / 10000.0f + 0.0001f)));
					//float perl = random_float (1,50 ) ; 
					//cout << perl <<"with seed : "<< (xx / 10000.0f + 0.0001f) <<  endl;
					wall->translate(xx, -50 + 300 * i + perl * 9000, zz);
					wall->set_obj_data();
					Quad_tree[x][y].add_to_chunk(wall);
				}
			}
		}
	}else if (biom == 1) { //Yesil Ova

		float x_x = x * chunk_size + chunk_size / 2.0f ;
		float z_z = y * chunk_size + chunk_size / 2.0f;
		float perl_ = my_noise.noise(glm::vec3((x_x / 10000.0f + 0.0001f), 0.01f, (z_z / 10000.0f + 0.0001f)));	
		auto lod = new GameObj3D(*get_mesh("./Mesh/Ground1.obj", "./Texture/ground_0.jpg"), sp);
		lod->scale(chunk_size/10.0f - 0.05f, 15 , chunk_size / 10.0f - 0.05f);
		lod->translate(x_x -128   , -170  + perl_ * 9000, z_z - 128 );
		lod->set_obj_data();
		Quad_tree[x][y].LOD = lod ;

		for (int i = 0; i < 1; i++) { // z 
			for (int j = 0; j < chunk_block_side; j++) { // y 
				for (int k = 0; k < chunk_block_side; k++) { // x
					//int RR = rand() % 100;
					GameObj3D* wall;
					
					wall = new GameObj3D(*get_mesh("./Mesh/Ground1.obj", "./Texture/ground_0.jpg"), sp);


					float side_len = 1 * (chunk_size / (10.0f * chunk_block_side)); //10 blogumuzun default uzunlugu
					wall->scale(side_len - 0.0005f, 4, side_len - 0.0005f);

					float xx = x * chunk_size    +  side_len * k * 10;
					float zz = y * chunk_size    +  side_len * j * 10;

					float perl = my_noise.noise(glm::vec3((xx / 10000.0f + 0.0001f), 0.01f, (zz / 10000.0f + 0.0001f)));
					//float perl = random_float (1,50 ) ; 
					//cout << perl <<"with seed : "<< (xx / 10000.0f + 0.0001f) <<  endl;
					wall->translate(xx, -50 + 300 * i + perl * 9000, zz);
					wall->set_obj_data();
					Quad_tree[x][y].add_to_chunk(wall);
				}
			}
		}
	}
	else if (biom == 2) { // kar

		float x_x = x * chunk_size + chunk_size / 2.0f ;
		float z_z = y * chunk_size + chunk_size / 2.0f;
		float perl_ = my_noise.noise(glm::vec3((x_x / 10000.0f + 0.0001f), 0.01f, (z_z / 10000.0f + 0.0001f)));	
		auto lod = new GameObj3D(*get_mesh("./Mesh/Ground1.obj", "./Texture/Baha_snow.jpg"), sp);
		lod->scale(chunk_size/10.0f - 0.05f, 15, chunk_size / 10.0f - 0.05f);
		lod->translate(x_x- 128 , -170  + perl_ * 9000, z_z- 128);
		lod->set_obj_data();
		Quad_tree[x][y].LOD = lod ;
		
		for (int i = 0; i < 1; i++) { // z 
			for (int j = 0; j < chunk_block_side; j++) { // y 
				for (int k = 0; k < chunk_block_side; k++) { // x
					int RR = rand() % 100;
					GameObj3D* wall;
					if (RR < 30) {
						
						wall = new GameObj3D(*get_mesh("./Mesh/Ground_with_christmas.obj", "./Texture/Cem_dag_agac.jpg"), sp);
						
					}
					else {
						wall = new GameObj3D(*get_mesh("./Mesh/Ground1.obj", "./Texture/Baha_snow.jpg"), sp);
					}


					float side_len = 1 * (chunk_size / (10.0f * chunk_block_side)); //10 blogumuzun default uzunlugu
					wall->scale(side_len - 0.0005f, 4, side_len - 0.0005f);

					float xx = x * chunk_size +  side_len * k * 10;
					float zz = y * chunk_size + side_len * j * 10;

					float perl = my_noise.noise(glm::vec3((xx / 10000.0f + 0.0001f), 0.01f, (zz / 10000.0f + 0.0001f)));
					//float perl = random_float (1,50 ) ; 
					//cout << perl <<"with seed : "<< (xx / 10000.0f + 0.0001f) <<  endl;
					wall->translate(xx, -50 + 300 * i + perl * 9000, zz);
					wall->set_obj_data();
					Quad_tree[x][y].add_to_chunk(wall);
				}
			}
		}
	}
	else if (biom == 3) {  //col

		float x_x = x * chunk_size + chunk_size / 2.0f ;
		float z_z = y * chunk_size + chunk_size / 2.0f;
		float perl_ = my_noise.noise(glm::vec3((x_x / 10000.0f + 0.0001f), 0.01f, (z_z / 10000.0f + 0.0001f)));	
		auto lod = new GameObj3D(*get_mesh("./Mesh/Ground1.obj", "./Texture/col.jpg"), sp);
		lod->scale(chunk_size/10.0f - 0.05f, 15, chunk_size / 10.0f - 0.05f);
		lod->translate(x_x - 128 , -170  + perl_ * 9000, z_z - 128 );
		lod->set_obj_data();
		Quad_tree[x][y].LOD = lod ;

	for (int i = 0; i < 1; i++) { // z 
		for (int j = 0; j < chunk_block_side; j++) { // y 
			for (int k = 0; k < chunk_block_side; k++) { // x
				int RR = rand() % 100;
				GameObj3D* wall;
				if (RR < 2 ) {					
					wall = new GameObj3D(*get_mesh("./Mesh/Ground_with_col.obj", "./Texture/Baha_desert_tree.jpg"), sp);
				}			
				else {
					wall = new GameObj3D(*get_mesh("./Mesh/Ground1.obj", "./Texture/col.jpg"), sp);
				}


				float side_len = 1 * (chunk_size / (10.0f * chunk_block_side)); //10 blogumuzun default uzunlugu
				wall->scale(side_len - 0.0005f, 4, side_len - 0.0005f);

				float xx = x * chunk_size  + side_len * k * 10;
				float zz = y * chunk_size  + side_len * j * 10;

				float perl = my_noise.noise(glm::vec3((xx / 10000.0f + 0.0001f), 0.01f, (zz / 10000.0f + 0.0001f)));
				//float perl = random_float (1,50 ) ; 
				//cout << perl <<"with seed : "<< (xx / 10000.0f + 0.0001f) <<  endl;
				wall->translate(xx, -50 + 300 * i + perl * 9000, zz);
				wall->set_obj_data();
				Quad_tree[x][y].add_to_chunk(wall);
			}
		}
	}
	}

}

inline void world::draw(glm::vec3 & camloc, ShaderProgram& sp, glm::mat4 view, glm::vec3 view_pos, glm::vec3 light_pos ,  float delta_time )
{
	unordered_map<int, unordered_map<int, bool>>   already_drawn;
	// determine jurisdiction 
	int x = num_normalizer(camloc.x) ;
	int y = num_normalizer(camloc.z);
	//cout << "x: " << x << "  " << camloc.x << " " << view_pos.x << endl;
	//cout << "z: " << y << "  " << camloc.z << " " << view_pos.z << endl;
	
	// 57.3 => 50 //  59.99 ->500          
	//init
	
	if (!Quad_init[x][y]) {
		init_chunk(x, y, sp);
	}
	
	
	Quad_tree[x][y].chunk_update(view, view_pos, light_pos);
	//Quad_tree[x][y].physics_update(delta_time );
	//Quad_tree[x][y].draw_chunk();   
	drawable* shell_insert = Quad_tree[x][y].if_not_fit(chunk_size * x   , chunk_size * y  , chunk_size);	
	insert(shell_insert, sp); //en son ekledik collider i debuglarken


	already_drawn[x][y] = true;
	draw(already_drawn,x ,y , x , y , sp,  view,  view_pos,  light_pos,delta_time);
	//draw(already_drawn,0 ,0 , 0 , 0 , sp,  view,  view_pos,  light_pos,delta_time);

} 
inline void world::draw(unordered_map<int, unordered_map<int, bool>> & already_drawn ,int x ,int y, const int& start_x, const int& start_y , ShaderProgram& sp, glm::mat4 view, glm::vec3 view_pos, glm::vec3 light_pos , float delta_time)
{
	if ( sqrt ( pow(abs(start_x -x),2)+ pow(abs(start_y - y),2 )) < draw_chunk_depth ){
		if (!Quad_init[x][y]) {
			init_chunk(x, y, sp);
		}
		
		//Quad_tree[x][y].lod_update(view, view_pos, light_pos);

		already_drawn[x ][y ] = true;

		Quad_tree[x][y].chunk_update(view, view_pos, light_pos);
		Quad_tree[x][y].physics_update(delta_time);
		Quad_tree[x][y].draw_chunk();
		drawable* shell_insert = Quad_tree[x][y].if_not_fit(chunk_size * x  , chunk_size * y  , chunk_size);
		
		insert(shell_insert, sp); //en son ekledik collider i debuglarken


		if (!already_drawn[x][y + 1]) {
			//insert(Quad_tree[x][y+1].if_not_fit(x, y+1, chunk_size), sp);
			draw(already_drawn, x, y + 1, start_x, start_y, sp, view, view_pos, light_pos, delta_time);
		}
		if (!already_drawn[x][y - 1]) {
			//insert(Quad_tree[x][y-1].if_not_fit(x, y-1, chunk_size), sp);
			draw(already_drawn, x, y - 1, start_x, start_y, sp, view, view_pos, light_pos, delta_time);
		}
		if (!already_drawn[x - 1][y]) {
			//insert(Quad_tree[x-1][y].if_not_fit(x-1, y, chunk_size), sp);
			draw(already_drawn, x - 1, y, start_x, start_y, sp, view, view_pos, light_pos, delta_time);
		}
		if (!already_drawn[x + 1][y]) {
			//insert(Quad_tree[x+1][y].if_not_fit(x+1, y, chunk_size), sp);
			draw(already_drawn, x + 1, y, start_x, start_y, sp, view, view_pos, light_pos, delta_time);
		}
		if (!already_drawn[x-1][y-1]) {
			//insert(Quad_tree[x-1][y-1].if_not_fit(x-1, y-1, chunk_size), sp);
			draw(already_drawn, x - 1, y - 1, start_x, start_y , sp, view, view_pos, light_pos, delta_time);
		}
		if (!already_drawn[x-1][y+1]) {
			//insert(Quad_tree[x-1][y+1].if_not_fit(x-1, y+1, chunk_size), sp);
			draw(already_drawn, x - 1, y + 1, start_x, start_y , sp, view, view_pos, light_pos, delta_time);
		}
		if (!already_drawn[x+1][y-1]) {
			//insert(Quad_tree[x+1][y-1].if_not_fit(x+1, y-1, chunk_size), sp);
			draw(already_drawn, x + 1, y - 1, start_x, start_y , sp, view, view_pos, light_pos, delta_time);
		}
		if (!already_drawn[x+1][y+1]) {
			//insert(Quad_tree[x + 1][y + 1].if_not_fit(x + 1, y + 1, chunk_size), sp);
			draw(already_drawn, x + 1, y + 1, start_x, start_y , sp, view, view_pos, light_pos, delta_time);
		}
		
	}
	else if (sqrt(pow(abs(start_x - x), 2) + pow(abs(start_y - y), 2)) < lod_draw_chunk_depth) {
		if (!Quad_init[x][y]) {
			init_chunk(x, y, sp);
		}
		Quad_tree[x][y].lod_update(view, view_pos, light_pos);
		Quad_tree[x][y].draw_lod();
		already_drawn[x][y] = true;

		if (!already_drawn[x][y + 1]) {
			//insert(Quad_tree[x][y+1].if_not_fit(x, y+1, chunk_size), sp);
			draw(already_drawn, x, y + 1, start_x, start_y, sp, view, view_pos, light_pos, delta_time);
		}
		if (!already_drawn[x][y - 1]) {
			//insert(Quad_tree[x][y-1].if_not_fit(x, y-1, chunk_size), sp);
			draw(already_drawn, x, y - 1, start_x, start_y, sp, view, view_pos, light_pos, delta_time);
		}
		if (!already_drawn[x - 1][y]) {
			//insert(Quad_tree[x-1][y].if_not_fit(x-1, y, chunk_size), sp);
			draw(already_drawn, x - 1, y, start_x, start_y, sp, view, view_pos, light_pos, delta_time);
		}
		if (!already_drawn[x + 1][y]) {
			//insert(Quad_tree[x+1][y].if_not_fit(x+1, y, chunk_size), sp);
			draw(already_drawn, x + 1, y, start_x, start_y, sp, view, view_pos, light_pos, delta_time);
		}
		if (!already_drawn[x - 1][y - 1]) {
			//insert(Quad_tree[x-1][y-1].if_not_fit(x-1, y-1, chunk_size), sp);
			draw(already_drawn, x - 1, y - 1, start_x, start_y, sp, view, view_pos, light_pos, delta_time);
		}
		if (!already_drawn[x - 1][y + 1]) {
			//insert(Quad_tree[x-1][y+1].if_not_fit(x-1, y+1, chunk_size), sp);
			draw(already_drawn, x - 1, y + 1, start_x, start_y, sp, view, view_pos, light_pos, delta_time);
		}
		if (!already_drawn[x + 1][y - 1]) {
			//insert(Quad_tree[x+1][y-1].if_not_fit(x+1, y-1, chunk_size), sp);
			draw(already_drawn, x + 1, y - 1, start_x, start_y, sp, view, view_pos, light_pos, delta_time);
		}
		if (!already_drawn[x + 1][y + 1]) {
			//insert(Quad_tree[x + 1][y + 1].if_not_fit(x + 1, y + 1, chunk_size), sp);
			draw(already_drawn, x + 1, y + 1, start_x, start_y, sp, view, view_pos, light_pos, delta_time);
		}

	}
}
inline void world::insert(drawable* input, ShaderProgram& sp)
{
	if (input== NULL ) {
		return ;
	}
	if (input == nullptr) {
		return ;
	}
	
	//if(input->get_name() == "sph") {
	//	cout << "x: " << input->position().x << " y: " << input->position().y << " z: " << input->position().z << endl;
	//}
	
	//cout << "x: " << input->position().x << "  z: " << input->position().z << endl;
	//cout << input << endl ; 
	auto tmp =input->position();

	float input_x = tmp.x;
	float input_y = tmp.z;

	int in_x = num_normalizer(input_x);
	int in_y = num_normalizer(input_y);

	
	
	


	if (!Quad_init[in_x][in_y]) {
		init_chunk(in_x, in_y, sp);
	}

	Quad_tree[in_x][in_y].add_to_chunk(input);
	//cout << input->position() << endl;
}

inline bool world::remove_from(drawable* input) {
	if (input == NULL) {
		return false ;
	}
	if (input == nullptr) {
		return false ;
	}


	auto tmp = input->position();

	float input_x = tmp.x;
	float input_y = tmp.z;
	bool ret;
	int in_x = num_normalizer(input_x);
	int in_y = num_normalizer(input_y);
	if (Quad_tree[in_x][in_y].is_there(input)) {
		Quad_tree[in_x][in_y].get_from_chunk(input);
		ret = true;
	}
	else {
		//cout << "not there " << endl;
		ret = false;
	}
	return ret;
}

#endif