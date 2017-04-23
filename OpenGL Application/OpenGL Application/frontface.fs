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
	
	/* Compute the incident light ray v. */
	
	vec3 v = p1 - eye_position;

	/* Compute T1, the refracted ray. */
	
	float n_air = 1.0f;
	float n_glass = 1.44f;
	
	vec3 t1 = refract(v, outFrontfaceNorm, n_glass/n_air);

	/* Compute the approximations for P2. */
	
	float d = abs(texture(backface_depth_texture, gl_FragCoord.xy / window_size).z - gl_FragCoord.z);
	//d = abs(gl_FragCoord.z);
	
	vec3 p2 = p1 + d * t1;
	
	/* Compute the approximations for N2. */
	
	vec2 n2_uv = vec2(p2.x + 1, 1 - p2.y) * (window_size / 2);
	vec3 n2 = texture(backface_normals_texture, n2_uv).xyz;
	
	/* Compute T2. */
	
	vec3 t2 = refract(t1, n2, n_air/n_glass);

	/* Project the doubly refracted ray T2 into the environment. */
	
	/* Take the rotation of the skybox into account. */
	
	t2 = vec3(inverse(skybox_model_matrix) * vec4(t2, 1.0f)); //skybox rotation
	
	return vec4(texture(skybox_texture, t2).rgb, 1.0f);
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
			frag_color = vec4((outFrontfaceNorm + 1.0f)/2.0f, 1.0f); //front normals
			break;
		case 2:
			frag_color = vec4((texture(backface_normals_texture, gl_FragCoord.xy / window_size).rgb + 1.0f)/2.0f, 1.0f);
			break;
		case 3:
			//source: https://learnopengl.com/#!Advanced-OpenGL/Depth-testing
			float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
			frag_color = vec4(vec3(depth), 1.0f);
			break;
		case 4:
			frag_color = vec4(texture(backface_depth_texture, gl_FragCoord.xy / window_size).zzz, 1); //back depth
			break;
	}
}
