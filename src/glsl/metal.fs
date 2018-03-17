#version 330 core

in vec3 outBackfaceNorm;
in vec3 frag_position;

out vec4 frag_color;

uniform vec3 eye_position;

uniform samplerCube skybox_texture;

uniform mat4 skybox_model_matrix_inv;

#define silver_ambient vec3(189,141,83)/255.0
#define silver_diffuse vec3(244,192,153)/255.0
#define silver_specular vec3(252,254,232)/255.0

#define gold_ambient vec3(1.0f, 0.843f, 0.0f)
#define gold_diffuse vec3(244,192,153)/255.0
#define gold_specular vec3(252,254,232)/255.0

void main() {

	vec3 lightPos = vec3( 0.0f, 2.5f, 6.0f);
	vec3 L = normalize(lightPos - frag_position);
	
	float lambertian = max(dot(outBackfaceNorm, L), 0.0);

	float specular = 0.0;

	if(lambertian > 0.0) {
		vec3 R = reflect(-L, outBackfaceNorm);      // Reflected light vector
		vec3 V = normalize(-frag_position); // Vector to viewer

		float specAngle = max(dot(R, V), 0.0);
		specular = pow(specAngle, 16);
	}

	vec3 ambientColor = silver_ambient,  diffuseColor = silver_diffuse, specularColor = silver_specular;

	float Ka = 0.89;
	float Kd = 0.43;
	float Ks = 0.3;
    
    frag_color = vec4(Ka * ambientColor + Kd * lambertian * diffuseColor + Ks * specular * specularColor, 1.0);

	vec3 eye_direction = normalize(frag_position - eye_position);
	vec3 reflection_ray = reflect(eye_direction, outBackfaceNorm);

	frag_color = mix(frag_color, texture(skybox_texture, vec3(skybox_model_matrix_inv * vec4(reflection_ray, 0.0f))), vec4(0.8, 0.8, 0.8, 0.5f));
}
