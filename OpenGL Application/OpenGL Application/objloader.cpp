#include <vector>
#include <stdio.h>
#include <cstring>

#include <glm.hpp>

#include "objloader.hpp"

#pragma warning(disable:4996)

bool loadOBJ(
	const char * path, std::vector<glm::vec3> & out_vertices, std::vector<glm::vec3> & out_normals, std::vector<GLuint> & out_indices)
{
	printf("Loading OBJ file: %s...\n", path);

	FILE * file = fopen(path, "r");

	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ?\n");
		getchar();
		return false;
	}

	for(;;) {

		char lineHeader[128];

		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			out_vertices.push_back(vertex);
		}
		else if(strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			out_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {

			GLuint vidx1, vidx2, vidx3; //vertex indices
			GLuint nidx1, nidx2, nidx3; //normal indices

			int matches = fscanf(file, "%i//%i %i//%i %i//%i", &vidx1, &nidx1, &vidx2, &nidx2, &vidx3, &nidx3);

			if (matches != 6) {
				return false;
			}

			out_indices.push_back(vidx1 - 1);
			out_indices.push_back(vidx2 - 1);
			out_indices.push_back(vidx3 - 1);
		}
		else {
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	return true;
}
