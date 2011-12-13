CFLAGS=-O2 -pipe -pedantic-errors -Wall -Wextra -march=native -fomit-frame-pointer -std=gnu99 `sdl-config --cflags` `pkg-config --cflags glew gl SDL_image libavcodec libavformat libavutil libswscale`
LDLIBS=`sdl-config --libs` `pkg-config --libs glew gl SDL_image libavcodec libavformat libavutil libswscale`

#CVT
#LDFLAGS=-L/usr/lib
#LDLIBS=`sdl-config --libs` -lSDL_image -lGL -lGLEW

all: colorLUT

.PHONY: clean pack

colorLUT:

clean:
	$(RM) colorLUT *.zip

pack:
	zip -9 xlogin00.zip *.c *.fs *.png Makefile
