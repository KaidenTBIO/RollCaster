
CXX = i586-mingw32msvc-g++
STRIP = i586-mingw32msvc-strip

CFLAGS = -O3 -Wall -fno-strict-aliasing
LDFLAGS = -shared

OBJS = dll.o state.o
LIBS = -lwinmm

all: state.dll

.cpp.o:
	$(CXX) $(CFLAGS) -o $@ -c $<

clean:
	rm $(OBJS) state.dll

state.dll: $(OBJS)
	$(CXX) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJS) $(LIBS)
	$(STRIP) $@
