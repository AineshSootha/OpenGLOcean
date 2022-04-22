#ifndef TEXTURE_H
#define TEXURE_H

#define NOMINMAX
#include <iostream>
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include "util.hpp"
#include <string>


class Texture{
    public:
        Texture(GLint width, GLint height, const GLfloat* data = NULL, GLenum format = GL_RGB);
        ~Texture();
        GLuint id();
        GLuint bind();
        GLuint bindImage(GLint unit, GLenum access);
    private:
        GLuint _id;
        GLuint texOut;
        GLuint _texName;

};

#endif