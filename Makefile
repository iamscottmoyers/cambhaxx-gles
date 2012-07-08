CXX := gcc
WARNINGFLAGS := -Wall -Wextra
CXXFLAGS := $(WARNINGFLAGS) -g --pedantic

UNAME = $(shell uname)
ifeq ($(UNAME), Darwin)
OPENGL_FLAGS = -framework OpenGL -framework GLUT
else
OPENGL_FLAGS = -lGL -lGLU -lglut
endif

LIBS := -lm $(OPENGL_FLAGS)

dude : main.o
	$(CXX) $(CXXFLAGS) $(LIBS) $^ -o $@

%.o : %.c
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

clean :
	rm -f *.o *~
	rm -f dude
