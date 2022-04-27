sources = \
	src/main.cpp \
	src/glstate.cpp \
	src/util.cpp \
	src/oceancalcs.cpp \
	src/compute.cpp \
	src/texture.cpp \
	src/glad.c 
libs = \
	-lGL \
	-lGLU \
	-lglfw \
	-lX11 \
	-lXrandr \
	-pthread \
	-lXi \
	-ldl
outname = base_freeglut

all:
	g++ -std=c++17 $(sources) $(libs) -o $(outname)
clean:
	rm $(outname)
