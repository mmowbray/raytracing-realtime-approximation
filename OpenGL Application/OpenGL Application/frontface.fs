#version 330 core

in vec3 outFrontfaceNorm;
in vec3 p1;

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
float far  = 20.0;

vec3 eye_position = vec3(0,0,50);

float LinearizeDepth(float depth) 
{
	//source: https://learnopengl.com/#!Advanced-OpenGL/Depth-testing
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));
}

vec4 approximate_rt(){
	
	float n_air = 1.0f;
	float n_glass = 1.2f;

	/* First refraction. */
	
	vec3 nEyeDir = normalize(p1 - eye_position);
	vec3 nNormal = normalize(outFrontfaceNorm);
	vec3 rayDir = refract(nEyeDir, nNormal, n_air/n_glass);
	
	if (draw_mode == 1) {
		/* Single refraction. */
		rayDir = vec3(inverse(skybox_model_matrix) * vec4(rayDir, 1.0f)); /* Take the rotation of the skybox into account. */		
		return vec4(texture(skybox_texture, rayDir).rgb, 1.0f);
	}
	
	/* Second refraction. */
	
	float eyeThick   = (LinearizeDepth(texture(backface_depth_texture, gl_FragCoord.xy / window_size).x) - LinearizeDepth(gl_FragCoord.z)) / far;
	float angleRatio = acos(dot(rayDir,  -outFrontfaceNorm)) / acos(dot(nEyeDir, -outFrontfaceNorm));
	float estThick   = angleRatio * eyeThick + (1.0-angleRatio);
	vec3  estExitPt  = eye_position + rayDir * estThick; 
	vec4  estExitPos = projection_matrix * model_matrix * view_matrix * vec4(estExitPt, 1.0);
	vec2  estExitPx  = (estExitPos.xy / estExitPos.w / 2.0) + 0.5;
	vec3  exitNormal = normalize(texture2D(backface_normals_texture, estExitPx).xyz * 2.0 - 1.0);
	vec3  rayDir2    = refract(rayDir, -exitNormal, n_glass/n_air);
	
	if (all(equal(rayDir2, vec3(0.0))))
		rayDir2 = reflect(rayDir, -exitNormal);
	
	rayDir2 = vec3(inverse(skybox_model_matrix) * vec4(rayDir2, 1.0f)); /* Take the rotation of the skybox into account. */		
	
	return vec4(texture(skybox_texture, rayDir2).rgb, 1.0f);
}

vec4 apply_diffuse_specular(vec3 fragment_colour){

	vec3 light_position = vec3(0.0, 0.0, 15.0); //the location of the light in world coordinates
	vec3 light_colour = vec3(1.0); // white light

	/* Diffuse lighting. */
	
	vec3 light_direction = normalize(light_position - p1); //p1 is the fragment's position in world coordinates
	float incident_degree = max(dot(outFrontfaceNorm, light_direction), 0.0); //intensity of incidence of light ray with face
	vec3 diffuse_contribution = incident_degree * light_colour * 0.8; //portion of the light added by diffuse lighting

	/* Specular lighting. */
	float specular_strength = 0.05f; //overall control knob
	vec3 view_direction = normalize(eye_position - p1);
	vec3 reflection_direction = reflect(-light_direction, outFrontfaceNorm); //the r vector is a reflection of the l vector through the normal vector
	int alpha = 64; //shininess coefficient
	float spec_degree = pow(max(dot(view_direction, reflection_direction), 0.0), alpha); //intensity of light from specular illumination
	vec3 specular_contribution = specular_strength * spec_degree * light_colour;

	//the final colour takes contributions from all 3 local lighting techniques
	vec3 resultant_colour = (diffuse_contribution + specular_contribution) + fragment_colour;

	return vec4(resultant_colour, 1.0f);
}

void main() {
	switch(draw_mode) {
		case 0:
			frag_color = approximate_rt();
			
			if(draw_specular)
				frag_color = apply_diffuse_specular(vec3(frag_color));			
			break;
		case 1:
			frag_color = approximate_rt();
			
			if(draw_specular)
				frag_color = apply_diffuse_specular(vec3(frag_color));			
			break;
		case 2: //front normals
			frag_color = vec4((outFrontfaceNorm + 1.0f)/2.0f, 1.0f);
			break;
		case 3: //back normals
			frag_color = vec4((texture(backface_normals_texture, gl_FragCoord.xy / window_size).rgb + 1.0f) / 2.0f, 1.0f);
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
	}
}
