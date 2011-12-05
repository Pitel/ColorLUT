CFLAGS=-O2 -pipe -pedantic-errors -Wall -Wextra -march=native -fomit-frame-pointer -std=gnu99 `sdl-config --cflags` `pkg-config --cflags glew gl SDL_image`
LDLIBS=`sdl-config --libs` `pkg-config --libs glew gl SDL_image`

all: colorLUT

.PHONY: clean pack

colorLUT:

clean:
	$(RM) colorLUT *.zip

pack:
	zip -9 xlogin00.zip *.c *.fs *.png Makefile
