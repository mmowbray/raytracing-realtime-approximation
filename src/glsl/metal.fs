#version 330 core

in vec3 outBackfaceNorm;
in vec3 frag_position;

out vec4 frag_color;

uniform vec3 eye_position;

uniform samplerCube skybox_texture;
uniform mat4 skybox_model_matrix_inv;

#define gold_yellow vec3(1.0f, 0.843f, 0.0f)
#define silver vec3(0.752f)

void main() {

	vec3 lightPos = vec3( 0.0f, 2.5f, 6.0f);
	vec3 L = normalize(lightPos - frag_position);

	// Lambert's cosine law
	float lambertian = max(dot(outBackfaceNorm, L), 0.0);

	float specular = 0.0;

	if(lambertian > 0.0) {
		vec3 R = reflect(-L, outBackfaceNorm);      // Reflected light vector
		vec3 V = normalize(-frag_position); // Vector to viewer

		// Compute the specular term
		float specAngle = max(dot(R, V), 0.0);
		specular = pow(specAngle, 16);
	}

	vec3 ambientColor = vec3(189,141,83)/255.0;
	vec3 diffuseColor = vec3(244,192,153)/255.0;
	vec3 specularColor = vec3(252,254,232)/255.0;

	ambientColor = vec3(143,149,173)/255.0;
	diffuseColor = vec3(0,0,0)/255.0;
	specularColor = vec3(254,254,254)/255.0;

	float Ka = 0.89;
	float Kd = 0.43;
	float Ks = 0.3;
    
    frag_color = vec4(Ka * ambientColor +
                      Kd * lambertian * diffuseColor +
                      Ks * specular * specularColor, 1.0);


	vec3 eye_direction = normalize(frag_position - eye_position);
	vec3 reflection_ray = reflect(eye_direction, outBackfaceNorm);

	frag_color = mix(frag_color, texture(skybox_texture, vec3(skybox_model_matrix_inv * vec4(reflection_ray, 0.0f))), vec4(0.8, 0.8, 0.8, 1.0f));
}
