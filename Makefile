CFLAGS=-O2 -pipe -pedantic-errors -Wall -Wextra -march=native -fomit-frame-pointer -std=gnu99 `sdl-config --cflags` `pkg-config --cflags glew gl SDL_image`
LDLIBS=`sdl-config --libs` `pkg-config --libs glew gl SDL_image`

CC=gcc

#CVT
LDFLAGS_CVT=-L/usr/lib
LDLIBS_CVT=`sdl-config --libs` -lSDL_image -lGL -lGLEW

#WITH FFMPEG
CFLAGS_FF=`pkg-config --cflags libavcodec libavformat libavutil libswscale`
LDLIBS_FF=`pkg-config --libs libavcodec libavformat libavutil libswscale` -lm

all: colorLUT

.PHONY: clean pack ffmpeg cvt

colorLUT: 
	$(CC) $(LDFLAGS)$(LDLIBS) $(CFLAGS) colorLUT.c -o colorLUT

cvt: 
	$(CC) $(LDFLAGS) $(LDLIBS) $(LDFLAGS_CVT) $(LDLIBS_CVT) $(CFLAGS) colorLUT.c -o colorLUT

ffmpeg: 
	$(CC) $(LDFLAGS)-DFFPLAYER $(LDLIBS) $(CFLAGS) $(LDLIBS_FF) $(CFLAGS_FF) ffmpeg_lut.c ffmpeg_lut.h colorLUT.c -o colorLUT

ffmpeg_cvt: 
	$(CC) $(LDFLAGS_CVT) $(LDLIBS_CVT) $(LDFLAGS)-DFFPLAYER $(LDLIBS) $(CFLAGS) $(LDLIBS_FF) $(CFLAGS_FF) ffmpeg_lut.c ffmpeg_lut.h colorLUT.c -o colorLUT


clean:
	$(RM) colorLUT *.zip *.o

pack:
	zip -9 xlogin00.zip *.c *.fs *.png Makefile
