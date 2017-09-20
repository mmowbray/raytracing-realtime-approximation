#version 330 core

in vec3 outFrontfaceNorm;
in vec3 frag_position;

out vec4 frag_color;

//read normal from second framebuffer

uniform sampler2D backface_depth_texture;
uniform sampler2D backface_normals_texture;
uniform samplerCube skybox_texture;

uniform vec2 window_size;

uniform mat4 skybox_model_matrix;

uniform int draw_mode;
uniform bool draw_specular;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

float near = 0.1; 
float far  = 100.0;

#define n_air 1.0f
#define n_diamond 2.417f

//returns depth in NDC (0..1)
float LinearizeDepth(float depth) 
{
	//source: https://learnopengl.com/#!Advanced-OpenGL/Depth-testing
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

vec4 approximate_rt_new_try(){

	/* Determine the camera -> fragment direction vector in world coordinates. */

	vec3 eye_position = vec3(0, 0, 10); //TODO: do not hardcode
	vec3 eye_direction = normalize(frag_position - eye_position);
	
	/* Determine an approximation for the thickness at this point. */
	// @TODO: Use precomputed normal depths for better thickness approximation
	
	float estimated_thickness = LinearizeDepth(texture(backface_depth_texture, gl_FragCoord.xy / window_size).x) - LinearizeDepth(gl_FragCoord.z);
	
	/* Estimate the refracted point on the back surface. */
	
	vec3 first_refraction = refract(eye_direction, outFrontfaceNorm, n_air/n_diamond);
	
	if (draw_mode == 1) {
		/* Single refraction. */
		return vec4(texture(skybox_texture, vec3(inverse(skybox_model_matrix) * vec4(first_refraction, 1.0f))).rgb, 1.0f);
	}
	
	vec3 back_point = frag_position + estimated_thickness * first_refraction;
	
	/* Determine the direction of the second refraction. */
	
	vec4 back_point_ndc = projection_matrix * view_matrix * vec4(back_point, 1.0);
	vec2 back_point_px = (back_point_ndc.xy / back_point_ndc.w / 2.0) + 0.5;
	
	vec3 back_point_normal = normalize((texture(backface_normals_texture, back_point_px).xyz * 2.0f) - 1.0f); //change the range from [0, 1] to [-1, 1] for image decoding
	vec3 second_refraction = refract(first_refraction, -back_point_normal, n_diamond/n_air);
	
	if (all(equal(second_refraction, vec3(0.0))))
		second_refraction = reflect(first_refraction, -back_point_normal.xyz);
	
	/* Cast the ray and sample the environment. */
	
	return vec4(texture(skybox_texture, vec3(inverse(skybox_model_matrix) * vec4(second_refraction, 1.0f))).rgb, 1.0f);
}

void main() {
	switch(draw_mode) {
		case 0:
			frag_color = approximate_rt_new_try();
			break;
		case 1:
			frag_color = approximate_rt_new_try();
			break;
		case 2: //front normals
			frag_color = vec4((outFrontfaceNorm + 1.0f)/2.0f, 1.0f);
			break;
		case 3: //back normals

			frag_color = vec4((texture(backface_normals_texture, gl_FragCoord.xy / window_size).rgb ) * 2.0, 1.0f);
			break;
		case 4: //front depth
			float front_depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
			frag_color = vec4(vec3(front_depth), 1.0f);
			break;
		case 5: //back depth
			float back_depth = LinearizeDepth(texture(backface_depth_texture, gl_FragCoord.xy / window_size).x) / far; // divide by far for demonstration
			frag_color = vec4(vec3(back_depth), 1.0f);
			break;
		case 6: //thickness
			float thickness = (LinearizeDepth(texture(backface_depth_texture, gl_FragCoord.xy / window_size).x) - LinearizeDepth(gl_FragCoord.z)) / far;
			
			frag_color = vec4(vec3(thickness), 1.0f);
			break;
		default:
			frag_color = vec4(1, 0.5, 0.2, 1);
	}
}
