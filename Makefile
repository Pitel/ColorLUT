CFLAGS=-O2 -pipe -pedantic-errors -Wall -Wextra -march=native -fomit-frame-pointer -std=gnu99 `sdl-config --cflags` `pkg-config --cflags glew gl`
LDLIBS=`sdl-config --libs` `pkg-config --libs glew gl` #-lGLEW

all: colorLUT

.PHONY: clean pack

colorLUT:

run: colorLUT
	./colorLUT || LIBGL_ALWAYS_SOFTWARE=1 ./colorLUT

clean:
	$(RM) colorLUT *.zip

pack:
	zip -9 xlogin00.zip *.c *.fs *.vs Makefile
