#version 330 core
  
layout (location = 0) in vec3 position;

out vec3 skyboxCoord;

uniform mat4 model_matrix; //Rotation
uniform mat4 projection_matrix;

void main()
{
    gl_Position = projection_matrix * model_matrix * vec4(position, 1.0f);
	skyboxCoord = position;
}