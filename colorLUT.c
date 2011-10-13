#include <stdbool.h>

#include <SDL.h>
#include <SDL_opengl.h>

int main() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
	
	if (SDL_SetVideoMode(800, 600, 0, SDL_OPENGL) == NULL) {
		fprintf(stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError());
		SDL_Quit();
		exit(2);
	}
	
	SDL_WM_SetCaption("colorLUT", NULL);
	
	bool done = false;
	while (!done) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE) {
				done = true;
			}
		}
	}
	SDL_Quit();
	exit();
}
