#version 330 core

in vec3 outFrontfaceNorm;
in vec3 p1;

out vec4 frag_color;

//read normal from second framebuffer

uniform sampler2D backface_depth_texture;
uniform sampler2D backface_normals_texture;
uniform sampler2D skybox_texture;

uniform vec2 window_size;

void main() {

	/* Compute the incident light ray v. */
	
	vec3 eye_position = vec3(0,0,15); 									//add as a uniform
	vec3 v = p1 - eye_position;

	/* Compute T1, the refracted ray. */
	
	float n_air = 1.000277f;
	float n_glass = 1.44f;
	
	vec3 t1 = refract(v, outFrontfaceNorm, n_air/n_glass); 							//transform normals properly - check eta order of division

	/* Compute the approximations for P2. */
	
	float d = abs(texture(backface_depth_texture, gl_FragCoord.xy / window_size).z - gl_FragCoord.z);
	vec3 p2 = p1 + d * t1;
	
	/* Compute the approximations for N2. */
	
	vec2 n2_uv = vec2(p2.x + 1, 1 - p2.y) * (window_size / 2);
	vec3 n2 = texture(backface_normals_texture, n2_uv).xyz;
	
	/* Compute T2. */
	
	vec3 t2 = refract(t1, n2, n_glass/n_air);

	/* Project the doubly refracted ray T2 into the environment. */
	
	//frag_color = vec4(texture(skybox_texture, t2).rgb, 1.0f);
	
	frag_color = vec4(texture(backface_normals_texture, gl_FragCoord.xy / window_size).rgb, 1);
	//frag_color = vec4(0,1,0,1);
}
