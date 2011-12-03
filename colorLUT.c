#include <stdbool.h>
#include <SDL.h>
#include <SDL_image.h>
#include <GL/glew.h>

char *file2string(const char *path) {
	FILE *fd;
	long len, r;
	char *str;
	if (!(fd = fopen(path, "r"))) {
		fprintf(stderr, "Can't open file '%s' for reading\n", path);
		return NULL;
	}
	fseek(fd, 0, SEEK_END);
	len = ftell(fd);
	printf("File '%s' is %ld long\n", path, len);
	fseek(fd, 0, SEEK_SET);
	if (!(str = malloc(len * sizeof(char)))) {
		fprintf(stderr, "Can't malloc space for '%s'\n", path);
		return NULL;
	}
	r = fread(str, sizeof(char), len, fd);
	str[r - 1] = '\0'; /* Shader sources have to term with null */
	fclose(fd);
	return str;
}

void shaderlog(GLuint obj) {
	int infologLength = 0;
	int maxLength;
	
	if(glIsShader(obj)) {
		glGetShaderiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
	} else {
		glGetProgramiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
	}
	char infoLog[maxLength];
	
	if (glIsShader(obj)) {
		glGetShaderInfoLog(obj, maxLength, &infologLength, infoLog);
	} else {
		glGetProgramInfoLog(obj, maxLength, &infologLength, infoLog);
	}
	
	if (infologLength > 0) {
		printf("%s\n", infoLog);
	}
}

int main(int argc, char **argv) {
	//Help
	if (argc < 2) {
		printf("Usage: colorLUT <image>");
		return EXIT_FAILURE;
	}
	
	//SDL init
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}
	
	//Load image
	SDL_Surface * surface = IMG_Load(argv[1]);
	if (surface == NULL) {
		fprintf(stderr, "Unable to load image %s!\n", argv[1]);
		SDL_Quit();
		return EXIT_FAILURE;
	}
	
	//Window and OpenGL init
	if (SDL_SetVideoMode(surface->w, surface->h, 0, SDL_OPENGL) == NULL) {
		fprintf(stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError());
		SDL_Quit();
		return EXIT_FAILURE;
	}
	
	SDL_WM_SetCaption("colorLUT", NULL);
	
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		SDL_Quit();
		return EXIT_FAILURE;
	}
	
	printf("Using GLEW Version %s\n", glewGetString(GLEW_VERSION));
	
	if (!GLEW_VERSION_2_0) {
		fprintf(stderr, "This machine sucks and does not support OpenGL 2.0!\n");
		SDL_Quit();
		return EXIT_FAILURE;
	}
	
	//Building shader
	const char* fs_src = file2string("colorLUT.fs");
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER_ARB);
	glShaderSource(fs, 1, &fs_src, NULL);
	glCompileShader(fs);
	shaderlog(fs);
	
	GLuint program = glCreateProgram();
	glAttachShader(program, fs);
	glLinkProgram(program);
	shaderlog(program);
	
	glUseProgram(program);
	
	//Texture
	GLuint texture_format, texture;
	if (surface->format->BytesPerPixel == 4) {
		if (surface->format->Rmask == 0x000000ff) {
			texture_format = GL_RGBA;
		} else {
			texture_format = GL_BGRA;
		}
	} else if (surface->format->BytesPerPixel == 3) {
		if (surface->format->Rmask == 0x000000ff) {
			texture_format = GL_RGB;
		} else {
			texture_format = GL_BGR;
		}
	} else {
		fprintf(stderr, "The image is not truecolor!\n");
		SDL_Quit();
		return EXIT_FAILURE;
	}
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE0, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, surface->format->BytesPerPixel, surface->w, surface->h, 0, texture_format, GL_UNSIGNED_BYTE, surface->pixels);
	glUniform1i(glGetUniformLocation(program, "tex"), 0);
	glUniform2f(glGetUniformLocation(program, "resolution"), surface->w, surface->w);
	
	//Geometry
	glEnableClientState(GL_VERTEX_ARRAY);
	static const short vertices[] = {-1,1, 1,1, 1,-1, -1,-1};
	glVertexPointer(2, GL_SHORT, 0, vertices);
	
	//Draw
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_QUADS, 0, 8);
	SDL_GL_SwapBuffers();
	
	//Event loop
	bool done = false;
	SDL_Event event;
	while (!done && SDL_WaitEvent(&event)) {
		//putchar('.');
		if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE) {
			done = true;
		}
	}
	SDL_Quit();
}
