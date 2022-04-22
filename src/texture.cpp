#include "texture.hpp"


Texture::Texture(GLint width, GLint height, const GLfloat* data /* = NULL*/, GLenum format /* = GL_RGB*/) : _id(-1) {
    glBindTexture(GL_TEXTURE_2D,0);
    glGenTextures(1, &_id);    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT,
    data);
    // glBindTexture(GL_TEXTURE_2D, 0);
    //AUTH id: 15637907
    //Code: 16747
}

Texture::~Texture(){
    return;
}

GLuint Texture::id(){
    return _id;
}

GLuint Texture::bind(){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _id);
    return 0;
}


GLuint Texture::bindImage(GLint unit, GLenum access){
    glBindImageTexture(unit, _id, 0, GL_FALSE, 0, access, GL_RGBA32F);
    return 0;
}