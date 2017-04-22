#pragma once

#include <mat4x2.hpp>
#include "glm.hpp"
#include <glew.h>

class Model
{
	glm::mat4 model_matrix;
	glm::vec3 rotation, translation;
	GLuint VAO;
	int numVertices;
	GLint model_matrix_uniform_location;
	glm::vec2 old_mouse_position;

public:
	Model(const char * model_path);
	void rotate(int mousex, int mousey);
	void draw(int model_matrix_uniform);
};
