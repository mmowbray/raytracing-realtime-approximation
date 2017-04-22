#include <vector>
#include <stdio.h>
#include <cstring>

#include <glm.hpp>

#include "objloader.hpp"

#pragma warning(disable:4996)

bool loadOBJ(
	const char * path, std::vector<glm::vec3> & out_vertices, std::vector<glm::vec3> & out_normals, std::vector<glm::vec2> & out_uvs, std::vector<GLuint> & out_indices)
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
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			out_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "f") == 0) {

			GLuint vidx1, vidx2, vidx3; //vertex indices
			GLuint uvidx1, uvidx2, uvidx3; //normal indices
			GLuint nidx1, nidx2, nidx3; //normal indices

			int matches = fscanf(file, "%i/%i/%i %i/%i/%i %i/%i/%i", &vidx1, &uvidx1, &nidx1, &vidx2, &uvidx2, &nidx2, &vidx3,& uvidx3, &nidx3);

			if (matches != 9) {
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
