#pragma once

#include <mat4x2.hpp>
#include "glm.hpp"
#include <glew.h>

class Skybox
{

	glm::vec3 rotation, translation;
	GLuint VAO;
	int numVertices;
	GLint model_matrix_uniform_location;

public:
	glm::mat4 model_matrix;
	Skybox(const char * model_path);
	void draw(int model_mat_uniform);
	void rotate(float rotY);
};
