#define NOMINMAX
#include <iostream>
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include "util.hpp"
#include <string>
#include "texture.hpp"
#include <memory>



class Compute{
    public:
        Compute(const char fname[]);
        ~Compute();
        GLuint setupShader(const char fname[]);
        GLuint dispatch(GLuint x, GLuint y);
        GLuint use();
        GLuint program();
    private:
        GLuint x,y;
        GLuint compute_program;

};