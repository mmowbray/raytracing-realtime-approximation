#version 330 core

in vec3 outBackfaceNorm;

layout(location = 0) out vec3 backface_normal; //output to COLOR_ATTACHMENT_0

void main() {
	backface_normal = outBackfaceNorm;
}
