#include <stdbool.h>
#include <SDL.h>
#include <SDL_image.h>
#include <GL/glew.h>

GLuint texture, tcAttrib, textureUniform;


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

/* The main drawing function. */
void DrawGLScene() {
	static const short vertices[] = {-1,1, 1,1, 1,-1, -1,-1};
	glClear(GL_COLOR_BUFFER_BIT);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_SHORT, 0, vertices);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(textureUniform,0);

	glDrawArrays(GL_QUADS, 0, 8);
	SDL_GL_SwapBuffers();
}


void SurfaceImage2D(GLenum target, GLint level, GLint internalformat, SDL_Surface * surface)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT,4);
    if(     (surface->format->Rmask == 0xff0000) &&
            (surface->format->Gmask == 0xff00) &&
            (surface->format->Bmask == 0xff) &&
            (surface->format->Amask == 0))
    {
        glTexImage2D(target, level, internalformat, surface->w, surface->h, 0, GL_BGR, GL_UNSIGNED_BYTE, surface->pixels);
    }
    else if((surface->format->Rmask == 0xff) &&
            (surface->format->Gmask == 0xff00) &&
            (surface->format->Bmask == 0xff0000) &&
            (surface->format->Amask == 0))
    {
        glTexImage2D(target, level, internalformat, surface->w, surface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);
    }
    else
    {
		fprintf(stderr, "FFFFFFFFUUUUUUUUU  - SurfaceImage2D");
    }
}



int main(int argc, char **argv) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

    SDL_Surface * surface;

    if(argc == 2)
    {
        surface = IMG_Load(argv[1]);
        if(surface == NULL)
        {
            fprintf(stderr, "Oh'no loadin' of picture is broken...\n");
            SDL_Quit();
            return EXIT_FAILURE;
        }

        if (SDL_SetVideoMode(surface->w, surface->h, 0, SDL_OPENGL) == NULL) {
            fprintf(stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError());
            SDL_Quit();
            return EXIT_FAILURE;
        }


    }
    else
    {
		fprintf(stderr, "usage: ./%s <image>", SDL_GetError());
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

    const char* vs_src = file2string("colorLUT.vs");
	GLuint vs = glCreateShader(GL_VERTEX_SHADER_ARB);
	glShaderSource(vs, 1, &vs_src, NULL);
	glCompileShader(vs);
	shaderlog(vs);

	const char* fs_src = file2string("colorLUT.fs");
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER_ARB);
	glShaderSource(fs, 1, &fs_src, NULL);
	glCompileShader(fs);
	shaderlog(fs);

	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	shaderlog(program);


    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    SurfaceImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface);

    textureUniform = glGetUniformLocation(program, "tex");


	glUseProgram(program);

	bool done = false;
	SDL_Event event;
	while (!done && SDL_WaitEvent(&event)) {
		DrawGLScene();
		if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE) {
			done = true;
		}
	}
	SDL_Quit();
}
