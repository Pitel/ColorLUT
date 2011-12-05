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

void lut(char* lutpath, GLuint shader) {
	GLuint texture, format;
	
	//LUT
	SDL_Surface * lutimage = IMG_Load(lutpath);
	if (lutimage == NULL) {
		fprintf(stderr, "Unable to load LUT %s!\n", lutpath);
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
	SDL_Surface * lut = SDL_CreateRGBSurface(lutimage->flags, 256, 16, lutimage->format->BitsPerPixel, lutimage->format->Rmask, lutimage->format->Gmask, lutimage->format->Bmask, lutimage->format->Amask);
	SDL_Rect rect = {0, 0, 256, 16};
	SDL_BlitSurface(lutimage, &rect, lut, NULL);
	SDL_FreeSurface(lutimage);
	if (lut->format->BytesPerPixel == 4) {
		if (lut->format->Rmask == 0x000000ff) {
			format = GL_RGBA;
		} else {
			format = GL_BGRA;
		}
	} else if (lut->format->BytesPerPixel == 3) {
		if (lut->format->Rmask == 0x000000ff) {
			format = GL_RGB;
		} else {
			format = GL_BGR;
		}
	} else {
		fprintf(stderr, "The image is not truecolor!\n");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, lut->format->BytesPerPixel, lut->w, lut->h, 0, format, GL_UNSIGNED_BYTE, lut->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glGenerateMipmap(GL_TEXTURE_2D);
	glUniform1i(glGetUniformLocation(shader, "lut"), 1);
	SDL_FreeSurface(lut);
	
	//Geometry
	glEnable(GL_VERTEX_ARRAY);
	const short vertices[] = {-1,1, 1,1, 1,-1, -1,-1};
	glVertexPointer(2, GL_SHORT, 0, vertices);
	
	//Draw
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_QUADS, 0, 8);
}

int main(int argc, char **argv) {
	//Help
	if (argc < 3) {
		printf("Usage: colorLUT <image> <LUT>...\n");
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
	GLuint texture, format;
	if (surface->format->BytesPerPixel == 4) {
		if (surface->format->Rmask == 0x000000ff) {
			format = GL_RGBA;
		} else {
			format = GL_BGRA;
		}
	} else if (surface->format->BytesPerPixel == 3) {
		if (surface->format->Rmask == 0x000000ff) {
			format = GL_RGB;
		} else {
			format = GL_BGR;
		}
	} else {
		fprintf(stderr, "The image is not truecolor!\n");
		SDL_Quit();
		return EXIT_FAILURE;
	}
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, surface->format->BytesPerPixel, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	glUniform1i(glGetUniformLocation(program, "tex"), 0);
	glUniform2f(glGetUniformLocation(program, "resolution"), surface->w, surface->h);
	
	int lutindex = 2;
	/*
	lut(argv[lutindex], program);
	SDL_GL_SwapBuffers();
	*/
	
	//Event loop
	SDL_Event event;
	while (SDL_WaitEvent(&event)) {
		if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE) {
			break;
		} else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
			lutindex++;
			if (lutindex >= argc) {
				lutindex = 2;
			}
		}
		printf("LUT: %s\n", argv[lutindex]);
		lut(argv[lutindex], program);
		SDL_GL_SwapBuffers();
	}
	SDL_FreeSurface(surface);
	SDL_Quit();
}
