#ifndef GLSTATE_HPP
#define GLSTATE_HPP
#include <GL/glew.h>
#include <string>
#include <memory>
#include <glm/glm.hpp>
// #include "gl_core_3_3.h"
// #include "camera.hpp"
#include "oceancalcs.hpp"
#include "compute.hpp"
// #include "texture.hpp"

// Manages OpenGL state, e.g. camera transform, objects, shaders
class GLState {
public:
	GLState();
	~GLState();
	// Disallow copy, move, & assignment
	GLState(const GLState& other) = delete;
	GLState& operator=(const GLState& other) = delete;
	GLState(GLState&& other) = delete;
	GLState& operator=(GLState&& other) = delete;
	// Callbacks
	void initializeGL();
	void paintGL();
	void cleanup();
protected:
	// Initialization
	void initShaders();
	GLint reverse(GLint index);
	// OpenGL state
	GLuint shader;		// GPU shader program
	// cameras:
	GLuint vao;			// Vertex array object
	GLuint vbuf;		// Vertex buffer
	GLuint ibuf;		// Index buffer
	GLuint ebuf;

private:
	
	std::unique_ptr<Ocean> mainOcean;
	GLuint length;
	GLuint vcount;
	GLuint totalVcount;
	GLint xFormLoc;
	glm::vec3 camCoords;
	struct GridVert {
		glm::vec3 pos;
		glm::vec2 tex;
	};
	std::vector<GridVert> vertices;
	std::vector<unsigned int> indices;
	float texScale;
	std::unique_ptr<Texture> gaussianRandTexture;
	GLuint gaussianRandId;
	std::unique_ptr<Texture> gaussianRandTextureMinusK;
	GLuint gaussianRandIdMinusK;
	std::unique_ptr<Compute> compute;
	GLuint computeshader;
	std::unique_ptr<Texture> computeTexture;

	std::unique_ptr<Compute> H0compute;
	GLuint H0computeshader;
	std::unique_ptr<Texture> H0computeTexture;

	std::unique_ptr<Compute> H0MinusKcompute;
	GLuint H0MinusKcomputeshader;
	std::unique_ptr<Texture> H0MinusKcomputeTexture;

	std::unique_ptr<Compute> HKcompute;
	GLuint HKcomputeshader;
	std::unique_ptr<Texture> HKcomputeTexture;

	std::unique_ptr<Compute> Butterflycompute;
	GLuint Butterflycomputeshader;
	std::unique_ptr<Texture> ButterflycomputeTexture;

	std::unique_ptr<Compute> HorizontalFFTcompute;
	GLuint HorizontalFFTcomputeshader;
	std::unique_ptr<Texture> HorizontalFFTcomputeTexture;

	GLfloat currTime;
	GLint log2N;
	std::vector<GLint> reverseIndices;
	bool parametersChanged;
	// GLuint computeshader;
};

#endif
