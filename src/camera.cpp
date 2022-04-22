#define NOMINMAX
#include <iostream>
#include "camera.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;

// constructor
Camera::Camera(const CameraType cType) :
	camType(cType),
	width(1), height(1),
	fovy(45.0f),
	view(glm::mat4(1.0f)), proj(glm::mat4(1.0f)) {

	// *********************************** TODO: this line below is hard-coded to help you get a better initial view of the scene, REMOVE it afterwards
	//view = glm::mat4(1.0f); view[3][3] = 1.0f;

	updateViewProj();
}

void Camera::turnLeft() {
	// *********************************** TODO
	currRotO -= rotStep;
	
	glm::mat4 currMat = glm::mat4(1.0f);
	currMat[0][0] = glm::cos(glm::radians(-rotStep));//currRotO));
	currMat[0][2] = glm::sin(glm::radians(-rotStep));//currRotO));
	currMat[2][0] = -glm::sin(glm::radians(-rotStep));//currRotO));
	currMat[2][2] = glm::cos(glm::radians(-rotStep));//currRotO));
	currMat[3][3] = 1.0f;
	view *= currMat;
	std::cout<<currRotO<<std::endl;
	for (int i = 0; i < 4; i++) {  
			for (int j = 0; j < 4; j++) {
				std::cout << view[i][j] << " ";
			}
		std::cout << "\n";
	}
	std::cout << std::endl;
}

void Camera::turnRight() {
	// *********************************** TODO

	glm::mat4 currMat = glm::mat4(1.0f);
	currMat[0][0] = glm::cos(glm::radians(rotStep));//currRotO));
	currMat[0][2] = glm::sin(glm::radians(rotStep));//currRotO));
	currMat[2][0] = -glm::sin(glm::radians(rotStep));//currRotO));
	currMat[2][2] = glm::cos(glm::radians(rotStep));//currRotO));
	currMat[3][3] = 1.0f;
	view *= currMat;
	std::cout<<currRotO<<std::endl;
	for (int i = 0; i < 4; i++) {  
			for (int j = 0; j < 4; j++) {
				std::cout << view[i][j] << " ";
			}
		std::cout << "\n";
	}
	std::cout << std::endl;
	//updateViewProj();
}

void Camera::moveForward() {
	// *********************************** TODO
	//updateViewProj();

	if(camType == OVERHEAD_VIEW){
		glm::mat3 currMat = glm::mat3(0.0f);
		currMat[0][0] = glm::cos(glm::radians(rotStep));//currRotO));
		currMat[0][2] = glm::sin(glm::radians(rotStep));//currRotO));
		currMat[2][0] = -glm::sin(glm::radians(rotStep));//currRotO));
		currMat[2][2] = glm::cos(glm::radians(rotStep));//currRotO));
		//currMat[3][3] = 1.0f;
		
		glm::vec3 res = currMat * glm::vec3(0.0f,0.0f,-1*moveStep);
		//res[33 = 0.0f;
		//view += glm::vec4(res,0.0f);
		view[3][0] += res[0];
		view[3][2] += res[1];
		view[3][1] += res[2];
	}
	for (int i = 0; i < 4; i++) {  
			for (int j = 0; j < 4; j++) {
				std::cout << view[i][j] << " ";
			}
		std::cout << "\n";
	}
	std::cout << std::endl;
}

void Camera::moveBackward() {
	// *********************************** TODO
	if(camType == OVERHEAD_VIEW){
		glm::mat3 currMat = glm::mat3(0.0f);
		currMat[0][0] = glm::cos(glm::radians(rotStep));//currRotO));
		currMat[0][2] = glm::sin(glm::radians(rotStep));//currRotO));
		currMat[2][0] = -glm::sin(glm::radians(rotStep));//currRotO));
		currMat[2][2] = glm::cos(glm::radians(rotStep));//currRotO));
		//currMat[3][3] = 1.0f;
		
		glm::vec3 res = currMat * glm::vec3(0.0f,0.0f,1*moveStep);
		//res[33 = 0.0f;
		//view += glm::vec4(res,0.0f);
		view[3][0] += res[0];
		view[3][2] += res[1];
		view[3][1] += res[2];
	}
	for (int i = 0; i < 4; i++) {  
			for (int j = 0; j < 4; j++) {
				std::cout << view[i][j] << " ";
			}
		std::cout << "\n";
	}
	std::cout << std::endl;
}

void Camera::updateViewProj() {
	// *********************************** TODO
	float aspect_ratio = (GLfloat)width / (GLfloat)height;
	if(camType == GROUND_VIEW){
		// proj = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, -30.0f, 30.0f);
		proj = glm::perspective(fovy, aspect_ratio, 0.1f, 200.0f);
		view = glm::mat4(1.0f);
		// view[1][1] = glm::cos(glm::radians(fovy));
		// view[1][2] = -glm::sin(glm::radians(fovy));
		// view[2][1] = glm::sin(glm::radians(fovy));
		// view[2][2] = glm::cos(glm::radians(fovy));
		//view = glm::translate( view, glm::vec3( 0.0f, 0.0f, -1.0f ) );
		view[3][0] = 0.0f;
		view[3][1] = -2.0f;
		view[3][2] = -25.0f;
		view[3][3] = 1.0f;
		//proj = glm::ortho(0.0f, 6.0f, 0.0f, 6.0f, 0.1f, 6.0f);
	}
	else{
		proj = glm::ortho(-12.0f, 12.0f, -12.0f / aspect_ratio, 12.0f / aspect_ratio, 1.0f, 200.0f);
		view = glm::mat4(1.0f);
		view[1][1] = 0.0f;
		view[1][2] = 1.0f;
		view[2][1] = -1.0f;
		view[2][2] = 0.0f;

		view[3][1] = 0.0f;
		//view[3][0] = -2.0f;
		//  view[3][2] = 15.0f;
		view[3][3] = 1.0f;
	}
	for (int i = 0; i < 4; i++) {  
		for (int j = 0; j < 4; j++) {
			std::cout << proj[i][j] << " ";
		}
		std::cout << "\n";
	}
	std::cout << std::endl;

	// std::cout<<proj[2][3]<<std::endl;
	//view
	//proj = glm::ortho()
}
