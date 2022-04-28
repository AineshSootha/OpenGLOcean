#define NOMINMAX
#include <iostream>
#include <chrono>
#include <thread>
#include "glstate.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "util.hpp"


#define RES 512
#define GRID 256

//TESTING VARIABLES
std::ofstream ofs, ofs2;
int w = GRID, h = GRID;
std::vector<float> compute_data(w*h*2);
std::vector<float> compute_data3(w*h*3);
std::vector<float> compute_data4(w*h*4);

GLint activeText = -1;
//

// Constructor
GLState::GLState() :
	shader(0),
	vcount(RES + 1),
	totalVcount(RES * RES * 6),
	xFormLoc(0),
	currTime(0.0f),
	log2N(log(GRID) / log(2)),
	camCoords(0.0f, -0.5f, 0.5f),
	reverseIndices(GRID, 0),
	vertices(vcount * vcount),
	windDir(1.0f, 0.0f),
	cameraPos(0.0f, 0.0f,  3.0f),
	cameraFront(0.0f, 0.0f, -1.0f),
	cameraUp(0.0f, 1.0f,  0.0f),
	// indices(32*32*6),//totalVcount),
	parametersChanged(true) {}
	
// Destructor
GLState::~GLState() {
	// Release OpenGL resources
	if (shader)	glDeleteProgram(shader);
	if (vao)	glDeleteVertexArrays(1, &vao);
	if (vbuf)	glDeleteBuffers(1, &vbuf);
	if (ibuf)	glDeleteBuffers(1, &ibuf);
}

void GLState::cleanup(){
	return;
}

GLint GLState::reverse(GLint index){
	GLint res = 0;
	for(int i = 0; i < log2N; i++){
		res = (res << 1) + (index & 1);
        index >>= 1;
	}
	return res;
}

void GLState::increaseWindDir(){
	windDir.y += 1.0f;
	windDir = glm::normalize(windDir);
	parametersChanged = true;
}
// Called when OpenGL context is created (some time after construction)
void GLState::initializeGL() {
	// General settings
	std::cout<<std::endl<<GL_MAX_TEXTURE_UNITS<<std::endl;
	windDir = glm::normalize(windDir);
	initShaders();

	int idx = 0;
	for(int j=-RES/2; j<=RES/2; ++j) {
		for(int i=-RES/2; i<=RES/2; ++i) {
			float x = 2 * (float)i/(float)RES;
			float y = 2 * (float)j/(float)RES;
			float z = 0.0;
			vertices[idx++].pos = (glm::vec3(x, y, z));
			float u = ((float)x / RES) + 0.5f;
			float v = ((float)z / RES) + 0.5f;
			// vertices[idx++].tex = glm::vec2(u, v) * 2.0f;
		}
	}
	assert(idx == vertices.size());
	idx = 0;
	
	//CounterClockwiseWinding
	for(int j=0; j<RES; ++j) {
        for(int i=0; i<RES; ++i) {
			int row1 =  j    * (RES+1);
			int row2 = (j+1) * (RES+1);
			indices.push_back(row1+i);
			indices.push_back(row2+i+1);
          	indices.push_back(row2+i);
			indices.push_back(row2+i+1);
			indices.push_back(row1+i);
			indices.push_back(row1+i+1);

        }
      }

	// length = (GLuint)indices.size() * 4;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbuf);
    glGenBuffers(1, &ibuf);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbuf);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GridVert), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (void*)0);
	// glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


	//GaussianRand Gen
	mainOcean = std::unique_ptr<Ocean>(new Ocean(GRID, 1000, 40, 2, GRID, GRID));
	mainOcean->generateRands(1, GRID,GRID);
	
	
	std::vector<GLfloat> gaussianRands = mainOcean -> getGaussianRands(1);
	std::vector<GLfloat> gaussianRandsMinusK = mainOcean -> getGaussianRands(2);
	gaussianRandTexture = std::unique_ptr<Texture>(new Texture(GRID, GRID, gaussianRands.data(), GL_RGBA));
	gaussianRandId = gaussianRandTexture->id();
	// glActiveTexture(GL_TEXTURE1);
	// glBindTexture(GL_TEXTURE_2D, gaussianRandId);
	// glGetTexImage( GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, compute_data.data() );
	// w = GRID;
	// h = GRID;
	// ofs.open("temp/test_randGauss.ppm", std::ios_base::out | std::ios_base::binary);
	// ofs2.open("temp/test_randGauss.txt");
	// ofs << "P6" << std::endl << w << ' ' << h << std::endl << "255" << std::endl;    
	// for(int curr = 0; curr < w*h*2; curr+=2){
	// 	ofs << (char) (compute_data.at(curr + 0) * 256) << (char) (compute_data.at(curr + 1) * 256) << (char) (0);
	// 	ofs2 <<  (compute_data.at(curr + 0)) << " " << (compute_data.at(curr + 1)) << " " <<  (0) << '\n';
	// }
	// ofs.close();
	// ofs2.close();

	gaussianRandTextureMinusK = std::unique_ptr<Texture>(new Texture(GRID, GRID, gaussianRandsMinusK.data(), GL_RGBA));
	gaussianRandIdMinusK = gaussianRandTextureMinusK->id();

	//H0 compute shader setup
	H0compute = std::unique_ptr<Compute>(new Compute("shaders/h0KCompute.glsl"));
	H0computeTexture = std::unique_ptr<Texture>(new Texture(GRID, GRID, NULL, GL_RGBA32F));
	H0computeshader = H0compute->program();

	activeText = -1;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
	// std::cout<<activeText<<H0computeTexture->id()<<std::endl;
	
	//H0(-k) compute shader setup
	H0MinusKcompute = std::unique_ptr<Compute>(new Compute("shaders/h0MinusKCompute.glsl"));
	H0MinusKcomputeTexture = std::unique_ptr<Texture>(new Texture(GRID, GRID, NULL, GL_RGBA32F));
	H0MinusKcomputeshader = H0MinusKcompute->program();

	activeText = -1;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
	// std::cout<<activeText<<H0computeTexture->id()<<std::endl;

	//H(k,t) compute shader setup
	HKcompute = std::unique_ptr<Compute>(new Compute("shaders/hKCompute.glsl"));
	HKxcomputeTexture = std::unique_ptr<Texture>(new Texture(GRID, GRID, NULL, GL_RGBA32F));
	HKycomputeTexture = std::unique_ptr<Texture>(new Texture(GRID, GRID, NULL, GL_RGBA32F));
	HKzcomputeTexture = std::unique_ptr<Texture>(new Texture(GRID, GRID, NULL, GL_RGBA32F));

	HKcomputeshader = HKcompute->program();

	activeText = -1;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
	// std::cout<<activeText<<H0computeTexture->id()<<std::endl;

	for(int i = 0; i < GRID; i++){
		reverseIndices[i] = reverse(i);
		std::cout<<reverseIndices[i]<<std::endl;
	}



	HorizontalFFTcompute = std::unique_ptr<Compute>(new Compute("shaders/fftHorizontalCompute.glsl"));
	HorizontalFFTcomputeTexture = std::unique_ptr<Texture>(new Texture(GRID, GRID, NULL, GL_RGBA32F));
	HorizontalFFTcomputeTexture2 = std::unique_ptr<Texture>(new Texture(GRID, GRID, NULL, GL_RGBA32F));
	HorizontalFFTcomputeTexture3 = std::unique_ptr<Texture>(new Texture(GRID, GRID, NULL, GL_RGBA32F));
	HorizontalFFTcomputeshader = HorizontalFFTcompute->program();

	VerticalFFTcompute = std::unique_ptr<Compute>(new Compute("shaders/fftVerticalCompute.glsl"));
	VerticalFFTcomputeTexture = std::unique_ptr<Texture>(new Texture(GRID, GRID, NULL, GL_RGBA32F));
	VerticalFFTcomputeTexture2 = std::unique_ptr<Texture>(new Texture(GRID, GRID, NULL, GL_RGBA32F));
	VerticalFFTcomputeTexture3 = std::unique_ptr<Texture>(new Texture(GRID, GRID, NULL, GL_RGBA32F));
	VerticalFFTcomputeshader = VerticalFFTcompute->program();

	InversionFFTcompute = std::unique_ptr<Compute>(new Compute("shaders/invertfftCompute.glsl"));
	InversionFFTcomputeTexture1 = std::unique_ptr<Texture>(new Texture(GRID, GRID, NULL, GL_RGBA32F));
	InversionFFTcomputeTexture2 = std::unique_ptr<Texture>(new Texture(GRID, GRID, NULL, GL_RGBA32F));
	InversionFFTcomputeTexture3 = std::unique_ptr<Texture>(new Texture(GRID, GRID, NULL, GL_RGBA32F));
	InversionFFTcomputeshader = InversionFFTcompute->program();
	
	
	CombineMapscompute = std::unique_ptr<Compute>(new Compute("shaders/combineDispMaps.glsl"));
	CombineMapscomputeTexture = std::unique_ptr<Texture>(new Texture(GRID, GRID, NULL, GL_RGBA32F));
	CombineMapscomputeshader = CombineMapscompute->program();
	

	NormalMapcompute = std::unique_ptr<Compute>(new Compute("shaders/normalMapCompute.glsl"));
	NormalMapcomputeTexture = std::unique_ptr<Texture>(new Texture(GRID, GRID, NULL, GL_RGBA32F));
	NormalMapcomputeshader = NormalMapcompute->program();
	

	activeText = -1;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
	// std::cout<<activeText<<H0computeTexture->id()<<std::endl;

}




// Called when window requests a screen redraw
void GLState::paintGL() {
	glViewport(-1.0,  -1.0, 2048, 2048);
	glClearColor(0.4f, 0.3f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	if(parametersChanged){
		std::cout<<"Changed Entry\n";
		// BUILD H0 IMAGE
		H0compute->use();
		glUniform1i(glGetUniformLocation(H0computeshader, "N"), mainOcean->getN());
		glUniform1i(glGetUniformLocation(H0computeshader, "startL"), mainOcean->getL());
		glUniform1i(glGetUniformLocation(H0computeshader, "V"), mainOcean->getWindspeed());
		glUniform1i(glGetUniformLocation(H0computeshader, "A"), mainOcean->getA());
		glUniform2f(glGetUniformLocation(H0computeshader, "windDir"),windDir[0], windDir[1]);
		glUniform1i(glGetUniformLocation(H0computeshader, "randSampler1"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gaussianRandId);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, H0computeTexture->id());
		H0computeTexture->bindImage(0, GL_WRITE_ONLY);

		activeText = -1;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
		// std::cout<<activeText<<H0computeTexture->id()<<std::endl;
		
		glDispatchCompute(GRID / 16, GRID / 16, 1);
		glMemoryBarrier( GL_ALL_BARRIER_BITS );
		//TESTING ONLY - h0Test
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, H0computeTexture->id());
		H0computeTexture->bindImage(0, GL_WRITE_ONLY);
		glGetTexImage( GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, compute_data.data() );
		w = GRID;
		h = GRID;
		ofs.open("temp/test_h0.ppm", std::ios_base::out | std::ios_base::binary);
		ofs2.open("temp/test_h0.txt");
		ofs << "P6" << std::endl << w << ' ' << h << std::endl << "255" << std::endl;    
		for(int curr = 0; curr < w*h*2; curr+=2){
			ofs << (char) (compute_data.at(curr + 0) * 256) << (char) (compute_data.at(curr + 1) * 256) << (char) (0);
			ofs2 <<  (compute_data.at(curr + 0)) << " " << (compute_data.at(curr + 1)) << " " <<  (0) << '\n';
		}
		ofs.close();
		ofs2.close();
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		// glDisable(GL_TEXTURE_2D);
		glUseProgram(0);

		// BUILD H0(-k) IMAGE
		H0MinusKcompute->use();
		glUniform1i(glGetUniformLocation(H0MinusKcomputeshader, "N"), mainOcean->getN());
		glUniform1i(glGetUniformLocation(H0MinusKcomputeshader, "startL"), mainOcean->getL());
		glUniform1i(glGetUniformLocation(H0MinusKcomputeshader, "V"), mainOcean->getWindspeed());
		glUniform1i(glGetUniformLocation(H0MinusKcomputeshader, "A"), mainOcean->getA());
		glUniform1i(glGetUniformLocation(H0MinusKcomputeshader, "randSampler1"), 1);
		glUniform2f(glGetUniformLocation(H0MinusKcomputeshader, "windDir"),windDir[0], windDir[1]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gaussianRandIdMinusK);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, H0MinusKcomputeTexture->id());
		H0MinusKcomputeTexture->bindImage(0, GL_WRITE_ONLY);

		activeText = -1;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
		// std::cout<<activeText<<H0MinusKcomputeTexture->id()<<std::endl;
		
		glDispatchCompute(GRID / 16, GRID / 16, 1);
		glMemoryBarrier( GL_ALL_BARRIER_BITS );
		//TESTING ONLY - h0Test
		glGetTexImage( GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, compute_data.data() );
		ofs.open("temp/test_h0MinusK.ppm", std::ios_base::out | std::ios_base::binary);
		ofs2.open("temp/test_h0MinusK.txt");
		ofs << "P6" << std::endl << w << ' ' << h << std::endl << "255" << std::endl;    
		for(int curr = 0; curr < w*h*2; curr+=2){
			ofs << (char) (compute_data.at(curr + 0) * 256) << (char) (compute_data.at(curr + 1) * 256) << (char) (0);
			ofs2 <<  (compute_data.at(curr + 0) * 256) << " " << (compute_data.at(curr + 1) * 256) << " " <<  (0) << '\n';
		}
		ofs.close();
		ofs2.close();
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		// glDisable(GL_TEXTURE_2D);
		glUseProgram(0);


		


		parametersChanged = false;
		std::cout<<"Changed Exit\n";
	}

	runHkShader();

	runFFTShaders(1);
	
	runFFTShaders(2);

	runFFTShaders(3);

	runCombineMapsShader();

	runNormalMapShader();

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	// glDisable(GL_TEXTURE_2D);
	glUseProgram(0);
	glFinish();
	int N = mainOcean->getN();
	

	glUseProgram(shader);
	glm::mat4 xform(1.0f);
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	view = glm::rotate(view, glm::radians(-45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	view = glm::rotate(view, glm::radians(camCoords.x), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 pos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,0.0f,0.0f));
	xform = proj * view;//* pos;// * fixBB;
	glUniformMatrix4fv(xFormLoc, 1, GL_FALSE, glm::value_ptr(xform));
	glUniform1f(glGetUniformLocation(shader, "a_time"), currTime);
	glUniform1i(glGetUniformLocation(shader, "dispMapZ"), 2);
	glUniform1i(glGetUniformLocation(shader, "dispMapX"), 3);
	glUniform1i(glGetUniformLocation(shader, "dispMapY"), 4);
	glUniform1i(glGetUniformLocation(shader, "dispMap"), 5);
	glUniform1f(glGetUniformLocation(shader, "speed"), mainOcean->getWindspeed());
	glUniform2f(glGetUniformLocation(shader, "windDir"), windDir[0], windDir[1]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, InversionFFTcomputeTexture1->id());
	InversionFFTcomputeTexture1->bindImage(2, GL_READ_ONLY);
	activeText = -1;
	// glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
	// std::cout<<activeText<<InversionFFTcomputeTexture1->id()<<std::endl;
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, InversionFFTcomputeTexture2->id());
	InversionFFTcomputeTexture2->bindImage(3, GL_READ_ONLY);


	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, InversionFFTcomputeTexture3->id());
	InversionFFTcomputeTexture3->bindImage(4, GL_READ_ONLY);
  	glBindVertexArray(vao);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, CombineMapscomputeTexture->id());
	CombineMapscomputeTexture->bindImage(5, GL_READ_ONLY);
  	glBindVertexArray(vao);

  	// glDrawElements(GL_LINES, length, GL_UNSIGNED_INT, NULL);
	glDrawElements(GL_TRIANGLES, totalVcount,  GL_UNSIGNED_INT, 0);

  	glBindVertexArray(0);

	glUseProgram(0);
	currTime += (glfwGetTime() - currTime);
	currTime = glfwGetTime();	
  	// glDisable(GL_DEPTH_TEST);
}

// Create shaders and associated state
void GLState::initShaders() {
	// Compile and link shader files
	std::vector<GLuint> shaders;
	shaders.push_back(compileShader(GL_VERTEX_SHADER, "shaders/v.glsl"));
	
	int success;
    char infoLog[512];
    glGetShaderiv(shaders[0], GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shaders[0], 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
	shaders.push_back(compileShader(GL_FRAGMENT_SHADER, "shaders/f.glsl"));

	glGetShaderiv(shaders[1], GL_COMPILE_STATUS, &success);
	if (!success)
    {
        glGetShaderInfoLog(shaders[1], 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
	shader = linkProgram(shaders);

	glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }


	// Cleanup extra state
	for (auto s : shaders)
		glDeleteShader(s);
	shaders.clear();

	// Get uniform locations
	// GLint color = glGetUniformLocation(shader, "outCol");
}



void GLState::runFFTShaders(int runCount){
	int inOut = 0;
	{//HORIZONTAL FFT
		HorizontalFFTcompute->use();

		GLuint ssbo;
		GLuint binding = 0;
		glGenBuffers(1, &ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, reverseIndices.size() * sizeof(GLint), reverseIndices.data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glUniform1i(glGetUniformLocation(HorizontalFFTcomputeshader, "N"), mainOcean->getN());
		glUniform1i(glGetUniformLocation(HorizontalFFTcomputeshader, "inOutTex0"), 1);
		glUniform1i(glGetUniformLocation(HorizontalFFTcomputeshader, "inOutTex1"), 2);
		glUniform1i(glGetUniformLocation(HorizontalFFTcomputeshader, "inOutDecide"), 0);
		glUniform1i(glGetUniformLocation(HorizontalFFTcomputeshader, "currStage"), 0);
		
		if(runCount == 1){
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, HKzcomputeTexture->id());
			HKzcomputeTexture->bindImage(1, GL_READ_WRITE);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, HorizontalFFTcomputeTexture->id());
			HorizontalFFTcomputeTexture->bindImage(2, GL_READ_ONLY);
		}else if(runCount == 2){
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, HKxcomputeTexture->id());
			HKxcomputeTexture->bindImage(1, GL_READ_WRITE);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, HorizontalFFTcomputeTexture2->id());
			HorizontalFFTcomputeTexture2->bindImage(2, GL_READ_ONLY);
		}else if(runCount == 3){
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, HKycomputeTexture->id());
			HKycomputeTexture->bindImage(1, GL_READ_WRITE);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, HorizontalFFTcomputeTexture3->id());
			HorizontalFFTcomputeTexture3->bindImage(2, GL_READ_ONLY);
		}
		activeText = -1;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
		// std::cout<<activeText<<H0computeTexture->id()<<std::endl;

		// glActiveTexture(GL_TEXTURE1);
		// glBindTexture(GL_TEXTURE_2D, HKzcomputeTexture->id());
		// HKzcomputeTexture->bindImage(1, GL_READ_WRITE);

		// // std::cout<<activeText<<H0MinusKcomputeTexture->id()<<std::endl;
		// glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, compute_data4.data() );
		// ofs.open("temp/fftInput1.ppm", std::ios_base::out | std::ios_base::binary);
		// ofs2.open("temp/fftInput1.txt");
		// ofs << "P6" << std::endl << w << ' ' << h << std::endl << "255" << std::endl;   
		// for(int curr = 0; curr < w*h*4; curr+=4){
		// 	ofs << (char) (compute_data4.at(curr + 0) * 256) << (char) (compute_data4.at(curr + 1) * 256) << (char) (compute_data4.at(curr + 2) * 256);
		// 	ofs2 <<  (compute_data4.at(curr + 0)) << " " << (compute_data4.at(curr + 1)) << " " <<  ((compute_data4.at(curr + 2))) << " " <<  ((compute_data4.at(curr + 3)))  << '\n';
		// }
		// ofs.close();
		// ofs2.close();
		for(int i = 0; i < log2N; i++){
			glUniform1i(glGetUniformLocation(HorizontalFFTcomputeshader, "currStage"), i);
			glUniform1i(glGetUniformLocation(HorizontalFFTcomputeshader, "inOutDecide"), inOut);
			glDispatchCompute(GRID / 16, GRID / 16, 1);
			glFinish();
			glMemoryBarrier( GL_ALL_BARRIER_BITS );
			// TESTING ONLY - VerticalFFTTest
			// std::string fname = "temp/test_HorizontalFFT" + std::to_string(i);
			// glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, compute_data4.data() );
			// ofs.open(fname + ".ppm", std::ios_base::out | std::ios_base::binary);
			// ofs2.open(fname + ".txt");
			// ofs << "P6" << std::endl << w << ' ' << h << std::endl << "255" << std::endl;   
			// ofs2 << "CurrStage: " << i << std::endl; 
			// for(int curr = 0; curr < w*h*4; curr+=4){
			// 	ofs << (char) (compute_data4.at(curr + 0) * 256) << (char) (compute_data4.at(curr + 1) * 256) << (char) (compute_data4.at(curr + 2) * 256);
			// 	ofs2 <<  (compute_data4.at(curr + 0)) << " " << (compute_data4.at(curr + 1)) << " " <<  ((compute_data4.at(curr + 2))) << " " <<  ((compute_data4.at(curr + 3)))  << '\n';
			// }
			// ofs.close();
			// ofs2.close();
			// exit(0);
			inOut = !inOut;
		}
		// exit(0);

		
		
		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		// glDisable(GL_TEXTURE_2D);
		glUseProgram(0);
		glFinish();
		
	}

	
	{//VERTICAL FFT
		VerticalFFTcompute->use();

		GLuint ssbo;
		GLuint binding = 0;
		glGenBuffers(1, &ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, reverseIndices.size() * sizeof(GLint), reverseIndices.data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glUniform1i(glGetUniformLocation(VerticalFFTcomputeshader, "N"), mainOcean->getN());
		glUniform1i(glGetUniformLocation(VerticalFFTcomputeshader, "inOutTex0"), 1);
		glUniform1i(glGetUniformLocation(VerticalFFTcomputeshader, "inOutTex1"), 2);
		// glUniform1i(glGetUniformLocation(VerticalFFTcomputeshader, "inOutDecide"), 0);
		glUniform1i(glGetUniformLocation(VerticalFFTcomputeshader, "currStage"), 0);
		if(runCount == 1){
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, HKzcomputeTexture->id());
			HKzcomputeTexture->bindImage(1, GL_READ_WRITE);
			activeText = -1;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
			// std::cout<<activeText<<H0computeTexture->id()<<std::endl;
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, HorizontalFFTcomputeTexture->id());
			HorizontalFFTcomputeTexture->bindImage(2, GL_READ_WRITE);
		}else if(runCount == 2){
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, HKxcomputeTexture->id());
			HKxcomputeTexture->bindImage(1, GL_READ_WRITE);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, HorizontalFFTcomputeTexture2->id());
			HorizontalFFTcomputeTexture2->bindImage(2, GL_READ_WRITE);
		}else if(runCount == 3){
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, HKycomputeTexture->id());
			HKycomputeTexture->bindImage(1, GL_READ_WRITE);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, HorizontalFFTcomputeTexture3->id());
			HorizontalFFTcomputeTexture3->bindImage(2, GL_READ_WRITE);
		}
		activeText = -1;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
		// std::cout<<activeText<<H0MinusKcomputeTexture->id()<<std::endl;
		// std::cout<<"HERE1"<<std::endl;
		for(int i = 0; i < log2N; i++){
			glUniform1i(glGetUniformLocation(VerticalFFTcomputeshader, "currStage"), i);
			glUniform1i(glGetUniformLocation(VerticalFFTcomputeshader, "inOutDecide"), inOut);
			glDispatchCompute(GRID / 16, GRID / 16, 1);
			glFinish();
			inOut = !inOut;
		}
		// std::cout<<"HERE2"<<std::endl;

		glMemoryBarrier( GL_ALL_BARRIER_BITS );
		
		//TESTING ONLY - VerticalFFTTest
		// glGetTexImage( GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, compute_data3.data() );
		// ofs.open("temp/test_VerticalFFT.ppm", std::ios_base::out | std::ios_base::binary);
		// ofs2.open("temp/test_VerticalFFT.txt");
		// ofs << "P6" << std::endl << w << ' ' << h << std::endl << "255" << std::endl;    
		// for(int curr = 0; curr < w*h*3; curr+=3){
		// 	ofs << (char) (compute_data3.at(curr + 0) * 256) << (char) (compute_data3.at(curr + 1) * 256) << (char) (compute_data3.at(curr + 2) * 256);
		// 	ofs2 <<  (compute_data3.at(curr + 0)) << " " << (compute_data3.at(curr + 1)) << " " <<  (0) << '\n';
		// }
		// ofs.close();
		// ofs2.close();

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		// glDisable(GL_TEXTURE_2D);
		glUseProgram(0);
		glFinish();
	}
	
	{//INVERSION (-1^x*-1*y*res)
		InversionFFTcompute->use();

		glUniform1i(glGetUniformLocation(InversionFFTcomputeshader, "N"), mainOcean->getN());
		glUniform1i(glGetUniformLocation(InversionFFTcomputeshader, "inOutTex0"), 1);
		glUniform1i(glGetUniformLocation(InversionFFTcomputeshader, "inOutTex1"), 2);
		glUniform1i(glGetUniformLocation(InversionFFTcomputeshader, "finalOut"), 0);

	
		if(runCount == 1){
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, HKzcomputeTexture->id());
			HKzcomputeTexture->bindImage(1, GL_READ_ONLY);
			activeText = -1;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
			// std::cout<<activeText<<H0computeTexture->id()<<std::endl;
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, HorizontalFFTcomputeTexture->id());
			HorizontalFFTcomputeTexture->bindImage(2, GL_READ_ONLY);
			activeText = -1;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, InversionFFTcomputeTexture1->id());
			InversionFFTcomputeTexture1->bindImage(0, GL_WRITE_ONLY);
		}else if(runCount == 2){
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, HKxcomputeTexture->id());
			HKxcomputeTexture->bindImage(1, GL_READ_ONLY);
			activeText = -1;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
			// std::cout<<activeText<<H0computeTexture->id()<<std::endl;
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, HorizontalFFTcomputeTexture2->id());
			HorizontalFFTcomputeTexture2->bindImage(2, GL_READ_ONLY);
			activeText = -1;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, InversionFFTcomputeTexture2->id());
			InversionFFTcomputeTexture2->bindImage(0, GL_WRITE_ONLY);
		}else if(runCount == 3){
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, HKycomputeTexture->id());
			HKycomputeTexture->bindImage(1, GL_READ_ONLY);
			activeText = -1;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
			// std::cout<<activeText<<H0computeTexture->id()<<std::endl;
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, HorizontalFFTcomputeTexture3->id());
			HorizontalFFTcomputeTexture3->bindImage(2, GL_READ_ONLY);
			activeText = -1;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, InversionFFTcomputeTexture3->id());
			InversionFFTcomputeTexture3->bindImage(0, GL_WRITE_ONLY);
		}
		glUniform1i(glGetUniformLocation(InversionFFTcomputeshader, "inOutDecide"), inOut);
		glDispatchCompute(GRID / 16, GRID / 16, 1);
		glFinish();

		glMemoryBarrier( GL_ALL_BARRIER_BITS );
		// // TESTING ONLY - InversionTest
		// if(runCount == 1){
		// 	glActiveTexture(GL_TEXTURE0);
		// 	glBindTexture(GL_TEXTURE_2D, InversionFFTcomputeTexture1->id());
		// 	InversionFFTcomputeTexture1->bindImage(1, GL_READ_ONLY);
		// 	glGetTexImage( GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, compute_data3.data() );
		// 	ofs.open("temp/test_inversion.ppm", std::ios_base::out | std::ios_base::binary);
		// 	ofs2.open("temp/test_inversion.txt");
		// 	ofs << "P6" << std::endl << w << ' ' << h << std::endl << "255" << std::endl;    
		// 	for(int curr = 0; curr < w*h*3; curr+=3){
		// 		ofs << (char) (compute_data3.at(curr + 0) * 256) << (char) (compute_data3.at(curr + 1) * 256) << (char) (compute_data3.at(curr + 2) * 256);
		// 		ofs2 <<  (compute_data3.at(curr + 0)) << " " << (compute_data3.at(curr + 1)) << " " <<  (0) << '\n';
		// 	}
		// 	ofs.close();
		// 	ofs2.close();
		// 	// exit(0);
		// }
		
	}
}


void GLState::runHkShader(){
	HKcompute->use();
	glUniform1f(glGetUniformLocation(HKcomputeshader, "T"), currTime);
	glUniform1i(glGetUniformLocation(HKcomputeshader, "N"), mainOcean->getN());
	glUniform1i(glGetUniformLocation(HKcomputeshader, "startL"), mainOcean->getL());
	glUniform1i(glGetUniformLocation(HKcomputeshader, "img_h0"), 1);
	glUniform1i(glGetUniformLocation(HKcomputeshader, "img_h0MinusK"), 2);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, H0computeTexture->id());
	H0computeTexture->bindImage(1, GL_READ_ONLY);

	activeText = -1;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
	// std::cout<<activeText<<H0computeTexture->id()<<std::endl;

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, H0MinusKcomputeTexture->id());
	H0MinusKcomputeTexture->bindImage(2, GL_READ_ONLY);

	activeText = -1;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
	// std::cout<<activeText<<H0MinusKcomputeTexture->id()<<std::endl;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, HKzcomputeTexture->id());
	HKzcomputeTexture->bindImage(0, GL_WRITE_ONLY);

	activeText = -1;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
	// std::cout<<activeText<<HKcomputeTexture->id()<<std::endl;


	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, HKxcomputeTexture->id());
	HKxcomputeTexture->bindImage(3, GL_WRITE_ONLY);

	activeText = -1;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
	// std::cout<<activeText<<HKcomputeTexture->id()<<std::endl;


	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, HKycomputeTexture->id());
	HKycomputeTexture->bindImage(4, GL_WRITE_ONLY);

	glDispatchCompute(GRID / 16, GRID / 16, 1);
	glMemoryBarrier( GL_ALL_BARRIER_BITS );


	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, HKzcomputeTexture->id());
	// HKzcomputeTexture->bindImage(0, GL_WRITE_ONLY);	






	// glGetTexImage( GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, compute_data.data() );
	// ofs.open("temp/test_hkz.ppm", std::ios_base::out | std::ios_base::binary);
	// ofs2.open("temp/test_hkz.txt");
	// ofs << "P6" << std::endl << w << ' ' << h << std::endl << "255" << std::endl;    
	// for(int curr = 0; curr < w*h*2; curr+=2){
	// 	ofs << (char) (compute_data.at(curr + 0) * 256) << (char) (compute_data.at(curr + 1) * 256) << (char) (0);
	// 	ofs2 <<  (compute_data.at(curr + 0)) << " " << (compute_data.at(curr + 1)) << " " <<  (0) << '\n';
	// }
	// ofs.close();
	// ofs2.close();
	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, HKzcomputeTexture->id());
	// HKycomputeTexture->bindImage(0, GL_WRITE_ONLY);	

	// glGetTexImage( GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, compute_data.data() );
	// ofs.open("temp/test_hky.ppm", std::ios_base::out | std::ios_base::binary);
	// ofs2.open("temp/test_hky.txt");
	// ofs << "P6" << std::endl << w << ' ' << h << std::endl << "255" << std::endl;    
	// for(int curr = 0; curr < w*h*2; curr+=2){
	// 	ofs << (char) (compute_data.at(curr + 0) * 256) << (char) (compute_data.at(curr + 1) * 256) << (char) (0);
	// 	ofs2 <<  (compute_data.at(curr + 0) * 256) << " " << (compute_data.at(curr + 1) * 256) << " " <<  (0) << '\n';
	// }
	// ofs.close();
	// ofs2.close();
	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, HKxcomputeTexture->id());
	// HKxcomputeTexture->bindImage(0, GL_WRITE_ONLY);	

	// glGetTexImage( GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, compute_data.data() );
	// ofs.open("temp/test_hkx.ppm", std::ios_base::out | std::ios_base::binary);
	// ofs2.open("temp/test_hkx.txt");
	// ofs << "P6" << std::endl << w << ' ' << h << std::endl << "255" << std::endl;    
	// for(int curr = 0; curr < w*h*2; curr+=2){
	// 	ofs << (char) (compute_data.at(curr + 0) * 256) << (char) (compute_data.at(curr + 1) * 256) << (char) (0);
	// 	ofs2 <<  (compute_data.at(curr + 0) * 256) << " " << (compute_data.at(curr + 1) * 256) << " " <<  (0) << '\n';
	// }
	// ofs.close();
	// ofs2.close();



	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, HKzcomputeTexture->id());
	// HKzcomputeTexture->bindImage(0, GL_WRITE_ONLY);	
	// glGetTexImage( GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, compute_data.data() );
	// ofs.open("temp/test_hkz.ppm", std::ios_base::out | std::ios_base::binary);
	// ofs2.open("temp/test_hkz.txt");
	// ofs << "P6" << std::endl << w << ' ' << h << std::endl << "255" << std::endl;    
	// for(int curr = 0; curr < w*h*2; curr+=2){
	// 	ofs << (char) (compute_data.at(curr + 0) * 256) << (char) (compute_data.at(curr + 1) * 256) << (char) (0);
	// 	ofs2 <<  (compute_data.at(curr + 0) * 256) << " " << (compute_data.at(curr + 1) * 256) << " " <<  (0) << '\n';
	// }
	// ofs.close();
	// ofs2.close();
	// exit(0);

	
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	// glDisable(GL_TEXTURE_2D);
	glUseProgram(0);
	glFinish();
}



void GLState::runCombineMapsShader(){
	CombineMapscompute->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, CombineMapscomputeTexture->id());
	CombineMapscomputeTexture->bindImage(0, GL_WRITE_ONLY);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, InversionFFTcomputeTexture1->id());
	InversionFFTcomputeTexture1->bindImage(1, GL_READ_ONLY);
	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, InversionFFTcomputeTexture2->id());
	InversionFFTcomputeTexture2->bindImage(2, GL_READ_ONLY);
	
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, InversionFFTcomputeTexture3->id());
	InversionFFTcomputeTexture3->bindImage(3, GL_READ_ONLY);
	glDispatchCompute(GRID / 16, GRID / 16, 1);
	glMemoryBarrier( GL_ALL_BARRIER_BITS );


	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, CombineMapscomputeTexture->id());
	// CombineMapscomputeTexture->bindImage(0, GL_READ_ONLY);	
	// glGetTexImage( GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, compute_data3.data() );
	// ofs.open("temp/test_combined.ppm", std::ios_base::out | std::ios_base::binary);
	// ofs2.open("temp/test_combined.txt");
	// ofs << "P6" << std::endl << w << ' ' << h << std::endl << "255" << std::endl;    
	// for(int curr = 0; curr < w*h*3; curr+=3){
	// 	ofs << (char) (compute_data3.at(curr + 0) * 256) << (char) (compute_data3.at(curr + 1) * 256) << (char) (compute_data3.at(curr + 2) * 256);
	// 	ofs2 <<  (compute_data3.at(curr + 0)) << " " << (compute_data3.at(curr + 1)) << " " <<  (compute_data3.at(curr + 2)) << '\n';
	// }
	// ofs.close();
	// ofs2.close();
	// exit(0);
}

void GLState::runNormalMapShader(){
	NormalMapcompute->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, CombineMapscomputeTexture->id());
	CombineMapscomputeTexture->bindImage(0, GL_READ_ONLY);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, NormalMapcomputeTexture->id());
	NormalMapcomputeTexture->bindImage(1, GL_WRITE_ONLY);
	
	glDispatchCompute(GRID / 16, GRID / 16, 1);
	glMemoryBarrier( GL_ALL_BARRIER_BITS );
}