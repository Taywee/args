OS = $(shell uname -s)

CC 			?= 	cc
CXX			?= 	c++
DESTDIR		?= 	/usr
FLAGS 		+= 	-std=c++11
ifdef DEBUG
FLAGS		+=	-ggdb -O0
else
FLAGS		+=	-O3
endif

LIBS 		= 	glfw3 assimp
CFLAGS		+=	-I. $(FLAGS) -c -MMD `pkg-config --cflags $(LIBS)`
LDFLAGS		+=	$(FLAGS) `pkg-config --libs $(LIBS)`
ifneq ($(OS), Darwin)
LDFLAGS		+=	-lGL
endif

SOURCES		= 	test.cxx
OBJECTS		= 	$(SOURCES:.cxx=.o)
DEPENDENCIES=	$(SOURCES:.cxx=.d)
EXECUTABLE	=	test

.PHONY: all clean pages

all: $(EXECUTABLE)

-include $(DEPENDENCIES)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm $(EXECUTABLE) $(OBJECTS) $(DEPENDENCIES)

%.o: %.cxx
	$(CXX) $< -o $@ $(CFLAGS)

pages:
	doxygen Doxyfile
	cp -rv html/* .
	rm -r html

