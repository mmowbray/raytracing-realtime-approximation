#include "Model.h"
#include "objloader.hpp"

#include "gtc/type_ptr.hpp"
#include "gtc/matrix_transform.hpp"


Model::Model(const char * model_path)
{
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;

	loadOBJ(model_path, vertices, normals);

	GLuint vertices_VBO, normals_VBO, EBO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &vertices_VBO);
	glGenBuffers(1, &normals_VBO);
	glGenBuffers(1, &EBO);
	 
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, normals_VBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	numVertices = vertices.size();
}

void Model::rotate(int mousex, int mousey)
{
	if(mousex != -1)
	{
		model_matrix = glm::rotate(model_matrix, (float)(0.005 * (old_mouse_position.x - mousex)), glm::vec3(0.0, 1.0, 0.0));
		model_matrix = glm::rotate(model_matrix, (float)(0.005 * (old_mouse_position.y - mousey)), glm::vec3(1.0, 0.0, 0.0));
	}

	old_mouse_position.x = mousex;
	old_mouse_position.y = mousey;
}

void Model::draw(int model_matrix_uniform)
{
	glBindVertexArray(VAO);

	/* Send model matrix as a uniform. */

	glUniformMatrix4fv(model_matrix_uniform, 1, GL_FALSE, value_ptr(model_matrix));
	
	/* Draw. */
	glDrawArrays(GL_TRIANGLES, 0, numVertices);

	glBindVertexArray(0);
}
