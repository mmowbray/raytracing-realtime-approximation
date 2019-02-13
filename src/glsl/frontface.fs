#version 330 core

in vec3 outFrontfaceNorm;
in vec3 frag_position; //world coordinates

out vec4 frag_color;

//read normal from second framebuffer

uniform sampler2D backface_depth_texture;
uniform sampler2D backface_normals_texture;
uniform samplerCube skybox_texture;

uniform vec2 window_size;

uniform mat4 skybox_model_matrix_inv;
uniform mat4 skybox_model_matrix;

uniform int draw_mode;

uniform float manual_thickness;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

uniform vec3 eye_position;

float near = 0.1; 
float far  = 100.0;

#define n_air 1.0f

#define n_diamond vec3(2.407, 2.426, 2.451)

//returns depth in NDC (0..1)
float LinearizeDepth(float depth) 
{
	//source: https://learnopengl.com/#!Advanced-OpenGL/Depth-testing
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

float approximate_rt(vec3 channel){

	float channel_refract = dot(channel, n_diamond);

	/* Determine the camera -> fragment direction vector in world coordinates. */

	vec3 eye_direction = normalize(frag_position - eye_position);
	
	/* Estimate the refracted point on the back surface. */
	
	vec3 first_refraction = refract(eye_direction, outFrontfaceNorm, n_air/channel_refract);

	/* Determine an approximation for the thickness at this point. */
	// @TODO: Use precomputed normal depths for better thickness approximation

	float angleRatio = acos(dot(first_refraction, -outFrontfaceNorm)) /
						acos(dot(eye_direction, -outFrontfaceNorm));
	
	float eye_thickness = LinearizeDepth(texture(backface_depth_texture, gl_FragCoord.xy / window_size).x) - LinearizeDepth(gl_FragCoord.z);

	float estimated_thickness = angleRatio * eye_thickness * (1.0f - angleRatio) * manual_thickness;
	
	vec3 back_point = frag_position + estimated_thickness * first_refraction;
	
	/* Determine the direction of the second refraction. */
	
	vec4 back_point_ndc = projection_matrix * view_matrix * vec4(back_point, 1.0);
	vec2 back_point_px = (back_point_ndc.xy / back_point_ndc.w / 2.0) + 0.5;
	
	vec3 back_point_normal = normalize((texture(backface_normals_texture, back_point_px).xyz * 2.0f) - 1.0f); //change the range from [0, 1] to [-1, 1] for image decoding
	vec3 second_refraction = refract(first_refraction, -back_point_normal, channel_refract/n_air);
	
	if (all(equal(second_refraction, vec3(0.0))))
		second_refraction = reflect(first_refraction, -back_point_normal);


	/* Cast the ray and sample the environment. */
	
	return dot(texture(skybox_texture, vec3(skybox_model_matrix_inv * vec4(second_refraction, 1.0f))).rgb, channel);
}

vec4 wholething(){

	vec4 final = vec4(1.0f);

	final.r = approximate_rt (vec3(1.0, 0.0, 0.0));

	final.g=approximate_rt (vec3(0.0, 1.0, 0.0));
	final.b=approximate_rt (vec3(0.0, 0.0, 1.0));

	//return final;
	vec3 lightColor = vec3(0.5f);
	vec3 lightPos = vec3(skybox_model_matrix *  vec4(0.0f, 4.5f, 0.0f, 1.0));
	vec3 lightDir = normalize(lightPos - frag_position);

	float diff = max(dot(outFrontfaceNorm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
	final = vec4(diffuse + final.rgb, 1.0f);

//	return vec4(diffuse, 1.0f);

	return final + 0.3 * vec4(0,0,65, 0.0) / 255.0;
}

void main() {
	switch(draw_mode) {
		case  0:
			frag_color = wholething();
			break;
		case 1: //front normals
			frag_color = vec4((outFrontfaceNorm + 1.0f)/2.0f, 1.0f);
			break;
		case 2: //back normals

			frag_color = vec4((texture(backface_normals_texture, gl_FragCoord.xy / window_size).rgb ) * 2.0, 1.0f);
			break;
		case 3: //front depth
			float front_depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
			frag_color = vec4(vec3(front_depth), 1.0f);
			break;
		case 4: //back depth
			float back_depth = LinearizeDepth(texture(backface_depth_texture, gl_FragCoord.xy / window_size).x) / far; // divide by far for demonstration
			frag_color = vec4(vec3(back_depth), 1.0f);
			break;
		case 5: //thickness
			float thickness = (LinearizeDepth(texture(backface_depth_texture, gl_FragCoord.xy / window_size).x) - LinearizeDepth(gl_FragCoord.z)) / far;
			
			frag_color = vec4(vec3(thickness), 1.0f);
			break;
		default:
			frag_color = vec4(1, 0.5, 0.2, 1);
	}
}
