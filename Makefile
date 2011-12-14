CFLAGS=-O2 -pipe -pedantic-errors -Wall -Wextra -march=native -fomit-frame-pointer -std=gnu99 `sdl-config --cflags` `pkg-config --cflags glew gl SDL_image libavcodec libavformat libavutil libswscale`
LDLIBS=`sdl-config --libs` `pkg-config --libs glew gl SDL_image libavcodec libavformat libavutil libswscale`

#CVT
#CFLAGS=-O2 -pipe -pedantic-errors -Wall -Wextra -march=native -fomit-frame-pointer -std=gnu99 `sdl-config --cflags`
#LDFLAGS=-L/usr/lib
#LDLIBS=`sdl-config --libs` -lSDL_image -lGL -lGLEW -lavcodec -lavformat -lavutil -lswscale

all: colorLUT

.PHONY: clean pack

colorLUT: ffmpeg_lut.o

clean:
	$(RM) colorLUT *.zip *.o

pack:
	zip -9 xlogin00.zip *.c *.h *.fs *.png Makefile
