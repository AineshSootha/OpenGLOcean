#include <string>
// #include <GL/glew.h>
#include "glad.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
// #include "gl_core_3_3.h"



class Ocean{
public:
    Ocean(GLint _N, GLint _L, GLint _ws, GLint _A, int w = 256, int h = 256);
    int generateRands(int runCall, int w, int h);
    std::vector<GLfloat>  getGaussianRands(int id);
    GLuint texOut;
    GLint getN() {return N;};
    GLint getL() {return L;};
    GLfloat getWindspeed() {return windspeed;};
    GLint getA() {return A;};
    void setWindspeed(GLfloat newSpeed) {windspeed = newSpeed;};

private:
    // std::vector<std::string> randImageStarters;
    std::vector<GLfloat> rands1;
    std::vector<GLfloat> rands2;
    GLint N;
    GLint L;
    GLint windspeed;
    GLint A;
};