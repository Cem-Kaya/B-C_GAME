#ifndef SHADER_PROGRAM_HPP
#define SHADER_PROGRAM_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <unordered_map>
using namespace std;


void get_s_uv(const glm::vec3& p, float& u, float& v) {
	// p: a given point on the sphere of radius one, centered at the origin.
	// u: returned value [0,1] of angle around the Y axis from X=-1.
	// v: returned value [0,1] of angle from Y=-1 to Y=+1.
	 
	auto theta = acos(-p.y);
	auto phi = atan2(-p.z, p.x) +3.1415f;

	u = phi / (2.0f * 3.1415f);
	v = theta / 3.1415f;
	u = u * 0.98 + 0.01;
	v = v * 0.98 + 0.01; 
}



unordered_map < string, vector<glm::vec3> > vvec_table;
unordered_map < string, vector<unsigned > > ind_table;
unordered_map < string, vector<glm::vec3> > nvec_table;
unordered_map < string, vector<glm::vec2>> textcord_table;

VAO* get_mesh(string mesh_location , string texture_location) {
	if (vvec_table.find(mesh_location) == vvec_table.end() ) {
		// mesh import system
		ifstream input(mesh_location, std::ifstream::in);
		string line = "";
		vector<glm::vec3> vvec;
		vector<glm::vec3> nvec;
		vector<unsigned int> ind;
		while (getline(input, line)) {
			istringstream istr(line);
			string temp;
			istr >> temp;
			if (temp == "v") {
				string temp1;
				string temp2;
				string temp3;
				istr >> temp1;
				istr >> temp2;
				istr >> temp3;
				vvec.push_back(glm::vec3(stof(temp1), stof(temp2), stof(temp3)));
			}
			else if (temp == "vn") {
				string temp1;
				string temp2;
				string temp3;
				istr >> temp1;
				istr >> temp2;
				istr >> temp3;
				nvec.push_back(glm::vec3(stof(temp1), stof(temp2), stof(temp3)));
			}
			else if (temp == "f") {
				string temp1;
				string temp2;
				string temp3;
				istr >> temp1;
				istr >> temp2;
				istr >> temp3;
				int ta = stoi(temp1) - 1;
				int tb = stoi(temp2) - 1;
				int tc = stoi(temp3) - 1;

				ind.push_back(ta);
				ind.push_back(tb);
				ind.push_back(tc);
				//fvec.push_back(new Trig(vvec.at(ta), vvec.at(tb), vvec.at(tc), dino));

			}

		}
		glm::vec3 avg(0, 0, 0);

		for (auto& index : ind) {
			avg += vvec.at(index);
		}
		avg = avg / (float)ind.size();

		vector<glm::vec3> tempvvec;
		for (auto& V : vvec) {
			tempvvec.push_back(V - avg);
		}

		vector<glm::vec3> temptempvvec;
		for (auto& V : tempvvec) {
			temptempvvec.push_back(glm::normalize(V));
		}

		vector<glm::vec2> textcord;
		for (auto& VV : temptempvvec) {
			float u, v;
			get_s_uv(VV, u, v);
			u = u > 0 ? (u < 1 ? u : 1) : 0;
			v = v > 0 ? v < 1 ? v : 1 : 0;
			textcord.push_back(glm::vec2(u, v));

		}

		vvec_table[mesh_location] = vvec ;
		ind_table[mesh_location]  = ind  ;
		nvec_table[mesh_location] = nvec ;
		textcord_table[mesh_location] = textcord;

		auto r = new VAO(vvec, ind, vector<glm::vec3>(ind.size(), glm::vec3(1, 1, 1)), textcord, texture_location, nvec);
		return r;
	}else{
		auto vvec =vvec_table[mesh_location];
		auto ind = ind_table[mesh_location];
		auto nvec = nvec_table[mesh_location];
		auto textcord = textcord_table[mesh_location];

		auto r = new VAO(vvec, ind, vector<glm::vec3>(ind.size(), glm::vec3(1, 1, 1)), textcord, texture_location, nvec);
		return r;
	}
}



struct object_properties {
	//matrix
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;

	//ambient
	glm::vec3 ambient;
	float ambient_strength;

	//color and light
	glm::vec3 light_pos;
	glm::vec3 light_color;
	glm::vec3 view_pos;

	//specular
	float specular_strength;


	object_properties(glm::mat4 projection, glm::mat4 model, glm::mat4 view, glm::vec3 ambient, float ambient_strength, glm::vec3 light_pos, glm::vec3 light_color, glm::vec3 view_pos, float specular_strength);
	object_properties(){};
};

object_properties::object_properties(glm::mat4 projection, glm::mat4 model, glm::mat4 view, glm::vec3 ambient, float ambient_strength,
	glm::vec3 light_pos, glm::vec3 light_color, glm::vec3 view_pos,
	float specular_strength) {

	this->projection = projection;
	this->model = model;
	this->view = view;
	this->ambient = ambient;
	this->ambient_strength = ambient_strength;
	this->light_pos = light_pos;
	this->light_color = light_color;
	this->view_pos = view_pos;
	this->specular_strength = specular_strength;
};



class ShaderProgram
{
public:
	unsigned int id = NULL; 
	ShaderProgram(const string &vertexPath, const string &fragmentPath);
	void use();
	void set_shader(object_properties& obj) {

		this->setVec3("obj.ambient", obj.ambient);
		this->setFloat("obj.ambient_strength", obj.ambient_strength);
		this->setMat4("obj.projection", obj.projection);
		this->setVec3("obj.light_pos", obj.light_pos);
		this->setVec3("obj.view_pos", obj.view_pos);
		this->setVec3("obj.light_color", obj.light_color);
		this->setFloat("obj.specular_strength", obj.specular_strength);
		this->setMat4("obj.model", obj.model);
		this->setMat4("obj.view", obj.view);
	};


	string parseShader(const string& shaderDirectory);
	int createProgram(const string &vertexPath, const string &fragmentPath);
	unsigned int compileShader(unsigned int type, const char *source);
	unsigned int compileProgram(const char *vertexShader, const char *fragmentShad);

public:
	
	void setBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
	}
	void setInt(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(id, name.c_str()), value);
	}
	void setFloat(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(id, name.c_str()), value);
	}
	// new 
	void setVec2(const std::string &name, const glm::vec2 &value) const
	{ 
		glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); 
	}
	void setVec2(const std::string &name, float x, float y) const
	{ 
		glUniform2f(glGetUniformLocation(id, name.c_str()), x, y); 
	}
	// ------------------------------------------------------------------------
	void setVec3(const std::string &name, const glm::vec3 &value) const
	{ 
		glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); 
	}
	void setVec3(const std::string &name, float x, float y, float z) const
	{ 
		glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z); 
	}
	// ------------------------------------------------------------------------
	void setVec4(const std::string &name, const glm::vec4 &value) const
	{ 
		glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); 
	}
	void setVec4(const std::string &name, float x, float y, float z, float w) const
	{ 
		glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w); 
	}
	// ------------------------------------------------------------------------
	void setMat2(const std::string &name, const glm::mat2 &mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat3(const std::string &name, const glm::mat3 &mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat4(const std::string &name, const glm::mat4 &mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}


};


ShaderProgram::ShaderProgram(const string &vertexPath, const string &fragmentPath) {
	id = createProgram(vertexPath, fragmentPath);
}

void ShaderProgram::use() {
	glUseProgram(id);
}

int ShaderProgram::createProgram(const string &vertexPath, const string &fragmentPath) {
	
	string vertexSource = parseShader(vertexPath);
	string fragmentSource = parseShader(fragmentPath);

	const char* vertexSourcePtr = &vertexSource[0];
	const char* fragmentSourcePtr = &fragmentSource[0];

	unsigned int program = glCreateProgram();
	unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSourcePtr);
	unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSourcePtr);

	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	// Error check
	int success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if(!success) {
		cout << "Error: Program linking failed" << endl;
		
		int length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
		char *message = new char[length];

		glGetProgramInfoLog(program, length, &length, message);
		cout << message << endl;
		
		delete []message;
		return NULL;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return program;
}

unsigned int ShaderProgram::compileShader(unsigned int type, const char *source)
{
	unsigned int id = glCreateShader(type);
	glShaderSource(id, 1, &source, nullptr);
	glCompileShader(id);

	// Error check
	int success;
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if(!success) {
		cout << "Error: Shader Compilation failed" << endl;
		cout << "Shader type: " << (type == GL_FRAGMENT_SHADER ? "fragment shader" : "vertex shader") << endl;
		
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char *message = new char[length];

		glGetShaderInfoLog(id, length, &length, message);
		cout << message << endl;
		
		delete []message;
		glDeleteShader(id);

		return NULL;
	}

	return id;
}

string ShaderProgram::parseShader(const string& shaderPath) {

	ifstream shaderFile(shaderPath);
	string shaderSource = "";
	string line;

	if(shaderFile.fail()) {
		cout << "Failed to open file: \"" << shaderPath << "\"" << endl; 
		return "";
	};

	while(getline(shaderFile, line)) {
		shaderSource += line + '\n';
	}

	return shaderSource;
}

#endif