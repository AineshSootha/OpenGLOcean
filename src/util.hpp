#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <vector>
// #include <GL/glew.h>
#include "glad.h"
#include <GLFW/glfw3.h>
// #include "gl_core_3_3.h"

GLuint compileShader(GLenum type, const std::string& filename);
GLuint linkProgram(std::vector<GLuint>& shaders);

#endif
