#include "compute.hpp"


Compute::Compute(const char fname[]){
    setupShader(fname);
}

Compute::~Compute(){
    return;
}

GLuint Compute::setupShader(const char fname[]){
    GLuint compute_shader = compileShader(GL_COMPUTE_SHADER, fname);
	// check for compilation errors as per normal here
	compute_program = glCreateProgram();
	glAttachShader(compute_program, compute_shader);
	glLinkProgram(compute_program);
    glDeleteShader(compute_shader);
    return compute_program;
}

GLuint Compute::dispatch(GLuint x, GLuint y){
    glDispatchCompute((GLuint)x, (GLuint)y, 1);
    return 0;
}

GLuint Compute::program(){
    return compute_program;
}

GLuint Compute::use(){
    glUseProgram(compute_program);
    return 0;
}
// GLuint Compute::use()