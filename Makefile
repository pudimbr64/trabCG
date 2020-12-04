CC = g++

GLLIBS = -lglut -lGLEW -lGL

all: modelo.cpp
	g++ -o modelo modelo.cpp cgImage.c cgTypes.c $(GLLIBS)
	
clean:
	rm -f modelo
