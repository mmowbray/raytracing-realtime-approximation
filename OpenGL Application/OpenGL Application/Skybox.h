#pragma once

#include <mat4x2.hpp>
#include "glm.hpp"
#include <glew.h>

class Skybox
{
	glm::mat4 model_matrix;
	glm::vec3 rotation, translation;
	GLuint VAO;
	int numIndices;
	GLint model_matrix_uniform_location;

public:
	Skybox(const char * model_path);
	void draw(int model_mat_uniform);
	void rotate(float rotY);
};
