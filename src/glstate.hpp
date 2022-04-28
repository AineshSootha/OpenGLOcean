#ifndef GLSTATE_HPP
#define GLSTATE_HPP
// #include <GL/glew.h>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
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
	void increaseWindDir();

	void paintGL();
	void cleanup();
	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
protected:
	// Initialization
	void initShaders();
	void runFFTShaders(int runCount);
	void runHkShader();
	void runCombineMapsShader();
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
		// glm::vec2 tex;
	};
	std::vector<GridVert> vertices;
	std::vector<GLuint> indices;
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
	std::unique_ptr<Texture> HKxcomputeTexture;
	std::unique_ptr<Texture> HKycomputeTexture;
	std::unique_ptr<Texture> HKzcomputeTexture;


	std::unique_ptr<Compute> Butterflycompute;
	GLuint Butterflycomputeshader;
	std::unique_ptr<Texture> ButterflycomputeTexture;

	std::unique_ptr<Compute> HorizontalFFTcompute;
	GLuint HorizontalFFTcomputeshader;
	std::unique_ptr<Texture> HorizontalFFTcomputeTexture;
	std::unique_ptr<Texture> HorizontalFFTcomputeTexture2;
	std::unique_ptr<Texture> HorizontalFFTcomputeTexture3;


	std::unique_ptr<Compute> VerticalFFTcompute;
	GLuint VerticalFFTcomputeshader;
	std::unique_ptr<Texture> VerticalFFTcomputeTexture;
	std::unique_ptr<Texture> VerticalFFTcomputeTexture2;
	std::unique_ptr<Texture> VerticalFFTcomputeTexture3;



	std::unique_ptr<Compute> InversionFFTcompute;
	GLuint InversionFFTcomputeshader;
	std::unique_ptr<Texture> InversionFFTcomputeTexture1;
	std::unique_ptr<Texture> InversionFFTcomputeTexture2;
	std::unique_ptr<Texture> InversionFFTcomputeTexture3;


	std::unique_ptr<Compute> CombineMapscompute;
	GLuint CombineMapscomputeshader;
	std::unique_ptr<Texture> CombineMapscomputeTexture;

	double currTime;
	GLint log2N;
	std::vector<GLint> reverseIndices;
	bool parametersChanged;

	glm::vec2 windDir;



	// GLuint computeshader;
};

#endif
