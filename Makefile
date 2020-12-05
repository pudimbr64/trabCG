CC = g++

GLLIBS = -lglut -lGLEW -lGL

all: celular.cpp
	g++ -o celular celular.cpp $(GLLIBS)
	
clean:
	rm -f celular
