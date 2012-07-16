CXX := gcc
WARNINGFLAGS := -Wall -Wextra
CXXFLAGS := $(WARNINGFLAGS) -g --pedantic
INCLUDE = -I../../include/

UNAME = $(shell uname)
ifeq ($(UNAME), Darwin)
OPENGL_FLAGS = -framework OpenGL -framework GLUT
else
OPENGL_FLAGS = -lGL -lGLU -lglut
endif

# Use as: make USE_GTK=1
ifdef USE_GTK
COMPILE_FLAGS += -DUSE_GTK
OPENGL_FLAGS = $(shell pkg-config --libs gtk+-2.0 gtkglext-1.0 gtkglext-x11-1.0)
CXXFLAGS += $(shell pkg-config --cflags gtk+-2.0 gtkglext-1.0 gtkglext-x11-1.0)
endif

# This might be better as a seperate rule if we want more stuff?
ifdef DEBUG
COMPILE_FLAGS += -DDEBUG
endif


LIBS := -lm $(OPENGL_FLAGS)

colin : main.o
	$(CXX) $(CXXFLAGS) $(LIBS) $^ -o $@

%.o : %.c
	$(CXX) $(COMPILE_FLAGS) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

clean :
	rm -f *.o *~
	rm -f colin
