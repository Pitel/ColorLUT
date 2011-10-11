CFLAGS=-O2 -pipe -pedantic-errors -Wall -Wextra -march=native -fomit-frame-pointer -std=gnu99 `sdl-config --cflags`
LDFLAGS=`sdl-config --libs`

all: colorLUT

.PHONY: clean pack

colorLUT:

clean:
	$(RM) colorLUT *.zip

pack:
	zip -9 xlogin00.zip *.c Makefile
