#version 330 core

in vec3 outFrontfaceNorm;
in vec3 p1;

out vec4 frag_color;

//read normal from second framebuffer

uniform sampler2D backface_depth_texture;
uniform sampler2D backface_normals_texture;
uniform samplerCube skybox_texture;

uniform vec2 window_size;

uniform int draw_mode;

vec4 approximate_rt(){
	
	/* Compute the incident light ray v. */
	
	vec3 eye_position = vec3(0,0,15); 									//add as a uniform
	vec3 v = p1 - eye_position;

	/* Compute T1, the refracted ray. */
	
	float n_air = 1.000277f;
	float n_glass = 1.44f;
	
	vec3 t1 = refract(v, outFrontfaceNorm, n_glass/n_air); 							//transform normals properly - check eta order of division

	/* Compute the approximations for P2. */
	
	float d = abs(texture(backface_depth_texture, gl_FragCoord.xy / window_size).z - gl_FragCoord.z);
	d = 20;
	vec3 p2 = p1 + d * t1;
	
	/* Compute the approximations for N2. */
	
	vec2 n2_uv = vec2(p2.x + 1, 1 - p2.y) * (window_size / 2);
	vec3 n2 = texture(backface_normals_texture, n2_uv).xyz;
	
	/* Compute T2. */
	
	vec3 t2 = refract(t1, n2, n_air/n_glass);

	/* Project the doubly refracted ray T2 into the environment. */
	
	return vec4(texture(skybox_texture, t2).rgb, 1.0f);
}

void main() {
	switch(draw_mode) {
		case 0:
			frag_color = approximate_rt();
			break;
		case 1:
			frag_color = vec4(texture(backface_normals_texture, gl_FragCoord.xy / window_size).rgb, 1);
			break;
		case 2:
			frag_color = vec4((outFrontfaceNorm + 1.0f)/2.0f, 1); //front normals
			break;
		case 3:
			frag_color = vec4(texture(backface_depth_texture, gl_FragCoord.xy / window_size).rgb, 1); //back_normals
			break;
	}
}
