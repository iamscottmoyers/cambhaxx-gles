# Standard Makefile
CXX := gcc
WARNINGFLAGS := -Wall -Wextra
CXXFLAGS := $(WARNINGFLAGS) -g --pedantic
# Executable name
EXEC = theDude

# Directories
OBJDIR = obj
SRCDIR = src
INCLUDEDIR := include

# OpenGL?
UNAME = $(shell uname)
ifeq ($(UNAME), Darwin)
OPENGL_FLAGS = -framework OpenGL -framework GLUT
else
OPENGL_FLAGS = -lGL -lglut
endif

# Use as: make USE_GTK=1
ifdef USE_GTK
COMPILE_FLAGS += -DUSE_GTK
OPENGL_FLAGS = $(shell pkg-config --libs gtk+-2.0 gtkglext-1.0 gtkglext-x11-1.0)
CXXFLAGS += $(shell pkg-config --cflags gtk+-2.0 gtkglext-1.0 gtkglext-x11-1.0)
SRCS = $(SRCDIR)/window_system_gtk.c
else
SRCS = $(SRCDIR)/window_system_glut.c
endif

# Libraries
LIBS := -lm $(OPENGL_FLAGS)

# Files and folders
SRCS   += $(SRCDIR)/dude.c
SRCDIRS = $(shell find . -name '*.c' | dirname {} | sort | uniq | sed 's/\/$(SRCDIR)//g' )
OBJS    = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

# Targets
$(EXEC): build-obj-store $(OBJS)
	$(CXX) $(OBJS) $(LIBS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CXX) $(OPTS) $(CXXFLAGS) $(COMPILE_FLAGS) -I$(INCLUDEDIR) -c $< -o $@

clean:
	rm -rf $(EXEC) $(OBJDIR)

build-obj-store:
	-mkdir -p $(OBJDIR)
