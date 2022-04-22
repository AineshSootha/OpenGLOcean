#include <GL/glew.h>
#include <iostream>
#include <memory>
#include <filesystem>
#include <algorithm>
#include "glstate.hpp"
#include <GL/freeglut.h>
#define GLEW_STATIC

namespace fs = std::filesystem;

// Menu identifiers
const int MENU_EXIT = 1;					// Exit application
std::vector<std::string> meshFilenames;		// Paths to .obj files to load

// OpenGL state
std::unique_ptr<GLState> glState;

// Initialization functions
void initGLUT(int* argc, char** argv);
void initMenu();

// Callback functions
void display();
void reshape(GLint width, GLint height);
void keyPress(unsigned char key, int x, int y);
void keyRelease(unsigned char key, int x, int y);
void idle();
void menu(int cmd);
void cleanup();
#define _WIDTH 1280
#define _HEIGHT 720



static void keyboard(unsigned char mkey, int x, int y) {
    switch(mkey) {
        case 0x1b:
            exit(EXIT_SUCCESS);
            break;
    }
}


void initGL(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(_WIDTH, _HEIGHT);
	glutInitWindowPosition(128, 128);
	glutInitContextVersion(4, 3);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	// Create the window
	glutCreateWindow("GPU Ocean Render");

	glutKeyboardFunc(keyboard);

	GLenum err = glewInit();
	if (GLEW_OK != err){
		printf("GLEW init failed: %s!\n", glewGetErrorString(err));
		exit(1);
	}else{
		printf("GLEW init success!n");
	}

}

// Program entry point
int main(int argc, char** argv) {

	// Set window and context settings
	std::cout<<"init\n"<<std::endl;
	try {
		// Create the window and men
		// Initialize OpenGL (buffers, shaders, etc.)
		initGL(argc, argv);
		glState = std::unique_ptr<GLState>(new GLState());
		glState->initializeGL();

	} catch (const std::exception& e) {
		// Handle any errors
		std::cerr << "Fatal error: " << e.what() << std::endl;
		cleanup();
		return -1;
	}
	std::cout<<"init done\n"<<std::endl;
	// Setup window and callbacks


    // initGL();
    // initShaders();
    // initRender();

    while(1) {
        display();
        glutMainLoopEvent();
    }

    return 0;
}

// Called whenever a screen redraw is requested
void display() {
	// Tell the GLState to render the scene
	glState->paintGL();

	// Scene is rendered to the back buffer, so swap the buffers to display it
	glutSwapBuffers();
	// std::cout<<"hello1\n"<<std::endl;
}

void idle(){
	glutPostRedisplay();
	glState->paintGL();
	// std::cout<<"hello2\n"<<std::endl;
}


// Called when the window is closed or the event loop is otherwise exited
void cleanup() {
	// Delete the GLState object, calling its destructor,
	// which releases the OpenGL objects
	glState->cleanup();
	glState.reset(nullptr);
}
