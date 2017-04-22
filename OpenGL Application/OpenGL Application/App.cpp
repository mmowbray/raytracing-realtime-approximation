/*
 * Maxwell Mowbray 27005334
 * As part of course requirements for SOEN 491
 * Implementation of realtime ray-tracing approximation conceived by Wyman (2005)
 */

#include "glew.h"
#include "glfw3.h"
#include "glm.hpp"

#include "matrix_transform.hpp"
#include "type_ptr.hpp"

#include <vector>
#include <string>
#include <fstream>

#include "Model.h"
#include <iostream>

glm::vec3 camera_position = glm::vec3(0.0f, 0.0f, 5.0f);

glm::mat4 view_matrix;
glm::mat4 projection_matrix;

const GLuint DEFAULT_WINDOW_WIDTH = 800, DEFAULT_WINDOW_HEIGHT = 800;
const GLfloat CAMERA_MOVEMENT_SPEED = 0.002f;

const char * MODEL_PATH = "../Models/diamond_2.obj";
Model* rayTracingModel;

int screen_width = DEFAULT_WINDOW_WIDTH;
int screen_height = DEFAULT_WINDOW_HEIGHT;

GLuint backface_normals_tex;
GLuint backface_depth_tex;

GLuint loadShaders(std::string vertex_shader_path, std::string fragment_shader_path)
{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_shader_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory?\n", vertex_shader_path.c_str());
		getchar();
		exit(-1);
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_shader_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory?\n", fragment_shader_path.c_str());
		getchar();
		exit(-1);
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_shader_path.c_str());
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, nullptr);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_shader_path.c_str());
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, nullptr);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint program_id = glCreateProgram();
	glAttachShader(program_id, VertexShaderID);
	glAttachShader(program_id, FragmentShaderID);

	glLinkProgram(program_id);

	// Check the program
	glGetProgramiv(program_id, GL_LINK_STATUS, &Result);
	glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(program_id, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(program_id, VertexShaderID);
	glDetachShader(program_id, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return program_id;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);

	screen_width = width;
	screen_height = height;

	// Update the Projection matrix after a window resize event
	projection_matrix = glm::perspective(45.0f, ((float)width / (float)height), 0.1f, 1000.0f);

	/* Resize the backface normals texture. */
	
	glBindTexture(GL_TEXTURE_2D, backface_normals_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screen_width, screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		rayTracingModel->rotate(xpos, ypos);
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
		rayTracingModel->rotate(-1, -1);
	}
}

void scroll_callback(GLFWwindow* window, double, double yoffset)
{
	camera_position.z = glm::clamp(camera_position.z + (float)yoffset, 1.5f, 15.0f);
}

int main()
{

	GLFWwindow* window;

	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_SAMPLES, 4);

	window = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwMakeContextCurrent(window);
	
	glEnable(GL_MULTISAMPLE);

	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		return -1;
	}

	GLuint backface_shader_program = loadShaders("backface.vs", "backface.fs");
	GLuint frontface_shader_program = loadShaders("frontface.vs", "frontface.fs");
	glUseProgram(frontface_shader_program);

	glClearColor(0.4f, 0.2f, 1.0f, 1.0f);

	projection_matrix = glm::perspective(45.0f, (float)DEFAULT_WINDOW_WIDTH / (float)DEFAULT_WINDOW_HEIGHT, 0.1f, 50.0f);

	rayTracingModel = new Model(MODEL_PATH);

	
	
	
	
	
	
	
	
	
	
	
	
	glEnable(GL_DEPTH_TEST);






	/* Grab uniforms handles for both shader programs. */

	GLint backface_model_matrix_id = glGetUniformLocation(backface_shader_program, "model_matrix");
	GLint backface_view_matrix_id = glGetUniformLocation(backface_shader_program, "view_matrix");
	GLint backface_projection_matrix_id = glGetUniformLocation(backface_shader_program, "projection_matrix");

	GLint frontface_model_matrix_id = glGetUniformLocation(frontface_shader_program, "model_matrix");
	GLint frontface_view_matrix_id = glGetUniformLocation(frontface_shader_program, "view_matrix");
	GLint frontface_projection_matrix_id = glGetUniformLocation(frontface_shader_program, "projection_matrix");

	GLint frontface_back_depth_sampler_id = glGetUniformLocation(frontface_shader_program, "backface_depth_texture");
	GLint frontface_back_normals_sampler_id = glGetUniformLocation(frontface_shader_program, "backface_normals_texture");

	GLint frontface_window_size_id = glGetUniformLocation(frontface_shader_program, "window_size");

	/* Declare our backface FBO. */

	GLuint backface_fbo = 0;
	glGenFramebuffers(1, &backface_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, backface_fbo);

	/* Declare the backface normals texture we will render to. */

	glGenTextures(1, &backface_normals_tex);
	glBindTexture(GL_TEXTURE_2D, backface_normals_tex);

	/* Set the backface normals texture to an empty texture. */

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	/* Connect the backface normals texture to the backface FBO. */

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backface_normals_tex, 0);

	/* Declare the backface depth buffer texture. */

	glGenTextures(1, &backface_depth_tex);
	glBindTexture(GL_TEXTURE_2D, backface_depth_tex);

	///* Set the backface depth texture to an empty texture. */

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	///* Connect the backface depth texture to the backface FBO. */

	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, backface_depth_tex, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;

	/* Declare the list of draw buffers. */

	GLenum draw_buffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, draw_buffers);

	/* Bind the backface normals and depth textures so the shaders can sample them. */

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, backface_normals_tex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, backface_depth_tex);

	do
	{
		/* Update transformations. */

		view_matrix = lookAt(camera_position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		/* First Pass. Render the backface depth and normals to the backface FBO. */

		glBindFramebuffer(GL_FRAMEBUFFER, backface_fbo);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, screen_width, screen_height); //remove?

		/* Clear the backface FBO colour and depth buffer textures. */

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* Use the backface shaders. */

		glUseProgram(backface_shader_program);

		/* Send uniforms to the shaders. */

		glUniformMatrix4fv(backface_view_matrix_id, 1, GL_FALSE, value_ptr(view_matrix));
		glUniformMatrix4fv(backface_projection_matrix_id, 1, GL_FALSE, value_ptr(projection_matrix));

		/* Draw backface. */

		glDepthFunc(GL_GREATER);
		rayTracingModel->draw(backface_model_matrix_id);

		/* Second Pass. Render the ray-traced model to the screen. */

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, screen_width, screen_height); //remove?

		/* Clear the screen color and depth buffers. */

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* Use the frontface shaders. */

		glUseProgram(frontface_shader_program);

		/* Set the shaders to sample the backface textures. */

		glUniform1i(frontface_back_normals_sampler_id, 0);
		glUniform1i(frontface_back_depth_sampler_id, 1);

		/* Send other uniforms. */

		glUniformMatrix4fv(frontface_view_matrix_id, 1, GL_FALSE, value_ptr(view_matrix));
		glUniformMatrix4fv(frontface_projection_matrix_id, 1, GL_FALSE, value_ptr(projection_matrix));
		glUniform2f(frontface_window_size_id, screen_width, screen_height);

		/* Draw raytraced model. */

		glDepthFunc(GL_LESS);
		rayTracingModel->draw(frontface_model_matrix_id);

		/* Swap buffers. */

		glfwSwapBuffers(window);
		glfwPollEvents();

	} while (!glfwWindowShouldClose(window));

	delete rayTracingModel;

	glfwTerminate();
	return 0;
}
