#ifndef VAO_HPP
#define VAO_HPP

#include <string>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <unordered_map>
#include "stb_image.h"
using namespace std;

class VAO
{
public:
	vector<glm::vec3> vao_positions;
	
	VAO(const vector<glm::vec3>& positions, const vector<unsigned int>& indices ,  const vector<glm::vec3>& color, const vector<glm::vec2>& texture, string texture_path, const vector<glm::vec3>& normals );
	~VAO();
	void bind();
	unsigned int getIndicesCount();
	void draw(   glm::mat4 transform );
	static unordered_map<string, unsigned > str_to_index;
	static unordered_map<string, stbi_uc* > texture_instance ;

	static int  u_trans;
	glm::vec3 max_point;
	glm::vec3 min_point;
	string texture_path_intput;
	unsigned int vao = -1;
	unsigned int indicesCount = -1;
	stbi_uc* Texture;
	int t_width;
	int t_height;
	int t_nrChannels;
	unsigned texture_id;
	static unsigned recent_texture_id ;
	void createVAO(const vector<glm::vec3> &positions, const vector<unsigned int> &indices);
};

void VAO::draw(  glm::mat4 transform = glm::mat4(1.0f)){
	
	glBindTexture(GL_TEXTURE_2D, texture_id);
	this->bind();
	glDrawElements(GL_TRIANGLES, getIndicesCount(), GL_UNSIGNED_INT, NULL);


}


void VAO::bind()
{
	glBindVertexArray(this->vao);
}

unsigned int VAO::getIndicesCount()
{
	return this->indicesCount;
}

void VAO::createVAO(const vector<glm::vec3> &positions, const vector<unsigned int> &indices)
{
	glGenVertexArrays(1, &this->vao);
	glBindVertexArray(this->vao);

	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * positions.size(), positions.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);
}



VAO::VAO(const vector<glm::vec3>& positions, const vector<unsigned int>& indices, const vector<glm::vec3>& color, const vector<glm::vec2>& texture, string texture_path, const vector<glm::vec3>& normals) {
	glGenVertexArrays(1, &this->vao);
	glBindVertexArray(this->vao);
	vao_positions = positions;
	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	vector<float> arr;
	for (int i = 0; i < positions.size(); i++) {
		// push back pos
		arr.push_back(positions.at(i).x);
		arr.push_back(positions.at(i).y);
		arr.push_back(positions.at(i).z);
		// push back color 
		arr.push_back(color.at(i).x);
		arr.push_back(color.at(i).y);
		arr.push_back(color.at(i).z);
		// push back texture
		arr.push_back(texture.at(i).x);
		arr.push_back(texture.at(i).y);
		// push back normals
		arr.push_back(normals.at(i).x);
		arr.push_back(normals.at(i).y);
		arr.push_back(normals.at(i).z);
	}

	glBufferData(GL_ARRAY_BUFFER,( sizeof(glm::vec3)+ sizeof(glm::vec3) + sizeof(glm::vec2)+ sizeof(glm::vec3)) * positions.size(), arr.data(), GL_STATIC_DRAW);
	//pos
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11, 0); 
	//color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11, (void*) (3 * sizeof(float)));
	//texture
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 11,(void*) (6*sizeof(float))) ;
	//normals 
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11,(void*) (8*sizeof(float))) ;

	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

// load image, create texture and generate mipmaps

	if (str_to_index.find(texture_path) == str_to_index.end()) {
		// yoksa texture 
		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id);
		
		
		//glGenTextures(1, &recent_texture_id);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		(this->Texture) = stbi_load(texture_path.c_str(), &t_width, &t_height, &t_nrChannels, 0);
		if (!this->Texture)
		{
			std::cout << "Failed to load" << texture_path << " " << std::endl;
			(this->Texture) = stbi_load("./Texture/T4.jpg", &t_width, &t_height, &t_nrChannels, 0);
			std::cout << "Defult load loading T4 " << std::endl;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t_width, t_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);
		//cout << *Texture << endl;
		//Texture = texture_instance[texture_path_intput];
		glGenerateMipmap(GL_TEXTURE_2D);
		
		//glBindTexture(GL_TEXTURE_2D, texture_id);

		str_to_index[texture_path] = texture_id;
	}
	else {
		//varsa 
		texture_id = str_to_index[texture_path];

	}


	/*
	glGenTextures(1, &texture_id);
	//glGenTextures(1, &recent_texture_id);

	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);




	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t_width, t_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);
	//cout << *Texture << endl;
	//Texture = texture_instance[texture_path_intput];
	glGenerateMipmap(GL_TEXTURE_2D);







	// TODO texture instancing  
	
	texture_path_intput = texture_path;
	if (!( texture_instance.find(texture_path) == texture_instance.end()) ) {
		
		Texture = texture_instance[texture_path];

		// cout << *this->Texture << endl;
	}
	else {

		


		stbi_uc* texture_data = stbi_load(texture_path.c_str(), &t_width, &t_height, &t_nrChannels, 0);
		//(this->Texture) = stbi_load(texture_path.c_str(), &t_width, &t_height, &t_nrChannels, 0);
		(this->Texture) = texture_data;
		//texture_instance[texture_path] = this->Texture; 
		texture_instance[texture_path] = texture_data; 
		//texture_instance.insert({ texture_path, this->Texture });
		//cout << "init texture chunk :" << *this->Texture << endl;

	}
	
		//stbi_uc* texture_data = stbi_load(texture_path.c_str(), &t_width, &t_height, &t_nrChannels, 0);
		
		
		////(this->Texture) = stbi_load(texture_path.c_str(), &t_width, &t_height, &t_nrChannels, 0);
		//(this->Texture) = texture_data;
		////texture_instance[texture_path] = this->Texture; 
		//texture_instance[texture_path] = texture_data; 
		////texture_instance.insert({ texture_path, this->Texture });
		////cout << "init texture chunk :" << *this->Texture << endl;

	if (!this->Texture)
	{
		std::cout << "Failed to load" << texture_path << " " << std::endl;

		(this->Texture) = stbi_load("./Texture/T4.jpg", &t_width, &t_height, &t_nrChannels, 0);
		std::cout << "Defult load loading T4 "<< std::endl;

	}
	*/
	
	this->indicesCount = indices.size();


	float max_x = 0, max_y = 0, max_z = 0, min_x = 0, min_y = 0, min_z = 0;
	max_x = std::numeric_limits<float>::min();
	max_y = std::numeric_limits<float>::min();
	max_z = std::numeric_limits<float>::min();
	min_x = std::numeric_limits<float>::max();
	min_y = std::numeric_limits<float>::max();
	min_z = std::numeric_limits<float>::max();

	//max
	for (auto& V : positions ) {
		if (max_x == std::numeric_limits<float>::min()) {
			max_x = V.x;
		}
		else {
			if (max_x < V.x) {
				max_x = V.x;
			}
		}
	}
	for (auto& V : positions) {
		if (max_y == std::numeric_limits<float>::min()) {
			max_y = V.y;
		}
		else {
			if (max_y < V.y) {
				max_y = V.y;
			}
		}
	}
	for (auto& V : positions) {
		if (max_z == std::numeric_limits<float>::min()) {
			max_z = V.z;
		}
		else {
			if (max_z < V.z) {
				max_z = V.z;
			}
		}
	}
	//mins
	for (auto& V : positions) {
		if (min_x > V.x) {
			min_x = V.x;
		}
	}
	for (auto& V : positions) {
		if (min_y > V.y) {
			min_y = V.y;
		}
	}
	for (auto& V : positions) {
		if (min_z > V.z) {
			min_z = V.z;
		}

	}

	min_x = min_x * -1;
	min_y = min_y * -1;
	min_z = min_z * -1;
	/*
	vector<glm::vec3> temptempvvec;
	for (auto& V : positions) {
		temptempvvec.push_back(glm::vec3(V.x > 0 ? V.x / max_x : V.x / min_x,
			V.y > 0 ? V.y / max_y : V.y / min_y,
			V.z > 0 ? V.z / max_z : V.z / min_z));
	}

	*/
	max_point = glm::vec3(max_x, max_y,max_z);
	min_point = glm::vec3(min_x,min_y,min_z);

	//stbi_image_free(Texture);
}



 VAO::~VAO()
{
	stbi_image_free(this->Texture);
}


#endif