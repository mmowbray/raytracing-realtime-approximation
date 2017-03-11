#pragma once

#include <mat4x2.hpp>
#include "glm.hpp"
#include <glew.h>

class Model
{
	glm::mat4 model_matrix;
	GLuint VAO;
	int numIndices;

public:
	Model(const char * model_path);
	void draw(GLint modelMatrixLoc) const;
};
