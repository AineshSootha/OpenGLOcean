// #include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <iostream>
#include <memory>
#include <filesystem>
#include <algorithm>
#include "glstate.hpp"
// #include <GL/freeglut.h>
#include "glad.h"
#include <GLFW/glfw3.h>

#define GLEW_STATIC

namespace fs = std::filesystem;

// Menu identifiers
const int MENU_EXIT = 1;					// Exit application
std::vector<std::string> meshFilenames;		// Paths to .obj files to load

// OpenGL state
std::unique_ptr<GLState> glState;
double lastX = 512, lastY = 512;
double xOffset = 0.0, yOffset = 0.0;
GLfloat yaw = -90.0f;
GLfloat pitch =  0.0f;
bool firstMouse = true;
// Initialization functions
void initGLUT(int* argc, char** argv);
void initMenu();

// Callback functions
void display(GLFWwindow* window);
void reshape(GLint width, GLint height);
void keyPress(unsigned char key, int x, int y);
void keyRelease(unsigned char key, int x, int y);
void idle();
void menu(int cmd);
void cleanup();
void processInput(GLFWwindow *window);
float deltaTime = 0.0f;
float lastFrame = 0.0f;
#define _WIDTH 1280
#define _HEIGHT 720



static void keyboard(unsigned char mkey, int x, int y) {
    switch(mkey) {
        case 0x1b:
            exit(EXIT_SUCCESS);
            break;
    }
}


void window_size_callback(GLFWwindow* window, int width, int height)
{
	glState->resize(width, height);
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	//getting cursor position
	
}  

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
 
}

// Program entry point
int main(int argc, char** argv) {
	
	// Set window and context settings
	std::cout<<"init\n"<<std::endl;
	GLFWwindow* window;
	try {
		// Create the window and men
		// Initialize OpenGL (buffers, shaders, etc.)
		// initGL(argc, argv);
		glfwInit();

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	

		window = glfwCreateWindow(1024, 1024, "GPU REALISTIC OCEAN WAVES | AINESH SOOTHA", NULL, NULL);
		if (!window)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return -1;
		}
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetWindowSizeCallback(window, window_size_callback);
		glfwSetMouseButtonCallback(window, mouse_button_callback);
		glfwMakeContextCurrent(window);	
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			return -1;
		}  
		
		glState = std::unique_ptr<GLState>(new GLState());
		glState->initializeGL();
	

	} catch (const std::exception& e) {
		// Handle any errors
		std::cerr << "Fatal error: " << e.what() << std::endl;
		cleanup();
		return -1;
	}
	// IMGUI_CHECKVERSION();
	// ImGui::CreateContext();
	// ImGuiIO &io = ImGui::GetIO();
	// // Setup Platform/Renderer bindings
	// bool x = ImGui_ImplGlfw_InitForOpenGL(window, true);
	// std::cout<<x<<std::endl;
	// x = ImGui_ImplOpenGL3_Init("#version 430");
	// std::cout<<x<<std::endl;

	// // Setup Dear ImGui style
	// ImGui::StyleColorsDark();
	std::cout<<"init done\n"<<std::endl;
		
	while(!glfwWindowShouldClose(window))
	{
		
		float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
		
		
		
		processInput(window);
		display(window);
		// ImGui_ImplOpenGL3_NewFrame();
        // ImGui_ImplGlfw_NewFrame();
        // ImGui::NewFrame();
		
		// bool x = ImGui::Begin("Demo window");
        // ImGui::TextColored(ImVec4(1.0,1.0,1.0,1.0), "HELLO");
		// // std::cout<<x<<std::endl;
        // ImGui::End();
		// // Render dear imgui into screen
		// ImGui::EndFrame();
        // ImGui::Render();
        // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		glfwPollEvents();    
	}
	// ImGui_ImplOpenGL3_Shutdown();
    // ImGui_ImplGlfw_Shutdown();
    // ImGui::DestroyContext();
	glfwDestroyWindow(window);
	
	cleanup();
    glfwTerminate();
	std::cout<<"Terminated"<<std::endl;
	return 0;
}

// Called whenever a screen redraw is requested
void display(GLFWwindow* window) {
	// Tell the GLState to render the scene
	// std::cout<<"in display\n";
	
	glState->paintGL();
	glfwSwapBuffers(window);
}


void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = static_cast<float>(0.5 * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        glState->cameraPos += cameraSpeed * glState->cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        glState->cameraPos -= cameraSpeed * glState->cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        glState->cameraPos -= glm::normalize(glm::cross(glState->cameraFront, glState->cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        glState->cameraPos += glm::normalize(glm::cross(glState->cameraFront, glState->cameraUp)) * cameraSpeed;
	if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
		xOffset = 0.0;
		yOffset = 0.3;
		yaw   += xOffset;
		pitch += yOffset;

		if(pitch > 89.0f)
			pitch = 89.0f;
		if(pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		glState -> cameraFront = glm::normalize(direction);
	}
	if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
		xOffset = 0.0;
		yOffset = -0.3;
		yaw   += xOffset;
		pitch += yOffset;

		if(pitch > 89.0f)
			pitch = 89.0f;
		if(pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		glState -> cameraFront = glm::normalize(direction);
	}
	if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
		yOffset = 0.0;
		xOffset = 0.3;
		yaw   += xOffset;
		pitch += yOffset;

		if(pitch > 89.0f)
			pitch = 89.0f;
		if(pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		glState -> cameraFront = glm::normalize(direction);
	}
	if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
		yOffset = 0.0;
		xOffset = -0.3;
		yaw   += xOffset;
		pitch += yOffset;

		if(pitch > 89.0f)
			pitch = 89.0f;
		if(pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		glState -> cameraFront = glm::normalize(direction);
	}
	if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		glState->increaseWindDir();
	if(glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS)
		glState->increaseWindSpeed();
	if(glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
		glState->decreaseWindSpeed();
	
	if(glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS){
		glState->triggerWireframe();
		// std::cout<<"Wireframe Mode: "<<std::endl;
	}
}
// Called when the window is closed or the event loop is otherwise exited
void cleanup() {
	// Delete the GLState object, calling its destructor,
	// which releases the OpenGL objects
	glState->cleanup();
	glState.reset(nullptr);
}
