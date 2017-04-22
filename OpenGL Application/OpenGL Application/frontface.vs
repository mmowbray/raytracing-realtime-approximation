#version 330 core

layout (location = 0) in vec3 frontface_vertex_position;
layout (location = 1) in vec3 frontface_vertex_normal;

out vec3 outFrontfaceNorm;
out vec3 p1; /* Fragment's position in world coordinates. */

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main() {
	p1 = vec3(model_matrix * vec4(frontface_vertex_position, 1));
	gl_Position = projection_matrix * view_matrix * vec4(p1, 1.0f);
	outFrontfaceNorm = mat3(transpose(inverse(model_matrix))) * frontface_vertex_normal;  
}
