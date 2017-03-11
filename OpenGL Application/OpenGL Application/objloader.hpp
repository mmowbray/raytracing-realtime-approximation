#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include <glew.h>

bool loadOBJ(
	const char * path,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec3> & out_normals,
	std::vector<GLuint> & out_indices
);

#endif
