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

#include "SkyBox.hpp"

#include <iostream>

// window
gps::Window myWindow;
std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
glm::vec3 lampPos;
const GLfloat near_plane = 0.1f, far_plane = 7.5f;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
int worldSizeX = 2, worldSizeZ = 2;
GLuint shadowMapFBO, depthMapTexture;
const unsigned int SHADOW_WIDTH=1024, SHADOW_HEIGHT = 1024;
GLfloat timeOfDay = 1.0f;
float timeSpeed = 0.0001;
bool increaseLight = false;
bool stayLightOrDark = true;
float timeLightOrDark = 200 * timeSpeed;
GLint timeOfDayLoc;
bool flashlightOn = 0;
bool fogOn = 0;
bool animationOn = false;
bool rotate = false;
int totalTimesToRotate, currentRotationTimes;
bool wireframe = false;
bool seeLightCube = false;

float angle_windmill_increment = 0.12f;
float angle_windmill = 0;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 0.0f, 7.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;
GLfloat cameraRotationAngle = 0.01f;

const int GL_WINDOW_WIDTH = 1640;
const int GL_WINDOW_HEIGHT = 900;

GLboolean pressedKeys[1024];
// models
gps::Model3D ground;
gps::Model3D bison;
gps::Model3D lightCube;
gps::Model3D tree;
gps::Model3D lamp;
gps::Model3D wall;
gps::Model3D windmill_building;
gps::Model3D windmill_spining;
gps::Model3D fence;
GLfloat angle;

// shaders
gps::Shader myBasicShader, lightShader, depthMapShader, skyboxShader;


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
    //std::cout << xpos << " " << ypos << "\n";

}

void rotateAtCollision() {
    if (currentRotationTimes >= totalTimesToRotate)
        rotate = false;
    else {
        myCamera.rotate(0, cameraRotationAngle);
        currentRotationTimes++;
    }
}

void CheckForColisionAndMove(gps::MOVE_DIRECTION direction, float speed) {
        gps::Camera newCamera(
            myCamera.getCameraPosition(),
            myCamera.getCameraTarget(),
            myCamera.getCameraUpDirection());
        newCamera.move(direction, speed);
        float newx = newCamera.getCameraPosition().x;
        float newz = newCamera.getCameraPosition().z;
        if (newx <= -19 || newx >= 19 || newz <= -19 || newz >= 19) //out
        {
            if (animationOn) {
                rotate = true;
                totalTimesToRotate = 25 + (std::rand() % (180 - 25 + 1));
                currentRotationTimes = 0;
            }
            else return;

        }
        else {
            myCamera.move(direction, speed);
            view = myCamera.getViewMatrix();
            return;
        }
}

void processCommand() {
    if (pressedKeys[GLFW_KEY_A]) {
        animationOn = not animationOn;
    }
    if (pressedKeys[GLFW_KEY_EQUAL]) {
        timeSpeed += 0.001;
    }
    if (pressedKeys[GLFW_KEY_MINUS]) {
        timeSpeed -= 0.001;
    }
    if (pressedKeys[GLFW_KEY_L]) {
        flashlightOn = not flashlightOn;
    }
    if (pressedKeys[GLFW_KEY_F]) {
        fogOn = not fogOn;
    }
    if (pressedKeys[GLFW_KEY_W]) {
        if (!wireframe) {
            wireframe = true;
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
            wireframe = false;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
    if (pressedKeys[GLFW_KEY_C]) {
        seeLightCube = not seeLightCube;
    }
}

void processMovement() {
    if (pressedKeys[GLFW_KEY_P]) {
        std::cout << myCamera.getCameraPosition().x << " " << myCamera.getCameraPosition().y << " " << myCamera.getCameraPosition().z << "\n";
        
    }
	if (pressedKeys[GLFW_KEY_UP]) {
        CheckForColisionAndMove(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_DOWN]) {
        CheckForColisionAndMove(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_LEFT]) {
        CheckForColisionAndMove(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_RIGHT]) {
        CheckForColisionAndMove(gps::MOVE_RIGHT, cameraSpeed);
    }
    
    if (pressedKeys[GLFW_KEY_G]) {
        myCamera.rotate(0, cameraRotationAngle);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }
    if (pressedKeys[GLFW_KEY_J]) {
        myCamera.rotate(0, -cameraRotationAngle);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }
    if (pressedKeys[GLFW_KEY_H]) {
        myCamera.rotate(cameraRotationAngle, 0);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }
    if (pressedKeys[GLFW_KEY_Y]) {
        myCamera.rotate(-cameraRotationAngle, 0);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
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
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
    glEnable(GL_FRAMEBUFFER_SRGB);
}

void initModels() {
    ground.LoadModel("models/ground/ground.obj");
    bison.LoadModel("models/bison/Bison.obj");
    tree.LoadModel("models/tree/trees9.obj");
    lightCube.LoadModel("models/cube/cube.obj");
    lamp.LoadModel("models/lamp/streetlamp.obj");
    wall.LoadModel("models/farmhouse/Farmhouse.obj");
    windmill_building.LoadModel("models/windmill/Windmill.obj");
    windmill_spining.LoadModel("models/windmill/Spinner.obj");

    fence.LoadModel("models/fence/woodfenceobj.obj");
}

void initShaders() {
    myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
    lightShader.useShaderProgram();
    depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
    depthMapShader.useShaderProgram();
}

void initUniforms() {
	myBasicShader.useShaderProgram();
    timeOfDayLoc = glGetUniformLocation(myBasicShader.shaderProgram, "timeOfDay");
    glUniform1f(timeOfDayLoc, timeOfDay);

    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");
	// create projection matrix
    projection = glm::perspective(glm::radians(45.0f), (float)GL_WINDOW_WIDTH / (float)GL_WINDOW_HEIGHT, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 2.0f, 1.0f);
    //(0.0f, 0.0f, 7.0f),
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));


    lampPos = glm::vec3(-8.0f, 2.0f, 1.0f);
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lampPos"), 1, glm::value_ptr(lampPos));

    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "flashlightOn"), flashlightOn);

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "cameraFrontDirection"), 1, glm::value_ptr(myCamera.getCameraFrontDirection()));

    lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}


void renderLamp(gps::Shader shader, bool depthPass) {
    glm::mat4 modelLamp(1.0f);
    modelLamp = glm::scale(modelLamp, glm::vec3(0.2f, 0.2f, 0.2f));
    modelLamp = glm::translate(modelLamp, glm::vec3(-33.0f, -10.0f, 1.0f));
    glm::mat3 normalMatrixLamp(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelLamp));
    if (!depthPass) {
        normalMatrixLamp = glm::mat3(glm::inverseTranspose(view * modelLamp));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixLamp));
    }
    
    lamp.Draw(shader);
}

void renderFence(gps::Shader shader, bool depthPass) {
    glm::mat4 modelLamp(1.0f);
    modelLamp = glm::scale(modelLamp, glm::vec3(0.05f, 0.05f, 0.05f));
    modelLamp = glm::translate(modelLamp, glm::vec3(-360.0f, -9.0f, 390.0f));
    glm::mat3 normalMatrixLamp(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelLamp));
    if (!depthPass) {
        normalMatrixLamp = glm::mat3(glm::inverseTranspose(view * modelLamp));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixLamp));
    }

    fence.Draw(shader);

    for (int i = 0; i < 7; i++) {
        modelLamp = glm::translate(modelLamp, glm::vec3(100.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelLamp));
        if (!depthPass) {
            normalMatrixLamp = glm::mat3(glm::inverseTranspose(view * modelLamp));
            glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixLamp));
        }

        fence.Draw(shader);
    }
}

void renderWindmill(gps::Shader shader, bool depthPass) {
    glm::mat4 modelWindmill1(1.0f);
    modelWindmill1 = glm::scale(modelWindmill1, glm::vec3(0.1f, 0.1f, 0.1f));
    modelWindmill1 = glm::translate(modelWindmill1, glm::vec3(153.0f, -40.0f, -90.0f));
    glm::mat3 normalMatrixWindmill1(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelWindmill1));
    if (!depthPass) {
        normalMatrixWindmill1 = glm::mat3(glm::inverseTranspose(view * modelWindmill1));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixWindmill1));
    }

    windmill_building.Draw(shader);

    glm::mat4 modelWindmill2(1.0f);

    angle_windmill += angle_windmill_increment;
    if (angle_windmill >= 360)
        angle_windmill = 0;

    modelWindmill2 = glm::scale(modelWindmill2, glm::vec3(0.1f, 0.1f, 0.1f));
    modelWindmill2 = glm::translate(modelWindmill2, glm::vec3(76.2f, 17.5f, -93.0f));
    modelWindmill2 = glm::rotate(modelWindmill2, glm::radians(angle_windmill), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat3 normalMatrixWindmill2(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelWindmill2));
    if (!depthPass) {
        normalMatrixWindmill2 = glm::mat3(glm::inverseTranspose(view * modelWindmill2));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixWindmill2));
    }

    windmill_spining.Draw(shader);
}

void renderWall(gps::Shader shader, bool depthPass) {
    glm::mat4 modelWall(1.0f);
    modelWall = glm::scale(modelWall, glm::vec3(0.3f, 0.3f, 0.3f));
    modelWall = glm::translate(modelWall, glm::vec3(75.0f, -6.8f, 40.0f));
    glm::mat3 normalMatrixWall(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelWall));
    if (!depthPass) {
        normalMatrixWall = glm::mat3(glm::inverseTranspose(view * modelWall));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixWall));
    }

    wall.Draw(shader);

    for (int i = 0; i < 3; i++) {
        modelWall = glm::translate(modelWall, glm::vec3(0.0f, 0.0f, -35.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelWall));
        if (!depthPass) {
            normalMatrixWall = glm::mat3(glm::inverseTranspose(view * modelWall));
            glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixWall));
        }

        wall.Draw(shader);
    }

    modelWall = glm::rotate(glm::mat4(1.0f),glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelWall = glm::scale(modelWall, glm::vec3(0.3f, 0.3f, 0.3f));
    modelWall = glm::translate(modelWall, glm::vec3(75.0f, -6.8f, 46.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelWall));
    if (!depthPass) {
        normalMatrixWall = glm::mat3(glm::inverseTranspose(view * modelWall));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixWall));
    }

    wall.Draw(shader);

    for (int i = 0; i < 3; i++) {
        modelWall = glm::translate(modelWall, glm::vec3(0.0f, 0.0f, -35.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelWall));
        if (!depthPass) {
            normalMatrixWall = glm::mat3(glm::inverseTranspose(view * modelWall));
            glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixWall));
        }

        wall.Draw(shader);
    }

    modelWall = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f));
    modelWall = glm::translate(modelWall, glm::vec3(-75.0f, -6.8f, -65.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelWall));
    if (!depthPass) {
        normalMatrixWall = glm::mat3(glm::inverseTranspose(view * modelWall));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixWall));
    }

    wall.Draw(shader);

    for (int i = 0; i < 3; i++) {
        modelWall = glm::translate(modelWall, glm::vec3(0.0f, 0.0f, 35.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelWall));
        if (!depthPass) {
            normalMatrixWall = glm::mat3(glm::inverseTranspose(view * modelWall));
            glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixWall));
        }

        wall.Draw(shader);
    }
}


void renderTree(gps::Shader shader, bool depthPass) {
    glm::mat4 modelTree(1.0f);
    modelTree = glm::scale(modelTree, glm::vec3(0.1f, 0.1f, 0.1f));
    modelTree = glm::translate(modelTree, glm::vec3(0.0f, -20.0f, -10.0f));
    glm::mat3 normalMatrixTree(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelTree));
    if (!depthPass) {
        normalMatrixTree = glm::mat3(glm::inverseTranspose(view * modelTree));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixTree));
    }
    tree.Draw(shader);
}

void renderBison(gps::Shader shader, bool depthPass) {
    glm::mat4 modelBison(1.0f);
    glm::mat4 viewBison;
    glm::mat4 projectionBison;
    glm::mat3 normalMatrixBison(1.0f);

    shader.useShaderProgram();
    modelBison = glm::rotate(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    modelBison *= glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
    modelBison *= glm::translate(glm::vec3(0.0f, -0.7f, 0.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelBison));
    if (!depthPass) {
        normalMatrixBison = glm::mat3(glm::inverseTranspose(view * modelBison));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixBison));
    }
    
    bison.Draw(shader);

    //bison 2
    modelBison *= glm::translate(glm::vec3(1.0f, 0.0f, -1.0f));
    
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelBison));
    if (!depthPass) {
        normalMatrixBison = glm::mat3(glm::inverseTranspose(view * modelBison));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixBison));
    }
    
    bison.Draw(shader);
}

void renderGround(gps::Shader shader, bool depthPass) {
    glm::mat4 modelGround(1.0f);
    glm::mat4 viewGround;
    glm::mat4 projectionGround;
    glm::mat3 normalMatrixGround(1.0f);
    
    modelGround = glm::mat4(1.0f);
    modelGround *= glm::translate(glm::vec3(0.0f, -2.0f, 0.0f));
    modelGround = glm::translate(modelGround, glm::vec3(10.0f,0,10.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelGround));

    if (!depthPass) {
        normalMatrixGround = glm::mat3(glm::inverseTranspose(view * modelGround));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixGround));
    }
    ground.Draw(shader);
    modelGround = glm::mat4(1.0f);
    modelGround *= glm::translate(glm::vec3(0.0f, -2.0f, 0.0f));
    modelGround = glm::translate(modelGround, glm::vec3(10.0f, 0, -10.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelGround));

    if (!depthPass) {
        normalMatrixGround = glm::mat3(glm::inverseTranspose(view * modelGround));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixGround));
    }
    ground.Draw(shader);
    modelGround = glm::mat4(1.0f);
    modelGround *= glm::translate(glm::vec3(0.0f, -2.0f, 0.0f));
    modelGround = glm::translate(modelGround, glm::vec3(-10.0f, 0, 10.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelGround));

    if (!depthPass) {
        normalMatrixGround = glm::mat3(glm::inverseTranspose(view * modelGround));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixGround));
    }
    ground.Draw(shader);
    modelGround = glm::mat4(1.0f);
    modelGround *= glm::translate(glm::vec3(0.0f, -2.0f, 0.0f));
    modelGround = glm::translate(modelGround, glm::vec3(-10.0f, 0, -10.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelGround));

    if (!depthPass) {
        normalMatrixGround = glm::mat3(glm::inverseTranspose(view * modelGround));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixGround));
    }
    ground.Draw(shader);
    
}

glm::mat4 lightSpaceMatrix()
{
    glm::mat4 lightView = glm::lookAt(glm::vec3(0.0f,5.0f,0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane);

    glm::mat4 lightSpaceMatrix = lightProjection * lightView;
    return lightSpaceMatrix;
}
void renderScene(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();
    renderWall(shader, depthPass);
    renderGround(shader, depthPass);
    renderBison(shader, depthPass);
    renderTree(shader, depthPass);
    renderLamp(shader, depthPass);
    renderWindmill(shader, depthPass);
    renderFence(shader, depthPass);
}

void computeDepthMapAndRender() {
    depthMapShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix()));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    //render scene = draw objects
    renderScene(depthMapShader, true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderWithBasicShader() {
    glViewport(0, 0, GL_WINDOW_WIDTH, GL_WINDOW_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    myBasicShader.useShaderProgram();
    timeOfDayLoc = glGetUniformLocation(myBasicShader.shaderProgram, "timeOfDay");
    glUniform1f(timeOfDayLoc, timeOfDay);

    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "cameraFrontDirection"), 1, glm::value_ptr(myCamera.getCameraFrontDirection()));

    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "flashlightOn"), flashlightOn);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "fogOn"), fogOn);

    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
    //bind the shadow map
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);


    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix()));
    timeOfDayLoc = glGetUniformLocation(myBasicShader.shaderProgram, "timeOfDay");

    renderScene(myBasicShader, false);
}

void drawWhiteCubeAroundLight() {
    lightShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 modelLightCube = glm::mat4(1.0f);
    modelLightCube = glm::translate(modelLightCube, 1.0f * lightDir);
    modelLightCube = glm::scale(modelLightCube, glm::vec3(0.05f, 0.05f, 0.05f));
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelLightCube));

    lightCube.Draw(lightShader);
}

void renderScene() {
    computeDepthMapAndRender();
    renderWithBasicShader();
    if(seeLightCube)
        drawWhiteCubeAroundLight();

    skyboxShader.useShaderProgram();
    timeOfDayLoc = glGetUniformLocation(skyboxShader.shaderProgram, "timeOfDay");
    glUniform1i(glGetUniformLocation(skyboxShader.shaderProgram, "fogOn"), fogOn);
    glUniform1f(timeOfDayLoc, timeOfDay);

    mySkyBox.Draw(skyboxShader, view, projection);
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

void initialize_shadow_things() {
    glGenFramebuffers(1, &shadowMapFBO);

    //create depth texture for FBO
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    //attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

    //bind nothing to attachment points
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    //unbind until ready to use
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initSkyBox()
{
    faces.push_back("textures/skybox3/posx.jpg");
    faces.push_back("textures/skybox3/negx.jpg");
    faces.push_back("textures/skybox3/posy.jpg");
    faces.push_back("textures/skybox3/negy.jpg");
    faces.push_back("textures/skybox3/posz.jpg");
    faces.push_back("textures/skybox3/negz.jpg");

    mySkyBox.Load(faces);
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
        glm::value_ptr(view));

    projection = glm::perspective(glm::radians(45.0f), (float)GL_WINDOW_WIDTH / (float)GL_WINDOW_HEIGHT, 0.1f, 1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
        glm::value_ptr(projection));
}

void processTimePassing() {
    if (stayLightOrDark) {
        if (timeLightOrDark <= 0.0f) {
            stayLightOrDark = false;
        }
        else timeLightOrDark -= timeSpeed;
    }
    else {
        if (increaseLight) {
            if (timeOfDay >= 1.0f) {
                increaseLight = false;
                stayLightOrDark = true;
                timeLightOrDark = 200 * timeSpeed;
            }
            else {
                timeOfDay += timeSpeed;
            }
        }
        else {
            if (timeOfDay <= 0.00f) {
                increaseLight = true;
                stayLightOrDark = true;
                timeLightOrDark = 200 * timeSpeed;
            }
            else {
                timeOfDay -= timeSpeed;
            }
        }
    }

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
    initialize_shadow_things();
    initSkyBox();
    setWindowCallbacks();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processTimePassing();
        processCommand();
        
        if (animationOn) {
            if (rotate) 
                rotateAtCollision();
            else
                CheckForColisionAndMove(gps::MOVE_FORWARD, cameraSpeed);
        }
        else 
            processMovement();
	    renderScene();
		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
