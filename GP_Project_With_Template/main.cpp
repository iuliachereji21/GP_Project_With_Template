#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
int worldSizeX = 50, worldSizeZ = 50;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 0.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;
GLfloat cameraRotationAngle = 0.01f;

const int GL_WINDOW_WIDTH = 1640;
const int GL_WINDOW_HEIGHT = 900;

GLboolean pressedKeys[1024];
// models
gps::Model3D teapot;
gps::Model3D ground;
gps::Model3D bison;
gps::Model3D grass_ground;
GLfloat angle;

// shaders
gps::Shader myBasicShader;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    //TODO
}

void processMovement() {
    if (pressedKeys[GLFW_KEY_P]) {
        std::cout << myCamera.getCameraPosition().x << " " << myCamera.getCameraPosition().y << " " << myCamera.getCameraPosition().z << "\n";
        
    }
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }
    if (pressedKeys[GLFW_KEY_R]) {
        angle -= 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(1, 0, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (pressedKeys[GLFW_KEY_T]) {
        angle += 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(1, 0, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (pressedKeys[GLFW_KEY_G]) {
        myCamera.rotate(0, cameraRotationAngle);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (pressedKeys[GLFW_KEY_J]) {
        myCamera.rotate(0, -cameraRotationAngle);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (pressedKeys[GLFW_KEY_H]) {
        myCamera.rotate(cameraRotationAngle, 0);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (pressedKeys[GLFW_KEY_Y]) {
        myCamera.rotate(-cameraRotationAngle, 0);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
}

void initOpenGLWindow() {
    myWindow.Create(GL_WINDOW_WIDTH, GL_WINDOW_HEIGHT, "OpenGL Project Iulia");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, GL_WINDOW_WIDTH, GL_WINDOW_HEIGHT);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    teapot.LoadModel("models/teapot/teapot20segUT.obj");
    ground.LoadModel("models/ground/ground.obj");
    bison.LoadModel("models/bison/Bison.obj");
    grass_ground.LoadModel("models/gnd/grass.obj");
}

void initShaders() {
	myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
}

void initUniforms() {
	myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 20.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
}

void renderTeapot(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    teapot.Draw(shader);
}
void renderBison(gps::Shader shader) {
    glm::mat4 modelBison(1.0f);
    glm::mat4 viewBison;
    glm::mat4 projectionBison;
    glm::mat3 normalMatrixBison(1.0f);
    // select active shader program
    shader.useShaderProgram();
    modelBison = glm::rotate(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    modelBison *= glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
    modelBison *= glm::translate(glm::vec3(0.0f, -0.7f, 0.0f));
    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBison));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixBison));

    // draw teapot
    bison.Draw(shader);
}

void renderGround(gps::Shader shader) {
    glm::mat4 modelGround(1.0f);
    glm::mat4 viewGround;
    glm::mat4 projectionGround;
    glm::mat3 normalMatrixGround(1.0f);
    // select active shader program
    shader.useShaderProgram();
    //send teapot model matrix data to shader
    //modelGround = glm::rotate(88.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    //modelGround *= glm::scale(glm::vec3(1.0f, 0.3f, 0.3f));
    //modelGround *= glm::translate(glm::vec3(0.0f, -4.0f, 0.0f));
    float TranslatePlace = -20.0f;
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixGround));
    for (int i = 0; i < worldSizeX; i++) {
        for (int j = 0; j < worldSizeZ; j++) {
            modelGround = glm::mat4(1.0f);
            modelGround *= glm::translate(glm::vec3(0.0f, -2.0f, 0.0f));
            modelGround = glm::translate(modelGround, glm::vec3((-(10 + 20 * (worldSizeX / 2)) + 20 * i), 0, (-(10 + 20 * (worldSizeX / 2)) + 20 * j)));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelGround));
            ground.Draw(shader);
        }
    }
        /*glm::mat4 modelGroundI = modelGround;
        modelGround *= glm::translate(glm::vec3(0.0f, 0.0f , i*TranslatePlace));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelGround));
        ground.Draw(shader);
    }*/
    //modelGround *= glm::translate(glm::vec3(0.0f, -7.0f, 0.0f));
    

    //send teapot normal matrix data to shader
    

    // draw teapot
    

    //*********************************
    /*viewGround = myCamera.getViewMatrix();
    modelGround = glm::mat4(1.0f);
    //modelGround = glm::translate(modelGround, glm::vec3((-(10 + 20 * (worldSizeX / 2)) + 20 * i), 0, (-(10 + 20 * (worldSizeZ / 2)) + 20 * j)));

    //send view matrix data to shader	
    //glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    //create normal matrix
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    //send normal matrix data to shader
    //glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    for (int i = 0; i < worldSizeX; i++) {
        for (int j = 0; j < worldSizeZ; j++) {
            //1
            model = glm::mat4(1.0f);

            model = glm::translate(model, glm::vec3((-(10 + 20 * (worldSizeX / 2)) + 20 * i), 0, (-(10 + 20 * (worldSizeZ / 2)) + 20 * j)));

            view = myCamera.getViewMatrix();

            glm::vec4 pos = view * model * glm::vec4(0, 0, 0, 1);

            pos.x = pos.x / pos.w;
            pos.z = pos.z / pos.w;

            if (pos.x < -40 || pos.z < -80 || pos.x > 40 || pos.z > 20)
                continue;
            

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            ground.Draw(shader);
        }
    }*/
}
void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//render the scene

    //renderTeapot(myBasicShader);
    
    renderGround(myBasicShader);
    renderBison(myBasicShader);
    //renderGrassGround(myBasicShader);
	// render the teapot

}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    setWindowCallbacks();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
