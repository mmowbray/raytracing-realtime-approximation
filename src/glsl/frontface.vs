#version 330 core

layout (location = 0) in vec3 frontface_vertex_position;
layout (location = 1) in vec3 frontface_vertex_normal;

out vec3 outFrontfaceNorm;
out vec3 frag_position; /* world coordinates. */

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main() {
	frag_position = vec3(model_matrix * vec4(frontface_vertex_position, 1));	
	gl_Position = projection_matrix * view_matrix * vec4(frag_position, 1.0f);	
	outFrontfaceNorm = normalize(mat3(transpose(inverse(model_matrix))) * frontface_vertex_normal);
}
