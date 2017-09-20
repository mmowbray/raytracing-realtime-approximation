#version 330 core

layout (location = 0) in vec3 backface_vertex_position;
layout (location = 1) in vec3 backface_vertex_normal;

out vec3 outBackfaceNorm;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main() {
	gl_Position = projection_matrix * view_matrix * model_matrix * vec4(backface_vertex_position, 1.0f);
	outBackfaceNorm = normalize(mat3(transpose(inverse(model_matrix))) * backface_vertex_normal);
}
