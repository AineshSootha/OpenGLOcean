#define NOMINMAX
#include <iostream>
#include <chrono>
#include <thread>
#include "glstate.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "util.hpp"

#define GRID 256

//TESTING VARIABLES
std::ofstream ofs, ofs2;
int w = GRID, h = GRID;
std::vector<float> compute_data(w*h*2);
GLint activeText = -1;
//

// Constructor
GLState::GLState() :
	shader(0),
	vcount(GRID + 1),
	totalVcount(GRID * GRID * 6),
	xFormLoc(0),
	currTime(0.0f),
	log2N(log(GRID) / log(2)),
	camCoords(0.0f, 50.0f, 3.0f),
	reverseIndices(GRID, 0),
	vertices(vcount * vcount),
	indices(totalVcount),
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

// Called when OpenGL context is created (some time after construction)
void GLState::initializeGL() {
	// General settings
	std::cout<<std::endl<<GL_MAX_TEXTURE_UNITS<<std::endl;

	initShaders();

	int idx = 0;
	for(int y = -GRID / 2; y <= GRID / 2; y++){
		for(int x = -GRID / 2; x <= GRID / 2; x++){
			vertices[idx].pos = {2 * (float)x, 0.0f, 2 * (float)y}; 
			float u = ((float)x / GRID) + 0.5f;
			float v = ((float)y / GRID) + 0.5f;
			vertices[idx++].tex = glm::vec2(u, v) * 2.0f;
		}
	}
	idx = 0;

	//CounterClockwiseWinding
	for(int i = 0; i < GRID; i++){
		for(int j = 0; j < GRID; j++){
			int r1 = i * vcount;
			int r2 = (i+1) * vcount;
			indices[idx++] = (r1 + j);
			indices[idx++] = (r2 + j + 1);
			indices[idx++] = (r2 + j);
			indices[idx++] = (r2 + j + 1);
			indices[idx++] = (r1 + j);
			indices[idx++] = (r1 + j + 1);
		}
	}

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbuf);
    glGenBuffers(1, &ibuf);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbuf);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GridVert), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


	//GaussianRand Gen
	mainOcean = std::unique_ptr<Ocean>(new Ocean(GRID, 1000, 40, 4));
	mainOcean->generateRands(1, GRID,GRID);
	
	
	std::vector<GLfloat> gaussianRands = mainOcean -> getGaussianRands(1);
	std::vector<GLfloat> gaussianRandsMinusK = mainOcean -> getGaussianRands(2);
	gaussianRandTexture = std::unique_ptr<Texture>(new Texture(GRID, GRID, &gaussianRands[0], GL_RGB));
	gaussianRandId = gaussianRandTexture->id();
	

	gaussianRandTextureMinusK = std::unique_ptr<Texture>(new Texture(GRID, GRID, &gaussianRandsMinusK[0], GL_RGB));
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
	HKcomputeTexture = std::unique_ptr<Texture>(new Texture(GRID, GRID, NULL, GL_RGBA32F));
	HKcomputeshader = HKcompute->program();

	activeText = -1;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
	// std::cout<<activeText<<H0computeTexture->id()<<std::endl;

	for(int i = 0; i < GRID; i++){
		reverseIndices[i] = reverse(i);
		std::cout<<reverseIndices[i]<<std::endl;
	}

	//Butterfly compute shader setup
	/*ONLY FOR TESTING
	Butterflycompute = std::unique_ptr<Compute>(new Compute("shaders/fftButterflyCompute.glsl"));
	ButterflycomputeTexture = std::unique_ptr<Texture>(new Texture(log2N, GRID, NULL, GL_RGBA32F));
	Butterflycomputeshader = Butterflycompute->program();
	{
		Butterflycompute->use();
		glUniform1i(glGetUniformLocation(Butterflycomputeshader, "N"), mainOcean->getN());

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ButterflycomputeTexture->id());
		ButterflycomputeTexture->bindImage(0, GL_WRITE_ONLY);

		activeText = -1;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
		// std::cout<<activeText<<ButterflycomputeTexture->id()<<std::endl;

		GLuint ssbo;
		GLuint binding = 1;
		glGenBuffers(1, &ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, reverseIndices.size() * sizeof(GLint), reverseIndices.data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glDispatchCompute(log2N, 16, 1);
		glMemoryBarrier( GL_ALL_BARRIER_BITS );


		glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, compute_data.data() );
		ofs.open("temp/test_butterfly.ppm", std::ios_base::out | std::ios_base::binary);
		ofs2.open("temp/test_butterfly.txt");
		w = 8;
		h = 256;
		ofs << "P6" << std::endl << 256 << ' ' << h << std::endl << "255" << std::endl;    
		for(int curr = 0; curr < w*h*4; curr+=4){
			for(int i = 0; i < 32; i++){
				ofs << (char) (compute_data.at(curr + 0) * 256) << (char) (compute_data.at(curr + 1) * 256) << (char) (compute_data.at(curr + 2));
				ofs2 <<  (compute_data.at(curr + 0)) << " " << (compute_data.at(curr + 1)) << " " <<  (compute_data.at(curr + 2)) << " " <<  (compute_data.at(curr + 3)) << '\n';
			}
		}
		ofs.close();
		ofs2.close();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);
	}
	*/

	HorizontalFFTcompute = std::unique_ptr<Compute>(new Compute("shaders/fftHorizontalCompute.glsl"));
	HorizontalFFTcomputeTexture = std::unique_ptr<Texture>(new Texture(GRID, GRID, NULL, GL_RGBA32F));
	HorizontalFFTcomputeshader = HorizontalFFTcompute->program();

	activeText = -1;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
	// std::cout<<activeText<<H0computeTexture->id()<<std::endl;

}



// Called when window requests a screen redraw
void GLState::paintGL() {
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	if(parametersChanged){
		std::cout<<"Changed Entry\n";
		// BUILD H0 IMAGE
		H0compute->use();
		glUniform1i(glGetUniformLocation(H0computeshader, "N"), mainOcean->getN());
		glUniform1i(glGetUniformLocation(H0computeshader, "startL"), mainOcean->getL());
		glUniform1i(glGetUniformLocation(H0computeshader, "V"), mainOcean->getWindspeed());
		glUniform1i(glGetUniformLocation(H0computeshader, "A"), mainOcean->getA());
		glUniform1i(glGetUniformLocation(H0computeshader, "randSampler1"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gaussianRandId);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, H0computeTexture->id());
		H0computeTexture->bindImage(0, GL_WRITE_ONLY);

		activeText = -1;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
		// std::cout<<activeText<<H0computeTexture->id()<<std::endl;
		
		glDispatchCompute(16, 16, 1);
		glMemoryBarrier( GL_ALL_BARRIER_BITS );
		//TESTING ONLY - h0Test
		glGetTexImage( GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, compute_data.data() );
		w = 256;
		h = 256;
		ofs.open("temp/test_h0.ppm", std::ios_base::out | std::ios_base::binary);
		ofs2.open("temp/test_h0.txt");
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

		// BUILD H0(-k) IMAGE
		H0MinusKcompute->use();
		glUniform1i(glGetUniformLocation(H0MinusKcomputeshader, "N"), mainOcean->getN());
		glUniform1i(glGetUniformLocation(H0MinusKcomputeshader, "startL"), mainOcean->getL());
		glUniform1i(glGetUniformLocation(H0MinusKcomputeshader, "V"), mainOcean->getWindspeed());
		glUniform1i(glGetUniformLocation(H0MinusKcomputeshader, "A"), mainOcean->getA());
		glUniform1i(glGetUniformLocation(H0MinusKcomputeshader, "randSampler1"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gaussianRandIdMinusK);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, H0MinusKcomputeTexture->id());
		H0MinusKcomputeTexture->bindImage(0, GL_WRITE_ONLY);

		activeText = -1;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
		// std::cout<<activeText<<H0MinusKcomputeTexture->id()<<std::endl;
		
		glDispatchCompute(16, 16, 1);
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

	{
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
	glBindTexture(GL_TEXTURE_2D, HKcomputeTexture->id());
	HKcomputeTexture->bindImage(0, GL_WRITE_ONLY);

	activeText = -1;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
	// std::cout<<activeText<<HKcomputeTexture->id()<<std::endl;

	glDispatchCompute(16, 16, 1);
	glMemoryBarrier( GL_ALL_BARRIER_BITS );

	/*glGetTexImage( GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, compute_data.data() );
	ofs.open("temp/test_hk.ppm", std::ios_base::out | std::ios_base::binary);
	ofs2.open("temp/test_hk.txt");
	ofs << "P6" << std::endl << w << ' ' << h << std::endl << "255" << std::endl;    
	for(int curr = 0; curr < w*h*2; curr+=2){
		ofs << (char) (compute_data.at(curr + 0) * 256) << (char) (compute_data.at(curr + 1) * 256) << (char) (0);
		ofs2 <<  (compute_data.at(curr + 0) * 256) << " " << (compute_data.at(curr + 1) * 256) << " " <<  (0) << '\n';
	}
	ofs.close();
	ofs2.close();*/
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	// glDisable(GL_TEXTURE_2D);
	glUseProgram(0);
	glFinish();
	}

	
	{
		HorizontalFFTcompute->use();
		glUniform1i(glGetUniformLocation(HorizontalFFTcomputeshader, "N"), mainOcean->getN());
		glUniform1i(glGetUniformLocation(HorizontalFFTcomputeshader, "inOutTex0"), 1);
		glUniform1i(glGetUniformLocation(HorizontalFFTcomputeshader, "inOutTex1"), 2);
		glUniform1i(glGetUniformLocation(HorizontalFFTcomputeshader, "inOutDecide"), 0);
		glUniform1i(glGetUniformLocation(HorizontalFFTcomputeshader, "currStage"), 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, HKcomputeTexture->id());
		HorizontalFFTcomputeTexture->bindImage(1, GL_READ_WRITE);

		activeText = -1;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
		// std::cout<<activeText<<H0computeTexture->id()<<std::endl;

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, HorizontalFFTcomputeTexture->id());
		H0MinusKcomputeTexture->bindImage(2, GL_READ_ONLY);

		activeText = -1;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeText);
		// std::cout<<activeText<<H0MinusKcomputeTexture->id()<<std::endl;
		int inOut = 0;
		for(int i = 0; i < log2N; i++){
			glUniform1i(glGetUniformLocation(HorizontalFFTcomputeshader, "currStage"), i);
			glUniform1i(glGetUniformLocation(HorizontalFFTcomputeshader, "inOutDecide"), inOut);
			glDispatchCompute(16, 16, 1);
			glFinish();
			inOut = !inOut;
		}
		glMemoryBarrier( GL_ALL_BARRIER_BITS );

		/*glGetTexImage( GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, compute_data.data() );
		ofs.open("temp/test_hk.ppm", std::ios_base::out | std::ios_base::binary);
		ofs2.open("temp/test_hk.txt");
		ofs << "P6" << std::endl << w << ' ' << h << std::endl << "255" << std::endl;    
		for(int curr = 0; curr < w*h*2; curr+=2){
			ofs << (char) (compute_data.at(curr + 0) * 256) << (char) (compute_data.at(curr + 1) * 256) << (char) (0);
			ofs2 <<  (compute_data.at(curr + 0) * 256) << " " << (compute_data.at(curr + 1) * 256) << " " <<  (0) << '\n';
		}
		ofs.close();
		ofs2.close();*/
		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		// glDisable(GL_TEXTURE_2D);
		glUseProgram(0);
		glFinish();
	}
	/*glGetTexImage( GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, compute_data.data() );
	ofs.open("temp/test_hk.ppm", std::ios_base::out | std::ios_base::binary);
	ofs2.open("temp/test_hk.txt");
	ofs << "P6" << std::endl << w << ' ' << h << std::endl << "255" << std::endl;    
	for(int curr = 0; curr < w*h*2; curr+=2){
		ofs << (char) (compute_data.at(curr + 0) * 256) << (char) (compute_data.at(curr + 1) * 256) << (char) (0);
		ofs2 <<  (compute_data.at(curr + 0) * 256) << " " << (compute_data.at(curr + 1) * 256) << " " <<  (0) << '\n';
	}
	ofs.close();
	ofs2.close();*/
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	// glDisable(GL_TEXTURE_2D);
	glUseProgram(0);
	glFinish();


	

	glUseProgram(shader);
	glm::mat4 xform(1.0f);
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -camCoords.z));
	view = glm::rotate(view, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	view = glm::rotate(view, glm::radians(camCoords.x), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 pos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,0.0f,0.0f));
	xform = proj * view;//* pos;// * fixBB;
	glUniformMatrix4fv(xFormLoc, 1, GL_FALSE, glm::value_ptr(xform));
	glUniform1f(glGetUniformLocation(shader, "a_time"), currTime);
	glUniform1i(glGetUniformLocation(shader, "imgHk"), 2);
	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, HKcomputeTexture->id());
	HKcomputeTexture->bindImage(2, GL_READ_ONLY);

	// std::cout<<curTime<<std::endl;
	// glUniformMatrix4fv(xFormLoc, 1, GL_FALSE, glm::value_ptr(view));



  	glBindVertexArray(vao);

  	// glDrawElements(GL_LINES, length, GL_UNSIGNED_INT, NULL);
	glDrawElements(GL_TRIANGLES, totalVcount, GL_UNSIGNED_INT, 0);

  	glBindVertexArray(0);

	glUseProgram(0);
	currTime += 0.1f;	
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
