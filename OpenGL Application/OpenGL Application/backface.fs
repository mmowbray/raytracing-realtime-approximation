#version 330 core

in vec3 outBackfaceNorm;

layout(location = 0) out vec3 backface_normal; //output to COLOR_ATTACHMENT_0

void main() {
	
	
	backface_normal = outBackfaceNorm; //transform first?
	
	
	//backface_normal = vec3(0.3, 0.4, 0.5);
	
	//backface_normal = vec3(.2,.2, gl_FragCoord.z);
}
