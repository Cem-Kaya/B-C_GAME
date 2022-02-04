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
#include "world.hpp"
#include "perlin.hpp"
#include "helper.hpp"

#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>
#include <unordered_map>

using namespace std;




// Globals
unsigned width = 1600, height = 1050;
int u_transform;
const bool debug = false   ;

//   position                 ,  up,                       ,  yaw,   pitch  we initialized our camera
Camera cam(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);


inline float random_float_m( int seed) {
	// Returns a random real in [0,1).
	srand(seed);
	//auto tmp = rand();
	//cout << tmp << endl;
	return   rand() / (RAND_MAX + 1.0);
}

inline float random_float_m(float min, float max, int seed ) {
	// Returns a random real in [min,max).
	return min + (max - min) * random_float_m(seed);
}




bool wi = false;
bool ai = false;
bool si = false;
bool di = false;
bool teleport_up = false;
bool momentum_increase_y = false;
bool momentum_decrease_y = false;
bool terminal_momentum_increase = false;
bool fired = false;
bool esc = false;
bool kolay = true;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		wi = true;
	}
	else if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
		wi = false;
	}else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		ai = true;
	}
	else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
		ai = false;
	}
	else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		si = true;
	}
	else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
		si = false;
	}else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		di = true;
	}
	else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
		di = false;
	}
	else if( key == GLFW_KEY_Q && action == GLFW_PRESS){
		teleport_up = true;
	}
	else if( key == GLFW_KEY_Q && action == GLFW_RELEASE){
		teleport_up = false;
	}
	else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
		momentum_increase_y = true;
	}
	else if(key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS){
		momentum_decrease_y = true;
	}
	else if(key ==GLFW_KEY_LEFT_SHIFT   && action == GLFW_PRESS && !terminal_momentum_increase ){
		terminal_momentum_increase = true;
	}
	else if(key ==GLFW_KEY_LEFT_SHIFT   && action == GLFW_PRESS && terminal_momentum_increase ){
		terminal_momentum_increase = false;
	}
	else if(key ==GLFW_KEY_ESCAPE && action == GLFW_PRESS ){
		esc = true;
	}
	else if(key == GLFW_KEY_K && action == GLFW_RELEASE && kolay){
		kolay =false;
	}
	else if(key == GLFW_KEY_K && action == GLFW_RELEASE && !kolay){
		kolay =true;
	}
	
	
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        fired = true;
}


static void cursorPositionCallback(GLFWwindow* window, double x, double y)
{
	cam.Yaw = 480.0 * (	( (float)x / width))  ;       // yaw
	cam.Pitch = 90.0 *( -2*(((float)y/1000.0f)-0.5f)    ) ; // pitch  0 da +90 1000 olduðunda -90

	//cam.Pitch = -90.0*(   2*(((float)y / height)-1)     )  ; // pitch
	//cout << "y: " << y << endl;

	if(cam.Pitch > 80.0f)
		cam.Pitch = 80.0f;
	else if(cam.Pitch < -89.0f)
		cam.Pitch = -89.0f;

}

	




//statics
unsigned VAO::recent_texture_id = 0; //initializing the static int
unordered_map<string, unsigned > VAO::str_to_index;
unordered_map<string, stbi_uc*  > VAO::texture_instance;

int VAO::u_trans;

bool in_relative_mode = true;


int main()
{
	Window::init(width, height, "Baha & Cem GAME ");
	glfwSetKeyCallback(Window::window, keyCallback);
	glfwSetCursorPosCallback(Window::window, cursorPositionCallback);
	glfwSetMouseButtonCallback(Window::window, mouse_button_callback);

	glfwSetInputMode(Window::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // only draw vertices

	// create shader
	ShaderProgram sp("./Shader/vertex.vert", "./Shader/frag.frag");
	ShaderProgram light_sp("./Shader/light_vertex.vert", "./Shader/light_frag.frag");





	// import mesh 
	glm::vec3 ambient(0.1f, 0.9f, 1.0f);
	float ambient_strength = 0.5f;
	glm::mat4 projection; //projection matrix
	projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f);
	glm::vec3 light_pos = glm::vec3(0, 1000, 0);
	glm::mat4 model(1);
	auto view = cam.GetViewMatrix();
	auto specular_strength = 0.3;
	auto view_pos = cam.Position;
	glm::vec3 light_color(1.0f, 1.0f, 1.0f);

	world new_world;

	//chunk mychunk;
	//dino
	//auto dino = new GameObj3D(*get_mesh("./Mesh/test2.obj", "./Texture/T3.jpg") , sp );
	//dino->set_obj_data(projection, model, view, ambient, ambient_strength,
	//	light_pos, light_color, view_pos,
	//	specular_strength);
	//mychunk.add_to_chunk(dino);
	//sphere

	auto sphere = new GameObj3D(*get_mesh("./Mesh/test1.obj", "./Texture/T2.jpg"), sp);
	sphere->set_obj_data(projection, model, view, ambient, ambient_strength,
		light_pos, light_color, view_pos,
		specular_strength);
	//mychunk.add_to_chunk(sphere);
	//wall
	auto wall = new GameObj3D(*get_mesh("./Mesh/test0.obj", "./Texture/T3.jpg"), sp);
	wall->set_obj_data(projection, model, view, ambient, ambient_strength,
		light_pos, light_color, view_pos,
		specular_strength);
	wall->translate(0, 100, 0);
	//mychunk.add_to_chunk(wall);

	auto sun = new GameObj3D(*get_mesh("./Mesh/test1.obj", "./Texture/T2.jpg"), light_sp);
	sun->set_obj_data(projection, model, view, ambient, ambient_strength,
		light_pos, light_color, view_pos,
		specular_strength);
	sun->scale(50, 50, 50);
	sun->translate(0, 3000, 0);
	//mychunk.add_to_chunk(new GameObj3D(*get_mesh("./Mesh/test1.obj", "./Texture/T3.jpg"), sp));
	//mychunk.add_to_chunk(new GameObj3D(*get_mesh("./Mesh/test0.obj", "./Texture/T3.jpg"), sp));
	int kill_count = 0;
	bool HUD_0_b1 = true;
	bool HUD_0_b2 = false;
	bool HUD_1_b1 = true;
	bool HUD_1_b2 = false;
	bool HUD_2_b1 = true;
	bool HUD_2_b2 = false;
	bool HUD_3_b1 = true;
	bool HUD_3_b2 = false;

	auto HUD_0 = new GameObj3D(*get_mesh("./Mesh/dual_face_1_0.obj", "./Texture/1_0.jpg"), sp);
	HUD_0->set_obj_data(projection, model, view, ambient, ambient_strength,
		light_pos, light_color, view_pos,
		specular_strength);
	HUD_0->scale(5, 5, 5);
	HUD_0->translate(5 + 112.5, 0, 555);

	auto HUD_1 = new GameObj3D(*get_mesh("./Mesh/dual_face_1_0.obj", "./Texture/1_0.jpg"), sp);
	HUD_1->set_obj_data(projection, model, view, ambient, ambient_strength,
		light_pos, light_color, view_pos,
		specular_strength);
	HUD_1->scale(5, 5, 5);
	HUD_1->translate(5 + 75, 0, 555);

	auto HUD_2 = new GameObj3D(*get_mesh("./Mesh/dual_face_1_0.obj", "./Texture/1_0.jpg"), sp);
	HUD_2->set_obj_data(projection, model, view, ambient, ambient_strength,
		light_pos, light_color, view_pos,
		specular_strength);
	HUD_2->scale(5, 5, 5);
	HUD_2->translate(5 + 37.5, 0, 555);


	auto HUD_3 = new GameObj3D(*get_mesh("./Mesh/dual_face_1_0.obj", "./Texture/1_0.jpg"), sp);
	HUD_3->set_obj_data(projection, model, view, ambient, ambient_strength,
		light_pos, light_color, view_pos,
		specular_strength);
	HUD_3->scale(5, 5, 5);
	HUD_3->translate(5, 0, 555);




	int health = 3;
	auto heart0 = new GameObj3D(*get_mesh("./Mesh/heart.obj", "./Texture/heart.jpg"), sp);
	heart0->set_obj_data(projection, model, view, ambient, ambient_strength,
		light_pos, light_color, view_pos,
		specular_strength);
	heart0->scale(1, 1, 1);
	heart0->translate(650, 0, 545);

	auto heart1 = new GameObj3D(*get_mesh("./Mesh/heart.obj", "./Texture/heart.jpg"), sp);
	heart1->set_obj_data(projection, model, view, ambient, ambient_strength,
		light_pos, light_color, view_pos,
		specular_strength);
	heart1->scale(1, 1, 1);
	heart1->translate(700, 0, 545);

	auto heart2 = new GameObj3D(*get_mesh("./Mesh/heart.obj", "./Texture/heart.jpg"), sp);
	heart2->set_obj_data(projection, model, view, ambient, ambient_strength,
		light_pos, light_color, view_pos,
		specular_strength);
	heart2->scale(1, 1, 1);
	heart2->translate(750, 0, 545);



	// done import mesh 

	//Gl configuration

	//GAME PLAY init
	vector<GameObj3D* > enemy_list;
	vector<GameObj3D* > bullet_list;
	vector<GameObj3D* > party_list;
	// game loop
	long long unsigned int frameCount = 0;
	chrono::time_point<std::chrono::high_resolution_clock> now, old;
	//dino->translate(0, 200, 0);
	//dino->set_movable(false);
	//dino->set_force(glm::vec3(0.000005, -0.00001, 0));
	//new_world.insert(dino, sp);
	drawable* my_hero = wall;
	bool boss = false;
	bool boss_creation = false;
	bool game_win = false;
	long long game_win_count = -1;
	while (!Window::isClosed())
	{
		//cout << "sphere pos :" << sphere->position().y << " sphere speed :"<< sphere->get_current_momentum().y << " min point y :"<< sphere->get_min_y() << endl;
		//cout << "sphere pos :" << sphere->position().y << " sphere speed :"<< sphere->get_current_momentum().y << " min point y :"<< sphere->get_min_y() << endl;
		//HUD
		glEnable(GL_DEPTH_TEST);
		if (frameCount == 100) {
			//sphere
			// cout << " spawn " << endl;
			string tstr = "./Texture/T3.jpg";
			int randint = random_int(0, 4);
			if (randint == 0) {
				tstr = "./Texture/T1.jpg";
			}
			else if (randint == 1) {
				tstr = "./Texture/T2.jpg";
			}
			else if (randint == 2) {
				tstr = "./Texture/T3.jpg";
			}
			else if (randint == 3) {
				tstr = "./Texture/T4.jpg";
			}

			//auto sphere = new GameObj3D(*get_mesh("./Mesh/test1.obj",tstr), sp);
			auto sphere = new GameObj3D(*get_mesh("./Mesh/test1.obj", tstr), sp);
			//sphere->scale(0.1f, 0.1f, 0.1f);

			my_hero = sphere;

			sphere->set_obj_data(projection, model, view, ambient, ambient_strength,
				light_pos, light_color, view_pos,
				specular_strength);
			//mychunk.add_to_chunk(sphere);
			sphere->set_name("hero");
			// sphere->scale(1/10.f, 1 / 10.f, 1 / 10.f);
			sphere->translate(100, random_float(120, 250), 120);
			sphere->set_movable(false);
			sphere->set_force(glm::vec3(0, -5.0, 0));
			//sphere->set_force(glm::vec3(0 ,0 , 0 ));
			//sphere->set_force(glm::vec3(0.00005 * random_int(1,3), -0.01 , 0.00005 * random_int(1, 3)));
			new_world.insert(sphere, sp);
		}

		//  TIME 
		old = now;
		now = std::chrono::high_resolution_clock::now();

		//my_hero->rotate(0, 360*sin(frameCount / 10.0f), 0);

		std::chrono::duration<double> delta_primitive = now - old;
		double delta = delta_primitive.count();

		float step = delta * 500.0f;



		if (debug) {
			if (wi)
				cam.Position += step * cam.Front;
			if (ai)
				cam.Position -= step * glm::normalize(glm::cross(cam.Front, cam.Up));
			if (si)
				cam.Position -= step * cam.Front;
			if (di)
				cam.Position += step * glm::normalize(glm::cross(cam.Front, cam.Up));
		}
		else {
			float x__, z__;
			//x = cos(yaw)cos(pitch);
			//target.x= cos(cam.Yaw)*cos(cam.Pitch); // x and z may be switched 
			//y = sin(yaw)cos(pitch);
			//target.z = sin(cam.Yaw)* cos(cam.Pitch);
			//z = sin(pitch);
			//target.y = sin(cam.Pitch);
			// 

			// 
			// INPUT
			if (wi) {
				//cam.Position += step * cam.Front;				
				//cout << "yaw: " << cam.Yaw<<" x: " << x__ << endl;				
				my_hero->set_current_momentum(my_hero->get_current_momentum() + glm::vec3(cam.Front.x, 0, cam.Front.z));
				//my_hero->set_current_momentum(glm::vec3(1,0,0) );

			}
			if (ai) {
				//cam.Position -= step * glm::normalize(glm::cross(cam.Front, cam.Up));

				my_hero->set_current_momentum(my_hero->get_current_momentum() + glm::vec3(-1.0f * cam.Right.x, 0, -1.0f * cam.Right.z));
			}
			if (si) {
				//cam.Position -= step  * cam.Front;

				my_hero->set_current_momentum(my_hero->get_current_momentum() + glm::vec3(-1.0f * cam.Front.x, 0, -1.0f * cam.Front.z));
			}
			if (di) {

				my_hero->set_current_momentum(my_hero->get_current_momentum() + glm::vec3(cam.Right.x, 0, cam.Right.z));
				//cam.Position += step * glm::normalize(glm::cross(cam.Front, cam.Up));

			}

			if (teleport_up) {
				my_hero->translate(my_hero->position().x, 1300.0f, my_hero->position().z);
				my_hero->set_current_momentum(glm::vec3(0, 0, 0));
			}
			if (momentum_increase_y) {

				my_hero->set_current_momentum(my_hero->get_current_momentum() + glm::vec3(0, 1, 0));
				momentum_increase_y = false;

			}

			if (momentum_decrease_y) {
				my_hero->set_current_momentum(my_hero->get_current_momentum() + glm::vec3(0, -1, 0));
				momentum_decrease_y = false;
				//my_hero->physics_update(delta);
			}
			if (terminal_momentum_increase) {
				my_hero->set_terminal_momentum(glm::vec3(3, 9, 3));
			}
			if (!terminal_momentum_increase) {
				my_hero->set_terminal_momentum(glm::vec3(1, 3, 1));
			}

			cam.Position = glm::vec3(0, 17, 0) + my_hero->get_origin();
		}

		if (esc) {
			glfwSetInputMode(Window::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			esc = false;
			in_relative_mode = false;
		}

		// CAM 
		cam.updateCameraVectors();



		// Coordinate System Stuff::::::**********************************************



		//path world fall

		//float perl_ = my_noise.noise((x_x / 10000.0f + 0.0001f), 0.01f, (z_z / 10000.0f + 0.0001f));	
		float y_should_pos = new_world.my_noise.noise(glm::vec3((my_hero->position().x / 10000.0f + 0.0001f), 0.01f, (my_hero->position().z / 10000.0f + 0.0001f))) * 9000;
		if ((y_should_pos - 100) > my_hero->position().y) {
			my_hero->translate(my_hero->position().x, y_should_pos + 150, my_hero->position().z);
		}



		//*****************************************************************************
		// UPDATE
		//  
		//sun->translate(cam.Position.x + 100 * cos ( delta *frameCount / 50.0f ), cam.Position.y+ 100 * sin(delta*frameCount / 50.0f ) , cam.Position.z );

		/*
		for (auto o : mychunk.draw_list) {
			o->set_view(cam.GetViewMatrix());
		}
		*/
		// DRAW









		//properties
		glm::vec3 ambient(0.008f, 0.9f, 1.0f);
		float ambient_strength = 0.5f;
		glm::mat4 projection; //projection matrix
		projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f);
		glm::vec3 light_pos = glm::vec3(0, 0, 0);
		glm::mat4 model;
		auto view = cam.GetViewMatrix();
		auto specular_strength = 0.3;
		auto view_pos = cam.Position;
		glm::vec3 light_color(1.0f, 1.0f, 1.0f);


		//mychunk.draw_chunk();

		// light 


		light_sp.use();

		////properties
		ambient = glm::vec3(0.008f, 0.9f, 1.0f);
		ambient_strength = 0.5f;
		projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f);
		light_pos = sun->position();
		model = sun->getTransform();
		view = cam.GetViewMatrix();
		specular_strength = 0.3;
		view_pos = cam.Position;
		light_color = glm::vec3(1.0f, 1.0f, 1.0f);

		object_properties  obj2(projection, model, view, ambient, ambient_strength,
			light_pos, light_color, view_pos,
			specular_strength);

		light_sp.set_shader(obj2);
		//rec2.draw();

		//sun->translate(0, 50, 0);
		float day_time = sin(frameCount / 5000.0f);
		float day_color = (((day_time + 1.0f) / 2.0f) + 0.2f) / 1.2f;
		glClearColor(day_color * 0.2f, day_color * 0.5f, day_color * 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glClear(GL_DEPTH_BUFFER_BIT);





		sun->translate(cam.Position.x + 3000 * cos(frameCount / 5000.0f), cam.Position.y + 3000 * day_time, cam.Position.z);


		sun->set_view(view);
		sun->set_view_pos(cam.Position);
		sun->set_light_pos(sun->position());
		sun->set_perspective(glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 5000.0f));








		// render
		sun->draw();

		//GAME PLAY

		//create boss
		if (kill_count == 15 && !boss) {
			boss = true;
			//cout << "kill count checks out" << endl;
		}

		if (boss && !boss_creation) {
			boss_creation = true;
			//destroy all other coronas
			for (int i = 0; i < enemy_list.size(); i++) {
				if (new_world.remove_from(enemy_list[i])) {
					delete  enemy_list.at(i);
				}
			}
			enemy_list.clear();


			GameObj3D* bad = new GameObj3D(*get_mesh("./Mesh/corona.obj", "./Texture/corona_king.jpg"), sp);

			bad->set_obj_data(projection, model, view, ambient, ambient_strength, light_pos, light_color, view_pos, specular_strength);
			//mychunk.add_to_chunk(sphere);
			bad->set_name("bad");

			float x_segment = random_float_m(0, 10,frameCount);
			float z_segment = random_float_m(0, 10, frameCount);

			float rand_x = x_segment >5 ? cam.Position.x - random_float_m(400, 800, frameCount) : cam.Position.x + random_float_m(400, 800,frameCount);
			float rand_z = z_segment > 5 ? cam.Position.z - random_float_m(400, 800, frameCount) : cam.Position.z + random_float_m(400, 800, frameCount);
			
			float rand_y = new_world.my_noise.noise(glm::vec3((rand_x / 10000.0f + 0.0001f), 0.01f, (rand_z / 10000.0f + 0.0001f))) * 9000;

			bad->translate(rand_x, rand_y+1000, rand_z);
			bad->set_movable(false);

			//boss mechanics
			bad->health_set(15);
			bad->scale(5, 5 , 5);
			bad->set_force(glm::vec3(0, -5.0, 0));
			bad->set_terminal_momentum(glm::vec3(2, 6, 2));
			//sphere->set_force(glm::vec3(0 ,0 , 0 ));
			//sphere->set_force(glm::vec3(0.00005 * random_int(1,3), -0.01 , 0.00005 * random_int(1, 3)));
			new_world.insert(bad, sp);
			enemy_list.push_back(bad);
			//cout << "boss is created " << endl;
		}

		if (boss && boss_creation) {
			//cout << "boss mechanics on" << endl;
			for (int i = 0; i < enemy_list.size(); i++) {
				if (enemy_list.at(i)->get_is_hit()) {
					enemy_list.at(i)->health_down();
					enemy_list.at(i)->not_hit();
					auto bh = enemy_list.at(i)->get_health() / 3.0f ; 
					enemy_list.at(i)->scale(bh, bh, bh);
				}


				if (enemy_list[i]->get_health() <= 0) {
					//kill_count++;
					if (new_world.remove_from(enemy_list.at(i))) {
						//cout << "test point 1" << endl;
						glm::vec3 party_pos = enemy_list.at(i)->position();

						for (int i = -4; i <= 4; i++) {
							for (int j = -4; j <= 4; j++) {
								for (int k = -4; k <= 4; k++) {
									GameObj3D* party = new GameObj3D(*get_mesh("./Mesh/Ground1.obj", "./Texture/corona_king.jpg"), sp);

									party->set_obj_data(projection, model, view, ambient, ambient_strength, light_pos, light_color, view_pos, specular_strength);

									party->set_name("party");


									party->translate(party_pos.x + i, party_pos.y + j, party_pos.z + k);
									party->set_movable(false);
									party->scale(19 / 10.0f, 10 / 10.0f, 10 / 10.0f);
									party->set_force(glm::vec3(0, -3.5f, 0));
									party->set_terminal_momentum(glm::vec3(3, 5, 3));
									//sphere->set_force(glm::vec3(0 ,0 , 0 ));
									//sphere->set_force(glm::vec3(0.00005 * random_int(1,3), -0.01 , 0.00005 * random_int(1, 3)));
									new_world.insert(party, sp);
									party_list.push_back(party);
								}
							}
						}
					}
					game_win = true;

				}


			}
		}


		if (frameCount % 200 == 50 && !boss) {
			GameObj3D* bad = new GameObj3D(*get_mesh("./Mesh/corona.obj", "./Texture/heart.jpg"), sp);

			bad->set_obj_data(projection, model, view, ambient, ambient_strength, light_pos, light_color, view_pos, specular_strength);
			//mychunk.add_to_chunk(sphere);
			bad->set_name("bad");
			// sphere->scale(1/10.f, 1 / 10.f, 1 / 10.f);

			
			float x_segment = random_float_m(0, 10, frameCount);
			float z_segment = random_float_m(0, 10, frameCount);

			float rand_x = x_segment>5 ? cam.Position.x + random_float_m(300, 600, frameCount) : cam.Position.x - random_float_m(300, 600 ,frameCount);
			float rand_z = z_segment > 5 ? cam.Position.z + random_float_m(300, 800, frameCount) : cam.Position.z - random_float_m(300, 800, frameCount);
			
			float rand_y = new_world.my_noise.noise(glm::vec3((rand_x / 10000.0f + 0.0001f), 0.01f, (rand_z / 10000.0f + 0.0001f))) * 9000;

			bad->translate(rand_x, rand_y+100, rand_z);
			bad->set_movable(false);
			bad->scale(7 / 10.0f, 7 / 10.0f, 7 / 10.0f);
			bad->set_force(glm::vec3(0, -5.0, 0));
			bad->set_terminal_momentum(glm::vec3(2, 6, 2));
			//sphere->set_force(glm::vec3(0 ,0 , 0 ));
			//sphere->set_force(glm::vec3(0.00005 * random_int(1,3), -0.01 , 0.00005 * random_int(1, 3)));
			new_world.insert(bad, sp);
			enemy_list.push_back(bad);
		}
		if (my_hero->get_is_hit()) {
			health--;
			if(!boss){ //ekledim burayý
				for (int i = 0; i < enemy_list.size(); i++) {
					if (new_world.remove_from(enemy_list[i])) {
						delete  enemy_list.at(i);
					}
				}
				enemy_list.clear();
			}
			my_hero->not_hit();
		}


		if (fired) {
			if (!in_relative_mode) {
				in_relative_mode = true;
				glfwSetInputMode(Window::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			//cout << "bang bang" << endl;

			glm::vec3 look = glm::normalize(cam.Front);
			GameObj3D* bullet = new GameObj3D(*get_mesh("./Mesh/VAX.obj", "./Texture/T3.jpg"), sp);

			bullet->set_obj_data(projection, model, view, ambient, ambient_strength, light_pos, light_color, view_pos, specular_strength);
			//mychunk.add_to_chunk(sphere);
			bullet->set_name("bullet");
			// sphere->scale(1/10.f, 1 / 10.f, 1 / 10.f);

			bullet->translate(cam.Position.x + look.x * 10,
				cam.Position.y + look.y * 10,
				cam.Position.z + look.z * 10);
			bullet->set_movable(false);
			bullet->scale(2 / 10.0f, 2 / 10.0f, 2 / 10.0f);
			bullet->set_force(glm::vec3(0, -1.0, 0));
			bullet->set_current_momentum(look * 15.0f);
			bullet->set_terminal_momentum(glm::vec3(17.5, 15, 17.5));
			//sphere->set_force(glm::vec3(0 ,0 , 0 ));
			//sphere->set_force(glm::vec3(0.00005 * random_int(1,3), -0.01 , 0.00005 * random_int(1, 3)));
			new_world.insert(bullet, sp);
			bullet_list.push_back(bullet);

			fired = false;
		}


		for (int i = 0; i < bullet_list.size(); i++) {
			if (bullet_list.at(i)->get_is_hit()) {
				if (new_world.remove_from(bullet_list.at(i))) {
					delete bullet_list.at(i);
					auto tmp = bullet_list[bullet_list.size() - 1];
					//auto tmp2 = bullet_list[i];
					bullet_list[bullet_list.size() - 1] = bullet_list[i];
					bullet_list[i] = tmp;
					bullet_list.pop_back();
				}
			}
		}


		for (int i = 0; i < enemy_list.size(); i++) {
			if (!boss) {
				enemy_list.at(i)->rotate(0, 360 * sin(frameCount / 25.0f), 0);
				//enemy_list.at(i)->scale(1, 1 - sin(frameCount / 20.0f) / 4.0f, 1);
			}
			else {
				enemy_list.at(i)->rotate(0, 360 * sin(frameCount / 65.0f), 0);
			}
			glm::vec3 gety =enemy_list.at(i)->get_current_momentum();
			glm::vec3 move;
			if (kolay) {
				move = glm::normalize(my_hero->position() - enemy_list.at(i)->position());
			}
			else {
				move = my_hero->position() - enemy_list.at(i)->position();
			}
			//enemy_list.at(i) -> set_current_momentum(glm::vec3(glm::length(enemy_list.at(i)->get_current_momentum()) * move.x, gety.y , glm::length(enemy_list.at(i)->get_current_momentum()) * move.z));
			
			move.y = gety.y;
			enemy_list.at(i)->set_current_momentum(move);
			

			// DEBUG 
			float y_should_pos = new_world.my_noise.noise(glm::vec3((enemy_list.at(i)->position().x / 10000.0f + 0.0001f), 0.01f, (enemy_list.at(i)->position().z / 10000.0f + 0.0001f))) * 9000;
			if ((y_should_pos - 100) > enemy_list.at(i)->position().y) {
				if(boss){
				enemy_list.at(i)->translate(enemy_list.at(i)->position().x, y_should_pos + 1000, enemy_list.at(i)->position().z);
				}
				else{
				enemy_list.at(i)->translate(enemy_list.at(i)->position().x, y_should_pos + 150, enemy_list.at(i)->position().z);

				}
			}


			if (enemy_list[i]->get_is_hit()) {
				enemy_list[i]->health_down();
				if (enemy_list[i]->get_health() <= 0) {
					kill_count++;
					if (new_world.remove_from(enemy_list.at(i))) {
						//cout << "test point 1" << endl;
						glm::vec3 party_pos = enemy_list.at(i)->position();

						for (int i = -2; i <= 2; i++) {
							for (int j = -2; j <= 2; j++) {
								for (int k = -2; k <= 2; k++) {
									GameObj3D* party = new GameObj3D(*get_mesh("./Mesh/Ground1.obj", "./Texture/heart.jpg"), sp);

									party->set_obj_data(projection, model, view, ambient, ambient_strength, light_pos, light_color, view_pos, specular_strength);

									party->set_name("party");


									party->translate(party_pos.x + i, party_pos.y + j, party_pos.z + k);
									party->set_movable(false);
									party->scale(19 / 10.0f, 10 / 10.0f, 10 / 10.0f);
									party->set_force(glm::vec3(0, -3.5f, 0));
									party->set_terminal_momentum(glm::vec3(3, 5, 3));
									//sphere->set_force(glm::vec3(0 ,0 , 0 ));
									//sphere->set_force(glm::vec3(0.00005 * random_int(1,3), -0.01 , 0.00005 * random_int(1, 3)));
									new_world.insert(party, sp);
									party_list.push_back(party);
								}
							}
						}
						//cout << "test point 2" << endl;


						auto delete_ptr = enemy_list.at(i);

						auto tmp = enemy_list.back();
						enemy_list[enemy_list.size() - 1] = enemy_list[i];
						enemy_list[i] = tmp;
						enemy_list.pop_back();

						delete delete_ptr;
						//cout << "test point 3" << endl;

					}
				}
				//enemy_list.at(i)->not_hit();
			}
		}





		for (int i = 0; i < party_list.size(); i++) {
			if (party_list.at(i)->get_is_hit() || party_list.at(i)->position().y < -1000 || party_list.at(i)->position().y > 2500) {
				if (new_world.remove_from(party_list.at(i))) {
					delete party_list.at(i);
					auto tmp = party_list[party_list.size() - 1];
					//auto tmp2 = bullet_list[i];
					party_list[party_list.size() - 1] = party_list[i];
					party_list[i] = tmp;
					party_list.pop_back();
				}
			}
		}

		if (game_win && game_win_count == -1) {
			game_win_count = frameCount + 70 * 25;
		}
		if (game_win_count == frameCount) {
			cout << "you win !!!" << endl;
			glfwTerminate();
			break;
		}


		new_world.draw(cam.Position, sp, cam.GetViewMatrix(), cam.Position, sun->position(), delta);



		// HUD
		sp.use();
		glDisable(GL_DEPTH_TEST);
		glm::mat4 orto = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, 0.1f, 5000.0f); //orthogonal projection matrix
		auto ortow = glm::lookAt(glm::vec3(0), glm::vec3(0) + glm::vec3(0, 1, 0), glm::vec3(0) + glm::vec3(0, 0, 1));


		HUD_0->set_view(ortow);
		HUD_0->set_view_pos(glm::vec3(0, 0, -1));
		HUD_0->set_light_pos(glm::vec3(-200, -200, -200));
		HUD_0->set_perspective(orto);

		HUD_1->set_view(ortow);
		HUD_1->set_view_pos(glm::vec3(0, 0, -1));
		HUD_1->set_light_pos(glm::vec3(-200, -200, -200));
		HUD_1->set_perspective(orto);

		HUD_2->set_view(ortow);
		HUD_2->set_view_pos(glm::vec3(0, 0, -1));
		HUD_2->set_light_pos(glm::vec3(-200, -200, -200));
		HUD_2->set_perspective(orto);

		HUD_3->set_view(ortow);
		HUD_3->set_view_pos(glm::vec3(0, 0, -1));
		HUD_3->set_light_pos(glm::vec3(-200, -200, -200));
		HUD_3->set_perspective(orto);




		if ((kill_count % 2) == 0) {
			if (HUD_0_b1) {
				HUD_0->rotate(0, 0, 90);
				HUD_0->translate(HUD_0->position().x + 37.5, HUD_0->position().y, HUD_0->position().z);
				HUD_0_b1 = false;
				HUD_0_b2 = true;
			}

		}
		else {
			if (HUD_0_b2) {

				HUD_0->rotate(0, 0, 0);
				HUD_0->translate(HUD_0->position().x - 37.5, HUD_0->position().y, HUD_0->position().z);
				HUD_0_b2 = false;
				HUD_0_b1 = true;
			}
		}

		if ((kill_count % 4) / 2 == 0) {
			if (HUD_1_b1) {
				HUD_1->rotate(0, 0, 90);
				HUD_1->translate(HUD_1->position().x + 37.5, HUD_1->position().y, HUD_1->position().z);

				HUD_1_b1 = false;
				HUD_1_b2 = true;
			}
		}
		else {
			if (HUD_1_b2) {
				HUD_1->rotate(0, 0, 0);
				HUD_1->translate(HUD_1->position().x - 37.5, HUD_1->position().y, HUD_1->position().z);
				HUD_1_b2 = false;
				HUD_1_b1 = true;
			}
		}

		if ((kill_count / 4) % 2 == 0) {
			if (HUD_2_b1) {
				HUD_2->rotate(0, 0, 90);
				HUD_2->translate(HUD_2->position().x + 37.5, HUD_2->position().y, HUD_2->position().z);

				HUD_2_b1 = false;
				HUD_2_b2 = true;
			}
		}
		else {
			if (HUD_2_b2) {
				HUD_2->rotate(0, 0, 0);
				HUD_2->translate(HUD_2->position().x - 37.5, HUD_2->position().y, HUD_2->position().z);

				HUD_2_b2 = false;
				HUD_2_b1 = true;
			}
		}
		if ((kill_count / 8) % 2 == 0) {
			if (HUD_3_b1) {
				HUD_3->rotate(0, 0, 90);
				HUD_3->translate(HUD_3->position().x + 37.5, HUD_3->position().y, HUD_3->position().z);

				HUD_3_b1 = false;
				HUD_3_b2 = true;
			}
		}
		else {
			if (HUD_3_b2) {
				HUD_3->rotate(0, 0, 0);
				HUD_3->translate(HUD_3->position().x - 37.5, HUD_3->position().y, HUD_3->position().z);
				HUD_3_b2 = false;
				HUD_3_b1 = true;
			}
		}
		HUD_0->draw();
		HUD_1->draw();
		HUD_2->draw();
		HUD_3->draw();


		heart0->set_view(ortow);
		heart0->set_view_pos(glm::vec3(0, 0, -1));
		heart0->set_light_pos(glm::vec3(900, 500, 500));
		heart0->set_perspective(orto);



		heart1->set_view(ortow);
		heart1->set_view_pos(glm::vec3(0, 0, -1));
		heart1->set_light_pos(glm::vec3(900, 500, 500));
		heart1->set_perspective(orto);


		heart2->set_view(ortow);
		heart2->set_view_pos(glm::vec3(0, 0, -1));
		heart2->set_light_pos(glm::vec3(900, 500, 500));
		heart2->set_perspective(orto);

		heart0->scale(1 + sin(frameCount / 10.0f) / 10.f, 1 + sin(frameCount / 10.0f) / 10.f, 1 + sin(frameCount / 10.0f) / 10.f);
		heart1->scale(1 + sin(frameCount / 10.0f + 120) / 10.f, 1 + sin(frameCount / 10.0f + 120) / 10.f, 1 + sin(frameCount / 10.0f + 120) / 10.f);
		heart2->scale(1 + sin(frameCount / 10.0f + 240) / 10.f, 1 + sin(frameCount / 10.0f + 240) / 10.f, 1 + sin(frameCount / 10.0f + 240) / 10.f);



		if (health >= 3) {
			heart0->draw();
		}
		if (health >= 2) {
			heart1->draw();
		}
		if (health >= 1) {
			heart2->draw();
		}
		else {
			cout << "You are DEAD !!!!!  " << endl;
			exit(0);
			glfwTerminate();
		}




		Window::swapBuffersAndPollEvents();
		frameCount++;


	}

	glfwTerminate();

	return 0;
}