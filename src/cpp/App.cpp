#include "App.h"

#include <fstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLSLProgram.h>
#include <Model.h>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

glm::vec3 camera_position = glm::vec3(0.0f, 0.0f, 5.0f);

glm::mat4 view_matrix;
glm::mat4 projection_matrix;

int screen_width = DEFAULT_WINDOW_WIDTH, screen_height = DEFAULT_WINDOW_HEIGHT;

const GLfloat CAMERA_MOVEMENT_SPEED = 0.28f;

GLSLProgram *backShader, *frontShader, *skyboxShader, *metalBandShader;

Model *skybox, *ringDiamond, *ringMetalBand;

double ypos_old = -1, xpos_old = -1;
double ypos_old_skybox = -1, xpos_old_skybox = -1;

int draw_mode = 0;

GLuint backface_normals_tex;
GLuint backface_depth_tex;

GLuint loadCubemap(std::vector<const GLchar*> faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    unsigned char *image;
    for (GLuint i = 0; i < faces.size(); i++)
    {
        int width, height, nrChannels;
        image = stbi_load(faces[i], &width, &height, &nrChannels, 0); 

        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
            GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
        );
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureID;
}

void framebuffer_size_callback(GLFWwindow*, int new_screen_width, int new_screen_height)
{
    glViewport(0, 0, new_screen_width, new_screen_height);
    projection_matrix = glm::perspective(45.0f, (GLfloat)new_screen_width / (GLfloat)new_screen_height, 0.1f, 100.0f);
    screen_width = new_screen_width;
    screen_height = new_screen_height;

    /* Resize the backface normals texture. */

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, backface_normals_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screen_width, screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    /* Resize the backface depth texture. */

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, backface_depth_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, screen_width, screen_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if(ypos_old != -1)
        {

            ringDiamond->setModelMatrix(glm::rotate(ringDiamond->getModelMatrix(), (float)(0.005 * (xpos_old - xpos)), glm::vec3(0.0, 1.0, 0.0)));
            ringDiamond->setModelMatrix(glm::rotate(ringDiamond->getModelMatrix(), (float)(0.005 * (ypos_old - ypos)), glm::vec3(1.0, 0.0, 0.0)));

            ringMetalBand->setModelMatrix(ringDiamond->getModelMatrix());

            ypos_old = ypos;
            xpos_old = xpos;
        }

        ypos_old = ypos;
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        ypos_old = -1;
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        if(ypos_old_skybox != -1)
        {

            skybox->setModelMatrix(glm::rotate(skybox->getModelMatrix(), (float)(0.005 * (xpos_old_skybox - xpos)), glm::vec3(0.0, 1.0, 0.0)));
            skybox->setModelMatrix(glm::rotate(skybox->getModelMatrix(), (float)(0.005 * (ypos_old_skybox - ypos)), glm::vec3(1.0, 0.0, 0.0)));

            ypos_old_skybox = ypos;
            xpos_old_skybox = xpos;
        }

        ypos_old_skybox = ypos;
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
        ypos_old_skybox = -1;
    }

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera_position.z += CAMERA_MOVEMENT_SPEED * yoffset;
    camera_position.z = glm::clamp(camera_position.z, 0.2f, 19.90f);
}

void key_callback(GLFWwindow*, int key, int, int action, int)
{
    if(action == GLFW_PRESS){
        if(key >= GLFW_KEY_1 && key <= GLFW_KEY_6)
            draw_mode = key - GLFW_KEY_1;
     }
}

GLSLProgram* setupShader(const char* shaderVSSource, const char* shaderFSSource)
{
    GLSLProgram* shaderProgram = new GLSLProgram();

    if(!shaderProgram->compileShaderFromFile(shaderVSSource, GL_VERTEX_SHADER))
    {
        printf("Fragment shader failed to compile! %s\n", shaderProgram->log().c_str());
        delete shaderProgram;
        exit(1);
    }

    if (!shaderProgram->compileShaderFromFile(shaderFSSource, GL_FRAGMENT_SHADER))
    {
        printf("Fragment shader failed to compile! %s\n", shaderProgram->log().c_str());
        delete shaderProgram;
        exit(1);
    }

    if (!shaderProgram->link())
    {
        printf("Shader program failed to link! %s\n", shaderProgram->log().c_str());
        delete shaderProgram;
        exit(1);
    }

    shaderProgram->printActiveUniforms();
    shaderProgram->printActiveAttribs();

    return shaderProgram;
}

int main()
{
    GLFWwindow* window;

    if (!glfwInit())
        exit(1);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    glfwWindowHint(GLFW_SAMPLES, 8);

    window = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(1);
    }

    glfwGetFramebufferSize(window, &screen_width, &screen_height);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK)
        exit(1);

    backShader = setupShader("src/glsl/backface.vs", "src/glsl/backface.fs");
    frontShader = setupShader("src/glsl/frontface.vs", "src/glsl/frontface.fs");
    skyboxShader = setupShader("src/glsl/skybox.vs", "src/glsl/skybox.fs");
    metalBandShader = setupShader("src/glsl/metal.vs", "src/glsl/metal.fs");

    ringDiamond = new Model("assets/models/justdiamond.obj");
    ringMetalBand = new Model("assets/models/ringMetal.obj");
    skybox = new Model("assets/models/cube.obj");

    glEnable(GL_DEPTH_TEST);

    projection_matrix = glm::perspective(45.0f, (float)screen_width/(float)screen_height, 0.1f, 100.0f);
    glEnable(GL_MULTISAMPLE);

    GLuint backface_fbo = 0;
    glGenFramebuffers(1, &backface_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, backface_fbo);

    // The texture we're going to render to
    glGenTextures(1, &backface_normals_tex);
    glBindTexture(GL_TEXTURE_2D, backface_normals_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screen_width, screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //NECESSARY!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //NECESSARY!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, backface_normals_tex, 0);

    //Depth texture.
    glGenTextures(1, &backface_depth_tex);
    glBindTexture(GL_TEXTURE_2D, backface_depth_tex);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT, screen_width, screen_height, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //NECESSARY!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //NECESSARY!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, backface_depth_tex, 0);

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        exit(1);

    /* Bind the backface normals and depth textures so the shaders can sample them. */

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, backface_normals_tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, backface_depth_tex);

    /* Load the skybox cubemap texture. */

    std::vector<const GLchar*> faces;

    faces.push_back("assets/textures/posx.jpg");
    faces.push_back("assets/textures/negx.jpg");
    faces.push_back("assets/textures/posy.jpg");
    faces.push_back("assets/textures/negy.jpg");
    faces.push_back("assets/textures/posz.jpg");
    faces.push_back("assets/textures/negz.jpg");

    glActiveTexture(GL_TEXTURE2);
    GLuint cubemapTexture = loadCubemap(faces);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    while (!glfwWindowShouldClose(window))
    {

        //
        view_matrix = glm::lookAt(camera_position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_LESS);

        skyboxShader->use();
        skyboxShader->setUniform("model_matrix", skybox->getModelMatrix());
        skyboxShader->setUniform("projection_matrix", projection_matrix);
        skyboxShader->setUniform("skyboxTexture", 2);

        glDepthMask(GL_FALSE);
        skybox->draw();
        glDepthMask(GL_TRUE);

        metalBandShader->use();
        metalBandShader->setUniform("model_matrix", ringMetalBand->getModelMatrix());
        metalBandShader->setUniform("view_matrix", view_matrix);
        metalBandShader->setUniform("projection_matrix", projection_matrix);
        metalBandShader->setUniform("eye_position", camera_position);
        metalBandShader->setUniform("skybox_model_matrix_inv", glm::inverse(skybox->getModelMatrix()));
        metalBandShader->setUniform("skybox_texture", 2);

        ringMetalBand->draw();

        glBindFramebuffer(GL_FRAMEBUFFER, backface_fbo);
        glClearDepth(0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_GREATER);

        backShader->use();
        backShader->setUniform("model_matrix", ringDiamond->getModelMatrix());
        backShader->setUniform("view_matrix", view_matrix);
        backShader->setUniform("projection_matrix", projection_matrix);

        ringDiamond->draw();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDepthFunc(GL_LESS);
        
        frontShader->use();
        frontShader->setUniform("model_matrix", ringDiamond->getModelMatrix());
        frontShader->setUniform("view_matrix", view_matrix);
        frontShader->setUniform("projection_matrix", projection_matrix);
        frontShader->setUniform("skybox_model_matrix_inv", glm::inverse(skybox->getModelMatrix()));
        frontShader->setUniform("backface_normals_texture", 0);
        frontShader->setUniform("backface_depth_texture", 1);
        frontShader->setUniform("skybox_texture", 2);
        frontShader->setUniform("window_size", screen_width, screen_height);
        frontShader->setUniform("draw_mode", draw_mode);
        frontShader->setUniform("eye_position", camera_position);

        ringDiamond->draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    exit(0);
}
